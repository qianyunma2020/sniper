#include <unistd.h>
#include <iostream>
#include "AlgLevelParallel/ALPDAGTunnel.h"
#include "AlgLevelParallel/ALPAlgorithmTask.h"

//init global lock
//GBSemaphore  ALPDAGTunnel::m_DataNodeNameToStatusMap_Lock;


/*ALPDAGTunnel::ALPDAGTunnel(const std::string& name) : Task(name) {
    m_SchedulerIsActivate = INACTIVE;
}*/

ALPDAGTunnel::ALPDAGTunnel(const std::string& name, unsigned int tunnelID, ALPDAG* tmpDAG, ALPThreadPool* tmpThreadPool, ALPAlgResourcePool* tmpAlgResourcePool, ALPEventPool* tmpEventPool) : Task(name)
{
    m_BusyFlag = false;
    m_SchedulerIsActivate = INACTIVE;
    m_DAGTunnelID = tunnelID;
    m_DAG = tmpDAG;
    m_ThreadPool = tmpThreadPool;
    m_AlgResourcePool = tmpAlgResourcePool;
    m_EventPool = tmpEventPool;
    //clear 3 map content
    m_AlgNodeNameToAlgInstanceMap.clear();
    m_AlgNodeNameToStatusMap.clear();
    m_DataNodeNameToStatusMap.clear();
   
    //lock init
    m_DataNodeNameToStatusMap_Lock.notify();
}


ALPDAGTunnel::~ALPDAGTunnel()
{
}

bool ALPDAGTunnel::initialize()
{
    //另起一个线程用于操作调度
    LogInfo << "Activating scheduler thread............................." << std::endl;
    InitAlgNodesStatus();
    InitDataNodesStatus();
    LogDebug<< "Function ALPDAGTunnel::initialize() implys step1." <<std::endl;

    m_UpdateNodeStateThread = std::thread([this]() {this->ActivateDAGTunnelSchedulerThread(); });
    LogDebug<< "Function ALPDAGTunnel::initialize() implys step2." <<std::endl;
    //等待线程激活
    while (ACTIVE != m_SchedulerIsActivate)
    {
        if (ERROR == m_SchedulerIsActivate) {
            LogError << "Function ALPDAGTunnel::initialize() implys flag m_SchedulerIsActivate is ERROR!" << std::endl;
            return false;
        }
        LogInfo << "Waiting for scheduler to active............." << std::endl;
        sleep(1);
    }
    LogDebug<< "Function ALPDAGTunnel::initialize() implys step3." <<std::endl;
    
    //分配所有算法实例
    ApplyAllAlgorithmInstances();
    LogDebug<< "Function ALPDAGTunnel::initialize() implys step4." <<std::endl;

    if (!m_svcs.initialize()) {
        LogInfo << "Function ALPDAGTunnel::initialize() initialize failed!" << std::endl;
        return false;
    }
    LogDebug<< "Function ALPDAGTunnel::initialize() implys step5." <<std::endl;
    
    //创建DAG通道的看门狗
    m_DAGTunnelDog = new ALPDAGTunnelDog(this,m_EventPool);
    LogDebug<< "Function ALPDAGTunnel::initialize() implys step6." <<std::endl;
    m_DAGTunnelDog->initialize();   
    LogDebug<< "Function ALPDAGTunnel::initialize() implys step7." <<std::endl;    

    return true;
}


bool ALPDAGTunnel::finalize()
{
    //停止操作调度线程
    if (ALPReturnCode::FAILURE == DeactivateDAGTunnelSchedulerThread()) {
        LogError << "Scheduler thread can't be deactive." << std::endl;
    }
    //回收调度线程资源
    LogInfo << "Recycle scheduler thread resource." << std::endl;
    m_UpdateNodeStateThread.join();
    if (m_SchedulerIsActivate == ERROR) {
        LogError << "DAG Tunnel thread Error!" << std::endl;
        return false;
    }
    //释放所有算法实例
    ReleaseAllAlgorithmInstances();


    bool stat = true;
    if (!m_svcs.finalize()) stat = false;
    if (stat) {
        LogInfo << "Function  ALPDAGTunnel::finalize() finalize successful!" << std::endl;
    }else {
        LogInfo << "Function ALPDAGTunnel::finalize() finalize failed!" << std::endl;
    }
    return stat;
}


