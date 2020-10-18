#include <tbb/task.h>
#include <tbb/task_scheduler_observer.h>
#include <tbb/tbb_thread.h>
#include "AlgLevelParallel/ALPThreadPool.h"

using namespace tbb;


void ALPThreadPool::SetPoolSize(const long int& poolsize){
    m_ThreadPoolSize = poolsize;
}

//初始化线程池
bool ALPThreadPool::initialize() {
     LogDebug << "Function ALPThreadPool::initialize() step1." <<std::endl;
     tbb::spin_mutex::scoped_lock lock(m_ThreadPool_Mutex);
     LogDebug << "Function ALPThreadPool::initialize() step2." <<std::endl;

     if (ALP_ThreadPool_DisableSize == m_ThreadPoolSize) {
          //不使用线程池
          LogDebug << "Function ALPThreadPool::initialize() Bracnch1 step1." <<std::endl;
	  m_TbbGlobalControl = std::unique_ptr<tbb::global_control>(new tbb::global_control(global_control::max_allowed_parallelism, 1));
          LogDebug << "Function ALPThreadPool::initialize() Bracnch1 step2." <<std::endl;
     }else {
             //
	     if (ALP_ThreadPool_DefaultSize == m_ThreadPoolSize) {
	             //创建默认数量的线程
	             LogDebug << "Function ALPThreadPool::initialize() Bracnch2 step1." <<std::endl;
	       	     m_TbbSchedulerInit = std::unique_ptr<tbb::task_scheduler_init>(new tbb::task_scheduler_init());
	             m_ThreadPoolSize = m_TbbSchedulerInit->default_num_threads();
                     LogDebug << "Function ALPThreadPool::initialize() Bracnch2 step2." <<std::endl;
             }else if (1 <= m_ThreadPoolSize) {
	             //将线程池数目限制为m_ThreadPoolSize+1
	             LogDebug << "Function ALPThreadPool::initialize() Bracnch3 step1." <<std::endl;
		     m_TbbGlobalControl = std::unique_ptr<tbb::global_control>(new tbb::global_control(global_control::max_allowed_parallelism, m_ThreadPoolSize + 1));
		     m_TbbSchedulerInit = std::unique_ptr<tbb::task_scheduler_init>(new tbb::task_scheduler_init(m_ThreadPoolSize + 1));
                     LogDebug << "Function ALPThreadPool::initialize() Bracnch3 step2." <<std::endl;
             }else {
		     //输出错误信息
		     LogFatal << " Value must be positive or  -1 or 0, can't init thread pool." << std::endl;
		     return false;
	     }
     }

     m_InitializedFlag = true;
     LogDebug << "Function ALPThreadPool::initialize() step4." <<std::endl;
     return true;
}

//最终化线程池
bool ALPThreadPool::finalize() 
{
	tbb::spin_mutex::scoped_lock lock(m_ThreadPool_Mutex);

	if (!m_InitializedFlag) {
		LogError << "Thread pool is uninitialized, please checks!" << std::endl;
		return false;
	}

	return true;
}
