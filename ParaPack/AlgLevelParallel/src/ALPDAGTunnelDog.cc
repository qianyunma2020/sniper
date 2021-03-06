#include "AlgLevelParallel/ALPDAGTunnelDog.h"
#include "SniperKernel/SniperLog.h"
#include "AlgLevelParallel/ALPDAGTunnel.h"


ALPDAGTunnelDog::ALPDAGTunnelDog(ALPDAGTunnel* tmpDAGTunnel, ALPEventPool* tmpEventPool)
{
    //置初值
    m_CurrentEventGroup = NULL;
    m_DAGTunnelPtr = tmpDAGTunnel;
    m_EventPool = tmpEventPool;

}

ALPDAGTunnelDog::~ALPDAGTunnelDog()
{
    //重设事例上下文环境
    m_CurrentEventContext.ReSetEventContext();
    //重设文件上下文环境
    m_CurrentEventContext.ResetFileContext();
    
}

bool ALPDAGTunnelDog::initialize() {   // start event loop thread 
    //置线程状态初�
    m_EventLoopThreadState = INACTIVE;
    LogInfo<<"Activating event loop thread......................"<<std::endl;
    //启动事例循环线程
    m_EventLoopThread = std::thread([this]() {this->ActivateTunnelEventLoopThread(); });
    //等待线程激活
    while (ACTIVE != m_EventLoopThreadState)
    {
        if (ERROR == m_EventLoopThreadState) {
            LogError << "Function ALPDAGTunnelDog::ALPDAGTunnelDog() implys flag m_EventLoopThreadState is ERROR!" << std::endl;
        }
        LogInfo << "Waiting for thread to active............." << std::endl;
        sleep(1);
    }
    return true;
}

bool ALPDAGTunnelDog::finalize() {     //nothing 
    return true;
}

//激活通道事例循环线程，负责不断的从事例池中获取事例组
void ALPDAGTunnelDog::ActivateTunnelEventLoopThread() {
    //更新事例循环线程状态
    m_EventLoopThreadState = ACTIVE;
    //事例循环线程开始向DAGTunnel 中压入事例
    LogInfo << "The Event Loop thread starts working......" << std::endl;
    
    while (ACTIVE == m_EventLoopThreadState)
    {
        if (NULL == m_CurrentEventGroup) {
            //若事例组尚未分配,则申请新的事例组
            //申请失败时，说明事例处理工作已经结束，程序准备退出
            if (ALPReturnCode::FAILURE == RequestReadyStateEventGroup()) {
                //停止通道事例循环线程
                DeactivateTunnelEventLoopThread();
            }
            else {
                //压入新的事例并等待Tunnel处理完成
                PushNewEventInTunnel();

                //弹出当前事例
                PopCurrentEventFromTunnel();
            }

        }else if (m_CurrentEventGroup->IsProcessed()) {
            //若事例组已经处理完成,则弹出该事例组，并分配新的事例组
            PopCurrentEventGroup();
            //申请失败时，说明事例处理工作已经结束，程序准备退出
            if (ALPReturnCode::FAILURE == RequestReadyStateEventGroup()) {
                //停止通道事例循环线程
                DeactivateTunnelEventLoopThread();
            }
            else {
                //压入新的事例并等待Tunnel处理完成
                PushNewEventInTunnel();

                //弹出当前事例
                PopCurrentEventFromTunnel();
            }  
            
        }else
        {
            //若事例组已经分配且尚未处理完成,则压入新的事例并等待Tunnel处理完成
            PushNewEventInTunnel();

            //弹出当前事例
            PopCurrentEventFromTunnel();
        }
        
    }
}

//停止通道事例循环线程
ALPReturnCode ALPDAGTunnelDog::DeactivateTunnelEventLoopThread() {
    if (ACTIVE == m_EventLoopThreadState) {
        LogInfo << "The Event Loop thread stop working......" << std::endl;
        m_EventLoopThreadState = INACTIVE;
    }
    return ALPReturnCode::SUCCESS;
}

//申请处于Ready状态的事例组
ALPReturnCode ALPDAGTunnelDog::RequestReadyStateEventGroup() {
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() says hello."<<std::endl;
    //申请新的事例组
    m_CurrentEventGroup = m_EventPool->AllocateReadyEventGroup();
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() step1."<<std::endl;
    //如果为NULL,则说明事例处理工作已经结束，程序准备退出
    if (NULL == m_CurrentEventGroup)
        return ALPReturnCode::FAILURE;
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() step2."<<std::endl;
    //如果文件发生变更，则更新文件上下文环境
    if (m_CurrentEventContext.FileOccurAlteration(m_CurrentEventGroup->GetCurrentFileName())) {
        m_CurrentEventContext.SetFileContext(m_CurrentEventGroup->GetCurrentFileName());
    }
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() step3."<<std::endl;
    return ALPReturnCode::SUCCESS;
}

//弹出已经处理完成的事例组
ALPReturnCode ALPDAGTunnelDog::PopCurrentEventGroup() {
    LogDebug<<"Class ALPDAGTunnelDog function PopCurrentEventGroup says hello."<<std::endl;
    //更新事例组的状态
    m_CurrentEventGroup->SetGroupState(EventIOQueueStream_GroupState_Processed);
    //当前事例组处理完成，更新Processed状态事例组数目的信号量，以供文件输出线程使用
    m_EventPool->PushProcessedEventGroupSemaphore();

    return ALPReturnCode::SUCCESS;
}

//申请算法实例，向DAG Tunnel中压入新的事例
ALPReturnCode ALPDAGTunnelDog::PushNewEventInTunnel() {
    //更新事例上下文环境
    //LogDebug<<"Class ALPDAGTunnelDog function PushNewEventInTunnel says hello."<<std::endl;
    m_CurrentEventContext.SetCurrentEventContext(m_CurrentEventGroup->EventToProcess());
    //LogDebug<<"Class ALPDAGTunnelDog function PushNewEventInTunnel step1."<<std::endl;
 
    //事例已经压入，开始处理当前事例
    m_DAGTunnelPtr->BeginEventProcessing(m_DAGTunnelPtr);
    //LogDebug<<"Class ALPDAGTunnelDog function PushNewEventInTunnel step2."<<std::endl; 

    return ALPReturnCode::SUCCESS;
}

//释放部分算法实例，从Tunnel中弹出已经处理完成的事例
ALPReturnCode ALPDAGTunnelDog::PopCurrentEventFromTunnel() {
    //LogDebug<<"Class ALPDAGTunnelDog function PopCurrentEventFromTunnel says hello."<<std::endl;
    //等待DAG Tunnel 完成当前事例的处理
    m_DAGTunnelPtr->m_TunnelEventEnd_Semaphore.wait();
    //LogDebug<<"Class ALPDAGTunnelDog function PopCurrentEventFromTunnel will pop event now."<<std::endl;
 
    //重设事例上下文环境
    m_CurrentEventContext.ReSetEventContext();
    //reset all DAG nodes status
    m_DAGTunnelPtr->ResetAllDAGNodesStatus();

    return ALPReturnCode::SUCCESS;
}


//获得指向当前事例数据的指针
void* ALPDAGTunnelDog::GetCurrentEventDataPtr() {
    //LogDebug<<"Class ALPDAGTunnelDog function GetCurrentEventDataPtr says hello."<<std::endl;
    return m_CurrentEventContext.GetEventDataPtr();
}



