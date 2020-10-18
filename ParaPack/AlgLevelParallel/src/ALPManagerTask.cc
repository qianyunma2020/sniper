#include "AlgLevelParallel/ALPManagerTask.h"
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/ALPFileIOSvc.h"
//#include "SniperKernel/DeclareDLE.h"

//ALPCustomParameterSvc ALPManagerTask::m_CustomParameterSvc("ALPCustomParameterSvc");


//SNIPER_DECLARE_DLE(ALPManagerTask);

ALPManagerTask::ALPManagerTask(const std::string& name) : Task(name)
{
	//��������
        LogDebug<<"Function ALPManagerTask::ALPManagerTask() step 1."<<std::endl;
	//�����㷨��Դ��
	m_AlgResourcePool = new ALPAlgResourcePool("AlgResourcePool");
        m_AlgResourcePool->setLogLevel(4);
        LogDebug<<"Function ALPManagerTask::ALPManagerTask() step 2."<<std::endl;
	//�����̳߳�
	m_ThreadPool = new ALPThreadPool("ThreadPool");
        m_ThreadPool->setLogLevel(4);
        LogDebug<<"Function ALPManagerTask::ALPManagerTask() step 3."<<std::endl;
       
	m_EventPool = new ALPEventPool("EventPool");
        m_EventPool->setLogLevel(4);
        LogDebug<<"Function ALPManagerTask::ALPManagerTask() step 4."<<std::endl;
	//����DAG
	m_DAG = new ALPDAG();
        LogDebug<<"Function ALPManagerTask::ALPManagerTask() step 5."<<std::endl;
}

ALPManagerTask::~ALPManagerTask()
{
        LogDebug<<"Function ALPManagerTask::~ALPManagerTask() says hello."<<std::endl; 

	//�ͷŸ���DAG Tunnel
	for (auto tunnel : m_DAGTunnelsVector) {
		delete tunnel;
	}
	m_DAGTunnelsVector.clear();
        m_CustomParameterSvc->SetDAGTunnelsCount(0);
	//�ͷ��㷨��Դ��
	delete m_AlgResourcePool;
	//�ͷ��̳߳�
	delete m_ThreadPool;
	//�ͷ�������
	delete m_EventPool;
	//�ͷ�DAG
	delete m_DAG;
        LogDebug<<"Function ALPManagerTask::~ALPManagerTask() says see you next job."<<std::endl;
}

bool ALPManagerTask::config() {
	return true;
}

bool ALPManagerTask::initialize() {

    LogDebug << "Function ALPManagerTask::initialize() step 1." <<std::endl; 
    m_CustomParameterSvc = static_cast<ALPCustomParameterSvc*>(find("ALPCustomParameterSvc"));

    //��ʼ���㷨��Դ��
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

    //��ʼ���̳߳�
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

    //����������
    if(m_CustomParameterSvc->GetEventPoolGroupCount() < EventPool_GroupDefaultCount)
    {    m_CustomParameterSvc->SetEventPoolGroupCount(EventPool_GroupDefaultCount); }

    //��������������������Ŀ,Ȼ���ʼ��������
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


    //����Json�ļ�����ȫ��DAG
    if (ALPReturnCode::SUCCESS == m_DAG->BuildDAG(m_CustomParameterSvc->GetAlgsPropertyFieName())) {
    		LogInfo << "DAG is already build." << std::endl;
    }else
    {
		LogError << "DAG builds failed!" << std::endl;
		return false;
    }
    LogDebug << "Function ALPManagerTask::initialize() step 6." <<std::endl;
	
    //����ָ����Ŀ��DAG Tunnel����ʼ��
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
	//���ջ�����DAG Tunnel
	for (auto tunnel : m_DAGTunnelsVector) {
		if (!tunnel->Snoopy().finalize()) {
			LogError << "DAG Tunnel finalize failed!" << std::endl;
			return false;
		}
	}
	//���ջ��̳߳�
	if (!m_ThreadPool->Snoopy().finalize()) {
		LogError << "Thread pool finalize failed!" << std::endl;
		return false;
	}
	//���ջ��㷨��Դ��
	if (!m_AlgResourcePool->Snoopy().finalize()) {
		LogError << "AlgResource pool finalize failed!" << std::endl;
		return false;
	}
	//���ջ�������
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