/*DAGTunnel中数据结点激活操作和算法结点激活操作构成一个操作队列，
  DAGTunnelScheduler线程用于管理此队列，
  此函数用于激活DAGTunnelScheduler线程。
*/
void ALPDAGTunnel::ActivateDAGTunnelSchedulerThread() {
    //当前操作
    std::function<ALPReturnCode()> currentAction;
    //更新调度器状态
    m_SchedulerIsActivate = ACTIVE;
    //处理操作队列，如果队列为空，则阻塞等待
    LogInfo << "The DAG Tunnel Scheduler thread starts working." << std::endl;
    while (m_SchedulerIsActivate == ACTIVE || 0 != m_ActivateNodeAction_Queue.size()) {
        m_ActivateNodeAction_Queue.pop(currentAction);
        ALPReturnCode tmpCode = currentAction();
        if (ALPReturnCode::SUCCESS == tmpCode) {
            LogInfo << "Current action is successful." << std::endl;
        }
        else {
            LogInfo << "Current action is failed, please check out in earnest !" << std::endl;
        }
    }

}

//关闭调度器
ALPReturnCode ALPDAGTunnel::DeactivateDAGTunnelSchedulerThread() {
    if (ACTIVE == m_SchedulerIsActivate) {
        //当前操作
        std::function<ALPReturnCode()> currentAction;
        //清空操作队列
        while (m_ActivateNodeAction_Queue.try_pop(currentAction)) {};

        //最后一个操作压入队列，提示线程结束
        m_ActivateNodeAction_Queue.push([this]()-> ALPReturnCode {
            std::cout <<"The DAG Tunnel Scheduler thread stops!"<< std::endl;
            //m_SchedulerIsActivate = INACTIVE;
            SetSchedulerINActive();
            return ALPReturnCode::SUCCESS;
        });
    }

    return ALPReturnCode::SUCCESS;
}

//激活DAG数据结点
ALPReturnCode ALPDAGTunnel::ActivateDataNode(const std::string name)
{
    //std::cout<<"Class ALPDAGTunnel function ActivateDataNode says hello: name="<<name<<", the size of m_DataNodeNameToStatusMap is:"<<m_DataNodeNameToStatusMap.size()<<std::endl;
    //获取DAG中对应的数据结点
    DataNode* tmpDataNodePtr = ALPDAG::GetDataNode(name);
    //std::cout<<"Class ALPDAGTunnel function ActivateDataNode step1."<<std::endl;
    //获取该数据结点所有的消费者
    std::vector<LeafAlgNode*> algnodesPtr = tmpDataNodePtr->GetConsumers();
    //std::cout<<"Class ALPDAGTunnel function ActivateDataNode step2."<<std::endl;

    m_DataNodeNameToStatusMap_Lock.wait();
    m_DataNodeNameToStatusMap[name] = true;
    for (unsigned int algcount = 0; algcount < algnodesPtr.size(); algcount++) {
        //检查每个消费者所依赖的输入数据是否全部可用
        //std::cout<<"Class ALPDAGTunnel function ActivateDataNode step3. algcount="<<algcount<<", algnodesPtr.size()="<<algnodesPtr.size()<<std::endl;
        bool flag = true;
        std::vector<DataNode*> datanodesPtr = algnodesPtr[algcount]->GetInputDataNodes();
        for (unsigned int datacount = 0; datacount < datanodesPtr.size(); datacount++) {
            //std::cout<<"Class ALPDAGTunnel function ActivateDataNode implys algnode="<<algnodesPtr[algcount]->GetName()<<" dependency datanode="<<datanodesPtr[datacount]->GetName()<<std::endl; 
            if (!m_DataNodeNameToStatusMap[datanodesPtr[datacount]->GetName()]) {
                flag = false;
                break;
            }
            else { 
                //std::cout<<"Class ALPDAGTunnel function ActivateDataNode implys datanode flag is true!"<<std::endl;
            }
        }
        //std::cout<<"Class ALPDAGTunnel function ActivateDataNode step4."<<std::endl;
        //如果消费者所依赖的输入数据全部可用，则激活该消费者算法
        if (flag) {
            std::string algnodename = algnodesPtr[algcount]->GetName();
            //std::cout<<"Class ALPDAGTunnel function ActivateDataNode step5. algnodename="<<algnodename<<std::endl;
            m_ActivateNodeAction_Queue.push([this, algnodename]() {
                return this->ActivateAlgorithmNode(algnodename);
            });
            //std::cout<<"Class ALPDAGTunnel function ActivateDataNode step6."<<std::endl;
            
        }
        //std::cout<<"Class ALPDAGTunnel function ActivateDataNode step7."<<std::endl;
    }
    m_DataNodeNameToStatusMap_Lock.notify();
    return ALPReturnCode::SUCCESS;
}

