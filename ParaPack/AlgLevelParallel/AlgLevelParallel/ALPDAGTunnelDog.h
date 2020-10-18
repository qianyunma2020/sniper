#ifndef SNIPER_ALP_DAG_TUNNEL_DOG_H
#define SNIPER_ALP_DAG_TUNNEL_DOG_H

#include <memory>
#include <thread>
#include "SniperKernel/ALPReturnCode.h"
#include "ALPDAGTunnelEventContext.h"
#include "SniperKernel/ALPEventPool.h"

class ALPDAGTunnel;

class ALPDAGTunnelDog final
{
public:
	ALPDAGTunnelDog(ALPDAGTunnel* tmpDAGTunnel, ALPEventPool* tmpEventPool);
	~ALPDAGTunnelDog();

public:
        bool initialize();   // start event loop thread 
        bool finalize();     //nothing  
        //���ָ��ǰ�������ݵ�ָ��
        void* GetCurrentEventDataPtr();

private:        
	//���봦��Ready״̬��������
	ALPReturnCode RequestReadyStateEventGroup();
	//�����Ѿ�������ɵ�������
	ALPReturnCode PopCurrentEventGroup();

	//�����㷨ʵ������DAG Tunnel��ѹ���µ�����
	ALPReturnCode PushNewEventInTunnel();
	//�ͷŲ����㷨ʵ������Tunnel�е����Ѿ�������ɵ�����
	ALPReturnCode PopCurrentEventFromTunnel();

       //����ͨ������ѭ���̣߳����𲻶ϵĴ��������л�ȡ������
       void ActivateTunnelEventLoopThread();
       //ֹͣͨ������ѭ���߳�
       ALPReturnCode DeactivateTunnelEventLoopThread();


private:
	//ָ��������
	ALPEventPool* m_EventPool;
	//ָ��DAGTunnel��ָ��
	ALPDAGTunnel* m_DAGTunnelPtr;
	//DAG Tunnel ��ǰ��������ָ��
	ALPEventGroup* m_CurrentEventGroup;
	//ָ��ǰ������������
	ALPDAGTunnelEventContext  m_CurrentEventContext;
	//����ѭ���߳�
	std::thread m_EventLoopThread;
	//DAGTunnelDog����ѭ���߳�״̬
	enum EventLoopThreadState { INACTIVE = 0, ACTIVE = 1, ERROR = 2 };   
	//�������ѭ���߳��Ƿ񼤻�����
	std::atomic<EventLoopThreadState> m_EventLoopThreadState;   
};


#endif
