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
	//����DAGͨ����ID��
	unsigned int GetTunnelID() { return m_DAGTunnelID; }

public:
	//run this task
	bool run() override final { return true; }
	//����
	bool start() { return true; }
	//ֹͣ
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
	/*DAGTunnel�����ݽ�㼤��������㷨��㼤���������һ���������У�
	  DAGTunnelScheduler�߳����ڹ���˶��У�
	  �˺������ڼ���DAGTunnelScheduler�̡߳�
	*/
	void ActivateDAGTunnelSchedulerThread();
	//ֹͣDAGTunnelScheduler�߳�
	ALPReturnCode DeactivateDAGTunnelSchedulerThread();

	/*DAG��������Ϊ��
	  <1>���ݾ������������ݽ�㣬�鿴���������ݵ��㷨�Ƿ����ִ��
	  <2>���㷨����Ӧ���㷨,ͨ��tbb::task::enqueue()������װ���ύ��tbb,tbb��ִ��ALPAlgorithmTask::execute()
	  <3>ALPAlgorithmTask::execute()ִ������㹤���󣬼���������ݶ�Ӧ��DAG���ݽ��
	  ����3������ѭ��������ֱ����ǰ����������ɡ�
	*/
	ALPReturnCode ActivateDataNode(const std::string name);        //����DAG���ݽ��
	ALPReturnCode ActiveOutputDataNodesSet(const std::string algname);     //�����ض��㷨������ݼ��е��������ݽ��
	ALPReturnCode ActivateAlgorithmNode(const std::string name);    //����DAG�㷨�ڵ�

        //set flag m_SchedulerIsActivate
        void SetSchedulerINActive() { m_SchedulerIsActivate = INACTIVE; }   

private:
	unsigned int m_DAGTunnelID;                //DAG tunnel ID��
	ALPDAG* m_DAG;                             //ָ��DAG��ָ��
	ALPThreadPool* m_ThreadPool;               //ָ���̳߳ص�ָ��
	ALPAlgResourcePool* m_AlgResourcePool;     //ָ���㷨��Դ�ص�ָ��
	ALPEventPool* m_EventPool;                 //ָ�������ص�ָ��


	std::thread m_UpdateNodeStateThread;            //������¸����㷨���ݽڵ��״̬
	tbb::concurrent_bounded_queue<std::function< ALPReturnCode() >> m_ActivateNodeAction_Queue;  //����DAG����������
	enum SchedulerState { INACTIVE = 0, ACTIVE = 1, ERROR = 2 };   //DAGTunnelScheduler������״̬
	std::atomic<SchedulerState> m_SchedulerIsActivate;   //���DAGTunnelScheduler�������Ƿ񼤻�����

	ALPDAGTunnelDog* m_DAGTunnelDog;                //DAGͨ���Ŀ��Ź�

public:
	//��ʼ�����ݽ��״̬
	ALPReturnCode InitDataNodesStatus();
	//��ʼ���㷨����״̬
	ALPReturnCode InitAlgNodesStatus();
        //reset all algorithm node status and all data node status, each event invoke once, except the first event
        ALPReturnCode ResetAllDAGNodesStatus();        

public:
	//����DAG��������ʼ�����ݽ��
	ALPReturnCode BeginEventProcessing(ALPDAGTunnel* tmpDAGTunnel);
	//DAGͨ����������������ź���
	GBSemaphore m_TunnelEventEnd_Semaphore;         

public:
	//��ѯ�㷨ʵ��ָ��
	ALPAlgBase* QueryAlgorithmInstance(const std::string name) const { return m_AlgNodeNameToAlgInstanceMap.at(name); }
	//�����㷨ʵ��
	ALPReturnCode  ApplyAlgorithmInstance(const std::string name);
	//�ͷ��㷨ʵ��
	ALPReturnCode  ReleaseAlgorithmInstance(const std::string name);

private:
	//Ϊ�����㷨����ʵ��
	ALPReturnCode ApplyAllAlgorithmInstances();
	//�ͷ������㷨ʵ��
	ALPReturnCode ReleaseAllAlgorithmInstances();

private:
	//�㷨������ƺ��㷨ʵ��ָ���ӳ��
	std::unordered_map<std::string, ALPAlgBase*> m_AlgNodeNameToAlgInstanceMap;   
	//�㷨������Ƶ�״̬��ӳ��
	std::unordered_map<std::string, ALPDAGAlgorithmNodeState*> m_AlgNodeNameToStatusMap;
	//���ݽ�����Ƶ����ݽ��״̬��ӳ��,������δ����Ϊfalse,�����Ѿ���Ϊtrue
	std::unordered_map<std::string, bool> m_DataNodeNameToStatusMap; 
	//data name to status map lock, prevent algorithm repeat execute 
	GBSemaphore  m_DataNodeNameToStatusMap_Lock;
	//ͨ���Ƿ��ڹ���״̬
	std::atomic_bool m_BusyFlag;


private:
	ALPDAGTunnel(const std::string& name);
};


#endif
