#include "AlgLevelParallel/ALPManagerTask.h"
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/ALPFileIOSvc.h"
//#include "SniperKernel/DeclareDLE.h"

//ALPCustomParameterSvc ALPManagerTask::m_CustomParameterSvc("ALPCustomParameterSvc");


//SNIPER_DECLARE_DLE(ALPManagerTask);

ALPManagerTask::ALPManagerTask(const std::string& name) : Task(name)
{
	//设置属性
        LogDebug<<"Function ALPManagerTask::ALPManagerTask() step 1."<<std::endl;
	//创建算法资源池
	m_AlgResourcePool = new ALPAlgResourcePool("AlgResourcePool");
        m_AlgResourcePool->setLogLevel(4);
        LogDebug<<"Function ALPManagerTask::ALPManagerTask() step 2."<<std::endl;
	//创建线程池
	m_ThreadPool = new ALPThreadPool("ThreadPool");
        m_ThreadPool->setLogLevel(4);
        LogDebug<<"Function ALPManagerTask::ALPManagerTask() step 3."<<std::endl;
       
	m_EventPool = new ALPEventPool("EventPool");
        m_EventPool->setLogLevel(4);
        LogDebug<<"Function ALPManagerTask::ALPManagerTask() step 4."<<std::endl;
	//创建DAG
	m_DAG = new ALPDAG();
        LogDebug<<"Function ALPManagerTask::ALPManagerTask() step 5."<<std::endl;
}

ALPManagerTask::~ALPManagerTask()
{
        LogDebug<<"Function ALPManagerTask::~ALPManagerTask() says hello."<<std::endl; 

	//释放各个DAG Tunnel
	for (auto tunnel : m_DAGTunnelsVector) {
		delete tunnel;
	}
	m_DAGTunnelsVector.clear();
        m_CustomParameterSvc->SetDAGTunnelsCount(0);
	//释放算法资源池
	delete m_AlgResourcePool;
	//释放线程池
	delete m_ThreadPool;
	//释放事例池
	delete m_EventPool;
	//释放DAG
	delete m_DAG;
        LogDebug<<"Function ALPManagerTask::~ALPManagerTask() says see you next job."<<std::endl;
}

bool ALPManagerTask::config() {
	return true;
}

