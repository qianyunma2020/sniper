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

#define ALP_ThreadPool_DefaultSize -1    //默认使用可用的硬件核数

class ALPThreadPool :public Task
{
public:
	ALPThreadPool(const std::string& name) :Task(name) {}
	~ALPThreadPool() {}

public:
	//run this task
	bool run() override final { return true; }
	//启动算法池
	bool start() { return true; }
	//停止算法池
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
	//获得线程池的线程数目
	long int GetPoolSize() const { return m_ThreadPoolSize; }
	//返回线程池的状态，是否已经初始化
	bool IsInitialized() const { return m_InitializedFlag; }

private:
	bool m_InitializedFlag=false;                  //线程池是否已经初始化
	long m_ThreadPoolSize = 0;            //线程池当前拥有的线程数目
	tbb::spin_mutex m_ThreadPool_Mutex;            //确保线程池的初始化和最终化为原子操作
	std::unique_ptr<tbb::task_scheduler_init> m_TbbSchedulerInit;   //tbb任务调度初始化程序
	std::unique_ptr<tbb::global_control> m_TbbGlobalControl;        //用于对tbb动态库进行设置

};

#endif