//激活DAG中事例开始的数据结点
ALPReturnCode ALPDAGTunnel::BeginEventProcessing(ALPDAGTunnel* tmpDAGTunnel) {
    //开始处理事例
    LogDebug<<"Class ALPDAGTunnel function BeginEventProcessing says hello."<<std::endl;
    m_ActivateNodeAction_Queue.push([tmpDAGTunnel, this]() {
        //return tmpDAGTunnel->ActivateEventBeginDataNode();
        const std::string bgevent = "EventBeginData";
        return tmpDAGTunnel->ActivateDataNode(bgevent);
    });
    LogDebug<<"Class ALPDAGTunnel function BeginEventProcessing says byebye."<<std::endl;
}

//激活特定算法输出数据集中的所有数据结点
ALPReturnCode ALPDAGTunnel::ActiveOutputDataNodesSet(const std::string algname) {
    //std::cout<<"Class ALPDAGTunnel function ActiveOutputDataNodesSet says hello. algname="<<algname<<std::endl; 
    //if event end alg, return success
    const std::string endeventalg = "ALPDAGTunnelEventEndAlg";
    if(endeventalg == algname) return ALPReturnCode::SUCCESS;

    for (auto tmpDataNodePtr : (ALPDAG::GetLeafAlgNode(algname)->GetOutputDataNodes())) {
        //LogDebug<<"Class ALPDAGTunnel function ActiveOutputDataNodesSet implys algname="<<algname<<", outdata="<<tmpDataNodePtr->GetName()<<std::endl;
        this->ActivateDataNode(tmpDataNodePtr->GetName());
    }
    //std::cout<<"Class ALPDAGTunnel function ActiveOutputDataNodesSet says byebye.algname="<<algname<<std::endl;
    return ALPReturnCode::SUCCESS;
}

