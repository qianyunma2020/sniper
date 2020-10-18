#ifndef SNIPER_ALP_DAG_TUNNEL_H
#define SNIPER_ALP_DAG_TUNNEL_H

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <vector>
//#include <map>
#include <unordered_map>
#include <tbb/concurrent_queue.h>
#include "SniperKernel/Task.h"
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/ALPAlgBase.h"
#include "SniperKernel/ALPDAGAlgorithmNodeState.h"
#include "SniperKernel/ALPEventPool.h"
#include "ALPDAG.h"
#include "ALPDAGTunnelDog.h"
#include "ALPDAGTunnelEventEndAlg.h"
#include "ALPAlgResourcePool.h"
#include "ALPThreadPool.h"



class ALPDAGTunnel : public Task
{
public:
	ALPDAGTunnel(const std::string& name, unsigned int tunnelID, ALPDAG* tmpDAG, ALPThreadPool* tmpThreadPool, ALPAlgResourcePool* tmpAlgResourcePool, ALPEventPool* tmpEventPool);
	~ALPDAGTunnel();
	//返回DAG通道的ID号
	unsigned int GetTunnelID() { return m_DAGTunnelID; }

public:
	//run this task
	bool run() override final { return true; }
	//启动
	bool start() { return true; }
	//停止
	bool stop() override final { return true; }
protected:
	//the concrete task operations
	bool config() override final { return true; }
	bool execute() override final { return true; }
	//clear all svcs and algs
	void reset() { }

	bool initialize() override final;
	bool finalize() override final;
	


private:
	/*DAGTunnel中数据结点激活操作和算法结点激活操作构成一个操作队列，
	  DAGTunnelScheduler线程用于管理此队列，
	  此函数用于激活DAGTunnelScheduler线程。
	*/
	void ActivateDAGTunnelSchedulerThread();
	//停止DAGTunnelScheduler线程
	ALPReturnCode DeactivateDAGTunnelSchedulerThread();

	/*DAG处理流程为：
	  <1>数据就绪，激活数据结点，查看依赖此数据的算法是否可以执行
	  <2>将算法结点对应的算法,通过tbb::task::enqueue()方法封装，提交到tbb,tbb会执行ALPAlgorithmTask::execute()
	  <3>ALPAlgorithmTask::execute()执行完计算工作后，激活输出数据对应的DAG数据结点
	  上述3个步骤循环工作，直至当前事例处理完成。
	*/
	ALPReturnCode ActivateDataNode(const std::string name);        //激活DAG数据结点
	ALPReturnCode ActiveOutputDataNodesSet(const std::string algname);     //激活特定算法输出数据集中的所有数据结点
	ALPReturnCode ActivateAlgorithmNode(const std::string name);    //激活DAG算法节点

        //set flag m_SchedulerIsActivate
        void SetSchedulerINActive() { m_SchedulerIsActivate = INACTIVE; }   

private:
	unsigned int m_DAGTunnelID;                //DAG tunnel ID号
	ALPDAG* m_DAG;                             //指向DAG的指针
	ALPThreadPool* m_ThreadPool;               //指向线程池的指针
	ALPAlgResourcePool* m_AlgResourcePool;     //指向算法资源池的指针
	ALPEventPool* m_EventPool;                 //指向事例池的指针


	std::thread m_UpdateNodeStateThread;            //负责更新各个算法数据节点的状态
	tbb::concurrent_bounded_queue<std::function< ALPReturnCode() >> m_ActivateNodeAction_Queue;  //激活DAG结点操作队列
	enum SchedulerState { INACTIVE = 0, ACTIVE = 1, ERROR = 2 };   //DAGTunnelScheduler调度器状态
	std::atomic<SchedulerState> m_SchedulerIsActivate;   //标记DAGTunnelScheduler调度器是否激活或错误

	ALPDAGTunnelDog* m_DAGTunnelDog;                //DAG通道的看门狗

public:
	//初始化数据结点状态
	ALPReturnCode InitDataNodesStatus();
	//初始化算法结点的状态
	ALPReturnCode InitAlgNodesStatus();
        //reset all algorithm node status and all data node status, each event invoke once, except the first event
        ALPReturnCode ResetAllDAGNodesStatus();        

public:
	//激活DAG中事例开始的数据结点
	ALPReturnCode BeginEventProcessing(ALPDAGTunnel* tmpDAGTunnel);
	//DAG通道事例处理结束的信号量
	GBSemaphore m_TunnelEventEnd_Semaphore;         

public:
	//查询算法实例指针
	ALPAlgBase* QueryAlgorithmInstance(const std::string name) const { return m_AlgNodeNameToAlgInstanceMap.at(name); }
	//申请算法实例
	ALPReturnCode  ApplyAlgorithmInstance(const std::string name);
	//释放算法实例
	ALPReturnCode  ReleaseAlgorithmInstance(const std::string name);

private:
	//为所有算法申请实例
	ALPReturnCode ApplyAllAlgorithmInstances();
	//释放所有算法实例
	ALPReturnCode ReleaseAllAlgorithmInstances();

private:
	//算法结点名称和算法实例指针的映射
	std::unordered_map<std::string, ALPAlgBase*> m_AlgNodeNameToAlgInstanceMap;   
	//算法结点名称到状态的映射
	std::unordered_map<std::string, ALPDAGAlgorithmNodeState*> m_AlgNodeNameToStatusMap;
	//数据结点名称到数据结点状态的映射,数据尚未就绪为false,数据已就绪为true
	std::unordered_map<std::string, bool> m_DataNodeNameToStatusMap; 
	//data name to status map lock, prevent algorithm repeat execute 
	GBSemaphore  m_DataNodeNameToStatusMap_Lock;
	//通道是否处于工作状态
	std::atomic_bool m_BusyFlag;


private:
	ALPDAGTunnel(const std::string& name);
};


#endif
