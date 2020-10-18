#ifndef SNIPER_ALP_THREAD_POOL_H
#define SNIPER_ALP_THREAD_POOL_H

#include <memory>
#include <tbb/spin_mutex.h>
#include <tbb/task_scheduler_init.h>
#define TBB_PREVIEW_GLOBAL_CONTROL 1
#include <tbb/global_control.h>
#include "SniperKernel/SniperLog.h"
#include "SniperKernel/Task.h"
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/gbmacrovar.h"

#define ALP_ThreadPool_DefaultSize -1    //Ĭ��ʹ�ÿ��õ�Ӳ������

class ALPThreadPool :public Task
{
public:
	ALPThreadPool(const std::string& name) :Task(name) {}
	~ALPThreadPool() {}

public:
	//run this task
	bool run() override final { return true; }
	//�����㷨��
	bool start() { return true; }
	//ֹͣ�㷨��
	bool stop() override final { return true; }

protected:
	//the concrete task operations
	bool config() override final { return true; }
	bool initialize() override final;
	bool finalize() override final;
	bool execute() override final { return true; }
	//clear all svcs and algs
	void reset() {}


public:
        //set the size of pool
        void SetPoolSize(const long int& poolsize);
	//����̳߳ص��߳���Ŀ
	long int GetPoolSize() const { return m_ThreadPoolSize; }
	//�����̳߳ص�״̬���Ƿ��Ѿ���ʼ��
	bool IsInitialized() const { return m_InitializedFlag; }

private:
	bool m_InitializedFlag=false;                  //�̳߳��Ƿ��Ѿ���ʼ��
	long m_ThreadPoolSize = 0;            //�̳߳ص�ǰӵ�е��߳���Ŀ
	tbb::spin_mutex m_ThreadPool_Mutex;            //ȷ���̳߳صĳ�ʼ�������ջ�Ϊԭ�Ӳ���
	std::unique_ptr<tbb::task_scheduler_init> m_TbbSchedulerInit;   //tbb������ȳ�ʼ������
	std::unique_ptr<tbb::global_control> m_TbbGlobalControl;        //���ڶ�tbb��̬���������

};

#endif