//激活DAG算法节点
ALPReturnCode ALPDAGTunnel::ActivateAlgorithmNode(const std::string name)
{
    //std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode says hello. name="<<name<<std::endl;
    //设置算法结点状态为数据就绪
    m_AlgNodeNameToStatusMap[name]->SetAlgNodeState(ALPDAGAlgorithmNodeState::AlgState::DATAREADY);
/*    if(ALPReturnCode::FAILURE == m_AlgNodeNameToStatusMap[name]->SetAlgNodeState(ALPDAGAlgorithmNodeState::AlgState::DATAREADY) ) 
    {
        std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode error: alname="<<name<<" cant't go state dataready."<<std::endl;   
    } else {
        std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode algname="<<name<<" go state dataready."<<std::endl;
    }
*/
    auto activeAlgOutputDataNodesSet = [&, this, name]() {
        this->m_ActivateNodeAction_Queue.push([&, this, name]() {
            return this->ActiveOutputDataNodesSet(name);
        });
        return ALPReturnCode::SUCCESS;
    };
    //std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode step1."<<std::endl;

    //如果线程池处于正常工作状态
    if (ALP_ThreadPool_DisableSize != m_ThreadPool->GetPoolSize())
    {
        //std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode step2."<<std::endl;
        //为任务分配内存空间
        tbb::task* algtask = new (tbb::task::allocate_root()) ALPAlgorithmTask(m_AlgNodeNameToAlgInstanceMap[name], m_DAGTunnelDog->GetCurrentEventDataPtr(), activeAlgOutputDataNodesSet);
        //设置算法结点为运行状态
        m_AlgNodeNameToStatusMap[name]->SetAlgNodeState(ALPDAGAlgorithmNodeState::AlgState::RUNNING);

/*        if(ALPReturnCode::FAILURE == m_AlgNodeNameToStatusMap[name]->SetAlgNodeState(ALPDAGAlgorithmNodeState::AlgState::RUNNING) ) 
        {
             std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode error: alname="<<name<<" cant't go state running."<<std::endl;
        } else{
             std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode algname="<<name<<" go state running."<<std::endl;
        }
*/

        //调度算法对应任务
        tbb::task::enqueue(*algtask);
        //std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode step3."<<std::endl;
    }
    else  //若线程池被禁用
    {
        //std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode step4."<<std::endl;
        ALPAlgorithmTask* algtask = new ALPAlgorithmTask(m_AlgNodeNameToAlgInstanceMap[name], m_DAGTunnelDog->GetCurrentEventDataPtr(), activeAlgOutputDataNodesSet);
        //设置算法结点为运行状态
         m_AlgNodeNameToStatusMap[name]->SetAlgNodeState(ALPDAGAlgorithmNodeState::AlgState::RUNNING);

/*        if(ALPReturnCode::FAILURE == m_AlgNodeNameToStatusMap[name]->SetAlgNodeState(ALPDAGAlgorithmNodeState::AlgState::RUNNING) )   
        {
            std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode error: alname="<<name<<" cant't go state running."<<std::endl;
        } else{ 
            std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode algname="<<name<<" go state running."<<std::endl;
        } 
*/
        //执行计算工作
        algtask->execute();
        //std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode step5."<<std::endl;
    }
    //std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode step6.name="<<name<<std::endl;
    
    //avoid threads coflict: tunnel dog rest state initial, this thread set eventaccepted
    if(name == "ALPDAGTunnelEventEndAlg")
       return  ALPReturnCode::SUCCESS;    

    //算法执行成功，更新对应DAG算法结点状态
    m_AlgNodeNameToStatusMap[name]->SetAlgNodeState(ALPDAGAlgorithmNodeState::AlgState::EVTACCEPTED);

/*    if(ALPReturnCode::FAILURE == m_AlgNodeNameToStatusMap[name]->SetAlgNodeState(ALPDAGAlgorithmNodeState::AlgState::EVTACCEPTED)) 
    {
        std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode error: algname="<<name<<" cant't go state eventaccpted."<<std::endl;
    } else{
        std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode algname="<<name<<" go state eventaccpted."<<std::endl;
    }
*/
    //std::cout<<"Class ALPDAGTunnel function ActivateAlgorithmNode says byebye."<<std::endl;
    return ALPReturnCode::SUCCESS;
}



//申请算法实例指针
ALPReturnCode  ALPDAGTunnel::ApplyAlgorithmInstance(const std::string name) {
    LogDebug<<"Class ALPDAGTunnel Function ApplyAlgorithmInstance sasys hello."<<std::endl; 
    ALPAlgBase* tmpAlgPtr = NULL;
    m_AlgResourcePool->AllocateAlgorithmInstance(name, tmpAlgPtr);
    // set event end alg's tunnel ptr
    if("ALPDAGTunnelEventEndAlg" == name) {
         dynamic_cast<ALPDAGTunnelEventEndAlg*>(tmpAlgPtr)->SetDAGTunnelPtr(this);
    }

    m_AlgNodeNameToAlgInstanceMap[name] = tmpAlgPtr;
    LogDebug<<"Class ALPDAGTunnel Function  ApplyAlgorithmInstance says byebye."<<std::endl;
    return ALPReturnCode::SUCCESS;
}

//释放算法实例指针
ALPReturnCode  ALPDAGTunnel::ReleaseAlgorithmInstance(const std::string name) {
    LogDebug<<"Class ALPDAGTunnel Function ReleaseAlgorithmInstance says hello."<<std::endl;
    m_AlgResourcePool->RecycleAlgorithmInstance(name, m_AlgNodeNameToAlgInstanceMap[name]);
    m_AlgNodeNameToAlgInstanceMap.erase(name);
    LogDebug<<"Class ALPDAGTunnel Function ReleaseAlgorithmInstance says byebye."<<std::endl;
    return ALPReturnCode::SUCCESS;
}

