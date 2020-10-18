#include "AlgLevelParallel/ALPDAGTunnelDog.h"
#include "SniperKernel/SniperLog.h"
#include "AlgLevelParallel/ALPDAGTunnel.h"


ALPDAGTunnelDog::ALPDAGTunnelDog(ALPDAGTunnel* tmpDAGTunnel, ALPEventPool* tmpEventPool)
{
    //�ó�ֵ
    m_CurrentEventGroup = NULL;
    m_DAGTunnelPtr = tmpDAGTunnel;
    m_EventPool = tmpEventPool;

}

ALPDAGTunnelDog::~ALPDAGTunnelDog()
{
    //�������������Ļ���
    m_CurrentEventContext.ReSetEventContext();
    //�����ļ������Ļ���
    m_CurrentEventContext.ResetFileContext();
    
}

bool ALPDAGTunnelDog::initialize() {   // start event loop thread 
    //���߳�״̬���
    m_EventLoopThreadState = INACTIVE;
    LogInfo<<"Activating event loop thread......................"<<std::endl;
    //��������ѭ���߳�
    m_EventLoopThread = std::thread([this]() {this->ActivateTunnelEventLoopThread(); });
    //�ȴ��̼߳���
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

//����ͨ������ѭ���̣߳����𲻶ϵĴ��������л�ȡ������
void ALPDAGTunnelDog::ActivateTunnelEventLoopThread() {
    //��������ѭ���߳�״̬
    m_EventLoopThreadState = ACTIVE;
    //����ѭ���߳̿�ʼ��DAGTunnel ��ѹ������
    LogInfo << "The Event Loop thread starts working......" << std::endl;
    
    while (ACTIVE == m_EventLoopThreadState)
    {
        if (NULL == m_CurrentEventGroup) {
            //����������δ����,�������µ�������
            //����ʧ��ʱ��˵�������������Ѿ�����������׼���˳�
            if (ALPReturnCode::FAILURE == RequestReadyStateEventGroup()) {
                //ֹͣͨ������ѭ���߳�
                DeactivateTunnelEventLoopThread();
            }
            else {
                //ѹ���µ��������ȴ�Tunnel�������
                PushNewEventInTunnel();

                //������ǰ����
                PopCurrentEventFromTunnel();
            }

        }else if (m_CurrentEventGroup->IsProcessed()) {
            //���������Ѿ��������,�򵯳��������飬�������µ�������
            PopCurrentEventGroup();
            //����ʧ��ʱ��˵�������������Ѿ�����������׼���˳�
            if (ALPReturnCode::FAILURE == RequestReadyStateEventGroup()) {
                //ֹͣͨ������ѭ���߳�
                DeactivateTunnelEventLoopThread();
            }
            else {
                //ѹ���µ��������ȴ�Tunnel�������
                PushNewEventInTunnel();

                //������ǰ����
                PopCurrentEventFromTunnel();
            }  
            
        }else
        {
            //���������Ѿ���������δ�������,��ѹ���µ��������ȴ�Tunnel�������
            PushNewEventInTunnel();

            //������ǰ����
            PopCurrentEventFromTunnel();
        }
        
    }
}

//ֹͣͨ������ѭ���߳�
ALPReturnCode ALPDAGTunnelDog::DeactivateTunnelEventLoopThread() {
    if (ACTIVE == m_EventLoopThreadState) {
        LogInfo << "The Event Loop thread stop working......" << std::endl;
        m_EventLoopThreadState = INACTIVE;
    }
    return ALPReturnCode::SUCCESS;
}

//���봦��Ready״̬��������
ALPReturnCode ALPDAGTunnelDog::RequestReadyStateEventGroup() {
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() says hello."<<std::endl;
    //�����µ�������
    m_CurrentEventGroup = m_EventPool->AllocateReadyEventGroup();
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() step1."<<std::endl;
    //���ΪNULL,��˵�������������Ѿ�����������׼���˳�
    if (NULL == m_CurrentEventGroup)
        return ALPReturnCode::FAILURE;
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() step2."<<std::endl;
    //����ļ����������������ļ������Ļ���
    if (m_CurrentEventContext.FileOccurAlteration(m_CurrentEventGroup->GetCurrentFileName())) {
        m_CurrentEventContext.SetFileContext(m_CurrentEventGroup->GetCurrentFileName());
    }
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() step3."<<std::endl;
    return ALPReturnCode::SUCCESS;
}

//�����Ѿ�������ɵ�������
ALPReturnCode ALPDAGTunnelDog::PopCurrentEventGroup() {
    LogDebug<<"Class ALPDAGTunnelDog function PopCurrentEventGroup says hello."<<std::endl;
    //�����������״̬
    m_CurrentEventGroup->SetGroupState(EventIOQueueStream_GroupState_Processed);
    //��ǰ�����鴦����ɣ�����Processed״̬��������Ŀ���ź������Թ��ļ�����߳�ʹ��
    m_EventPool->PushProcessedEventGroupSemaphore();

    return ALPReturnCode::SUCCESS;
}

//�����㷨ʵ������DAG Tunnel��ѹ���µ�����
ALPReturnCode ALPDAGTunnelDog::PushNewEventInTunnel() {
    //�������������Ļ���
    //LogDebug<<"Class ALPDAGTunnelDog function PushNewEventInTunnel says hello."<<std::endl;
    m_CurrentEventContext.SetCurrentEventContext(m_CurrentEventGroup->EventToProcess());
    //LogDebug<<"Class ALPDAGTunnelDog function PushNewEventInTunnel step1."<<std::endl;
 
    //�����Ѿ�ѹ�룬��ʼ����ǰ����
    m_DAGTunnelPtr->BeginEventProcessing(m_DAGTunnelPtr);
    //LogDebug<<"Class ALPDAGTunnelDog function PushNewEventInTunnel step2."<<std::endl; 

    return ALPReturnCode::SUCCESS;
}

//�ͷŲ����㷨ʵ������Tunnel�е����Ѿ�������ɵ�����
ALPReturnCode ALPDAGTunnelDog::PopCurrentEventFromTunnel() {
    //LogDebug<<"Class ALPDAGTunnelDog function PopCurrentEventFromTunnel says hello."<<std::endl;
    //�ȴ�DAG Tunnel ��ɵ�ǰ�����Ĵ���
    m_DAGTunnelPtr->m_TunnelEventEnd_Semaphore.wait();
    //LogDebug<<"Class ALPDAGTunnelDog function PopCurrentEventFromTunnel will pop event now."<<std::endl;
 
    //�������������Ļ���
    m_CurrentEventContext.ReSetEventContext();
    //reset all DAG nodes status
    m_DAGTunnelPtr->ResetAllDAGNodesStatus();

    return ALPReturnCode::SUCCESS;
}


//���ָ��ǰ�������ݵ�ָ��
void* ALPDAGTunnelDog::GetCurrentEventDataPtr() {
    //LogDebug<<"Class ALPDAGTunnelDog function GetCurrentEventDataPtr says hello."<<std::endl;
    return m_CurrentEventContext.GetEventDataPtr();
}



