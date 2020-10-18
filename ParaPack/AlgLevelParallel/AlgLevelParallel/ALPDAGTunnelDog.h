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
        //获得指向当前事例数据的指针
        void* GetCurrentEventDataPtr();

private:        
	//申请处于Ready状态的事例组
	ALPReturnCode RequestReadyStateEventGroup();
	//弹出已经处理完成的事例组
	ALPReturnCode PopCurrentEventGroup();

	//申请算法实例，向DAG Tunnel中压入新的事例
	ALPReturnCode PushNewEventInTunnel();
	//释放部分算法实例，从Tunnel中弹出已经处理完成的事例
	ALPReturnCode PopCurrentEventFromTunnel();

       //激活通道事例循环线程，负责不断的从事例池中获取事例组
       void ActivateTunnelEventLoopThread();
       //停止通道事例循环线程
       ALPReturnCode DeactivateTunnelEventLoopThread();


private:
	//指向事例池
	ALPEventPool* m_EventPool;
	//指向DAGTunnel的指针
	ALPDAGTunnel* m_DAGTunnelPtr;
	//DAG Tunnel 当前的事例组指针
	ALPEventGroup* m_CurrentEventGroup;
	//指向当前的事例上下文
	ALPDAGTunnelEventContext  m_CurrentEventContext;
	//事例循环线程
	std::thread m_EventLoopThread;
	//DAGTunnelDog事例循环线程状态
	enum EventLoopThreadState { INACTIVE = 0, ACTIVE = 1, ERROR = 2 };   
	//标记事例循环线程是否激活或错误
	std::atomic<EventLoopThreadState> m_EventLoopThreadState;   
};


#endif