//为所有算法申请实例
ALPReturnCode ALPDAGTunnel::ApplyAllAlgorithmInstances() {
    LogDebug<< "Function ALPDAGTunnel::ApplyAllAlgorithmInstances() says hello." << std::endl;
    for (auto algname : m_DAG->GetLeafAlgNodesNames()) {
        ALPAlgBase* tmpAlgPtr = NULL;
        LogDebug<< "Function ALPDAGTunnel::ApplyAllAlgorithmInstances() step1. algname="<<algname<<std::endl; 
        m_AlgResourcePool->AllocateAlgorithmInstance(algname, tmpAlgPtr);
        // set event end alg's tunnel ptr
        if("ALPDAGTunnelEventEndAlg" == algname) {
            dynamic_cast<ALPDAGTunnelEventEndAlg*>(tmpAlgPtr)->SetDAGTunnelPtr(this);
        }
        m_AlgNodeNameToAlgInstanceMap[algname] = tmpAlgPtr;
        LogDebug<< "Function ALPDAGTunnel::ApplyAllAlgorithmInstances() step2." << std::endl;
    }
    LogDebug<< "Function ALPDAGTunnel::ApplyAllAlgorithmInstances() says byebye." << std::endl;
    return ALPReturnCode::SUCCESS;
}

//释放所有算法实例
ALPReturnCode ALPDAGTunnel::ReleaseAllAlgorithmInstances() {
    LogDebug<<"Class ALPDAGTunnel Function ReleaseAllAlgorithmInstances says hello."<<std::endl;
    for (auto algname : m_DAG->GetLeafAlgNodesNames()) {
        m_AlgResourcePool->RecycleAlgorithmInstance(algname, m_AlgNodeNameToAlgInstanceMap[algname]);
        m_AlgNodeNameToAlgInstanceMap.erase(algname);
    }
    LogDebug<<"Class ALPDAGTunnel Function ReleaseAllAlgorithmInstances says byebye."<<std::endl;
    return ALPReturnCode::SUCCESS;
}


//初始化数据结点状态
ALPReturnCode ALPDAGTunnel::InitDataNodesStatus() {
    LogDebug<<"Class ALPDAGTunnel Function InitDataNodesStatus says hello."<<std::endl;
    std::vector<std::string> datanames = ALPDAG::GetDataNodesNames();
    //为空则返回失败码
    if (datanames.empty()) {
        LogError << "InitDataNodeStatus() function implys that no data nodes found! Error!" << std::endl;
        return ALPReturnCode::FAILURE;
    }
    //初始化为false
    for (unsigned int count = 0; count < datanames.size(); count++) {
        LogDebug<<"Class ALPDAGTunnel function InitDataNodesStatus implys datanode="<<datanames[count]<<std::endl;
        m_DataNodeNameToStatusMap[datanames[count]] = false;
    }
    LogDebug<<"Class ALPDAGTunnel Function InitDataNodesStatus says byebye."<<std::endl;
    return ALPReturnCode::SUCCESS;
}

//初始化算法结点的状态
ALPReturnCode ALPDAGTunnel::InitAlgNodesStatus() {
    LogDebug<<"Class ALPDAGTunnel Function InitAlgNodesStatus says hello."<<std::endl;
    std::vector<std::string> algsnames = ALPDAG::GetLeafAlgNodesNames();
    if (algsnames.empty()) {
        LogError << "InitAlgNodesStatus() function implys that no algorithm nodes found! Error!" << std::endl;
        return ALPReturnCode::FAILURE;
    }
    //算法结点状态的初始化
    for (unsigned int count = 0; count < algsnames.size(); count++) {
        m_AlgNodeNameToStatusMap[algsnames[count]] = new ALPDAGAlgorithmNodeState();
    }
    LogDebug<<"Class ALPDAGTunnel Function InitAlgNodesStatus says byebye."<<std::endl;
    return ALPReturnCode::SUCCESS;
}

//reset all algorithm node status and all data node status, each event invoke once, except the first event
ALPReturnCode ALPDAGTunnel::ResetAllDAGNodesStatus() {
    // reset data node status
    for(std::unordered_map<std::string, bool>::iterator tmpMapIterator = m_DataNodeNameToStatusMap.begin(); tmpMapIterator != m_DataNodeNameToStatusMap.end(); tmpMapIterator++) {
        tmpMapIterator->second=false;
    }

    // reset alg node status
    for(std::unordered_map<std::string, ALPDAGAlgorithmNodeState*>::iterator tmpMapIterator = m_AlgNodeNameToStatusMap.begin(); tmpMapIterator != m_AlgNodeNameToStatusMap.end(); tmpMapIterator++) {
        tmpMapIterator->second->ResetAlgNodeState();
    }
 
    return ALPReturnCode::SUCCESS;
}