bool ALPManagerTask::initialize() {

    LogDebug << "Function ALPManagerTask::initialize() step 1." <<std::endl; 
    m_CustomParameterSvc = static_cast<ALPCustomParameterSvc*>(find("ALPCustomParameterSvc"));

    //初始化算法资源池
    m_AlgResourcePool->setAlgsPropertyFileName(m_CustomParameterSvc->GetAlgsPropertyFieName());
    if(m_AlgResourcePool->Snoopy().config()) {
        if (m_AlgResourcePool->Snoopy().initialize()) {
	    	LogInfo << "Algorithm resources pool initialize successful." << std::endl;
        }else
        {
	    	LogError << "Algorithm resources pool initialize failed!" << std::endl;
		return false;
        }
    }
    else{
               LogError << "Algorithm resources pool config failed!" << std::endl;
                return false;   
    }
    LogDebug << "Function ALPManagerTask::initialize() step 2." <<std::endl;

    //初始化线程池
    if(m_CustomParameterSvc->GetThreadPoolThreadsCount() > m_CustomParameterSvc->GetDAGTunnelsCount())
        {  m_ThreadPool->SetPoolSize(m_CustomParameterSvc->GetThreadPoolThreadsCount());  } 
    else if(ALP_ThreadPool_DisableSize == m_CustomParameterSvc->GetThreadPoolThreadsCount())
        {  m_ThreadPool->SetPoolSize(m_CustomParameterSvc->GetThreadPoolThreadsCount());  }
    else
        {  m_ThreadPool->SetPoolSize(m_CustomParameterSvc->GetDAGTunnelsCount()); } 
    if( m_ThreadPool->Snoopy().config() ) {
        if (m_ThreadPool->Snoopy().initialize()) {
		LogInfo << "Thread pool initialize successful." << std::endl;
        }else
        {
		LogError << "Thread pool initialize failed!" << std::endl;
		return false;
        }
    }
    else{
               LogError << "Thread pool config failed!" << std::endl;
                return false;
    }
    LogDebug << "Function ALPManagerTask::initialize() step 3." <<std::endl;

    //创建事例池
    if(m_CustomParameterSvc->GetEventPoolGroupCount() < EventPool_GroupDefaultCount)
    {    m_CustomParameterSvc->SetEventPoolGroupCount(EventPool_GroupDefaultCount); }

    //设置事例池中事例组数目,然后初始化事例池
    m_EventPool->InitRelatedVar(m_CustomParameterSvc->GetEventPoolGroupCount(), m_CustomParameterSvc->GetInputThreadsCount(), 1, m_CustomParameterSvc->GetInputFilesNameString(), m_CustomParameterSvc->GetOutputFilesNameString(), m_CustomParameterSvc->GetCollectionNamesFilterString());
    m_EventPool->InitSvcVar(dynamic_cast<ALPFileIOSvc*>(find("ALPFileInputSvc")), dynamic_cast<ALPFileIOSvc*>(find("ALPFileOutputSvc")));
    LogDebug << "Function ALPManagerTask::initialize() step 4." <<std::endl;
    if( m_EventPool->Snoopy().config() ) {
        if (m_EventPool->Snoopy().initialize()) {
	  	    LogInfo << "Event pool initialize successfull." << std::endl;
        }else{
		    LogError << "Event pool initialize failed!" << std::endl;
		    return false;
        }
    }
    else{
               LogError << "Event pool config failed!" << std::endl;
                return false;
    }
    LogDebug << "Function ALPManagerTask::initialize() step 5." <<std::endl;


    //根据Json文件建立全局DAG
    if (ALPReturnCode::SUCCESS == m_DAG->BuildDAG(m_CustomParameterSvc->GetAlgsPropertyFieName())) {
    		LogInfo << "DAG is already build." << std::endl;
    }else
    {
		LogError << "DAG builds failed!" << std::endl;
		return false;
    }
    LogDebug << "Function ALPManagerTask::initialize() step 6." <<std::endl;
	
    //创建指定数目的DAG Tunnel并初始化
    for (unsigned int count = 0; count < m_CustomParameterSvc->GetDAGTunnelsCount(); count++)
    {
        ALPDAGTunnel* tmpDAGTunnel = new ALPDAGTunnel("DAGTunnel" + std::to_string(count), count, m_DAG, m_ThreadPool, m_AlgResourcePool, m_EventPool);

        tmpDAGTunnel->setLogLevel(4);
        LogDebug << "Function ALPManagerTask::initialize() step 6.1." <<std::endl;
        if(tmpDAGTunnel->Snoopy().config() ) {
	    if (!tmpDAGTunnel->Snoopy().initialize()) {
	    	LogError << "DAG tunnel initialize failed!" << std::endl;
		return false;
	    }
        }
        else{
                LogError << "DAG Tunnel  config failed!" << std::endl;
                return false;
        }
        LogDebug << "Function ALPManagerTask::initialize() step 6.2" <<std::endl;

	m_DAGTunnelsVector.push_back(tmpDAGTunnel);
    }
    LogDebug << "Function ALPManagerTask::initialize() step 7." <<std::endl;

    LogInfo << "Manager task initialize successful!" << std::endl;
    return true;
}


bool ALPManagerTask::finalize() {
	//最终化各个DAG Tunnel
	for (auto tunnel : m_DAGTunnelsVector) {
		if (!tunnel->Snoopy().finalize()) {
			LogError << "DAG Tunnel finalize failed!" << std::endl;
			return false;
		}
	}
	//最终化线程池
	if (!m_ThreadPool->Snoopy().finalize()) {
		LogError << "Thread pool finalize failed!" << std::endl;
		return false;
	}
	//最终化算法资源池
	if (!m_AlgResourcePool->Snoopy().finalize()) {
		LogError << "AlgResource pool finalize failed!" << std::endl;
		return false;
	}
	//最终化事例池
	if (!m_EventPool->Snoopy().finalize()) {
		LogError << "Event pool finalize failed!" << std::endl;
	}
	
	LogInfo << "Manager task finalize successful." << std::endl;
	return true;
}


bool ALPManagerTask::run() {
   initialize();
   global_ALPManagerTask_JobRunOver_Semaphore.wait();
   finalize(); 
}
