#ifndef GBMACROVAR_H
#define GBMACROVAR_H

#include <mutex>
#include <condition_variable>

//事例池默认拥有的事例组数目
#define EventPool_GroupDefaultCount 1000 
//DAG Tunnel 最大共存数目
#define EventPool_DAGTunnel_MaxCount 100
//disable threads pool, enforce serial execution
#define ALP_ThreadPool_DisableSize  0

//最大线程数目
#define GlobalBuffer_Thread_MaxCount 10000
//访问全局缓冲区时的步长
#define GlobalBuffer_StepSize       1

#define EventIOQueueStream_GroupSize                 2000          //一个事例组包含的事例数目
#define EventIOQueueStream_GroupMinAmount           500          // 事例组数目
#define EventIOQueueStream_GroupInputOnce            100           //一次性最多尝试输入100个事例组

//对应事例组的状态：初始状态即事例组为空、数据就绪、处理中、处理完成等待输出、已输出等待内存释放、ERROR。
#define EventIOQueueStream_GroupState_Empty          0
#define EventIOQueueStream_GroupState_Ready          1
#define EventIOQueueStream_GroupState_Running        2
#define EventIOQueueStream_GroupState_Processed      3
#define EventIOQueueStream_GroupState_Release        4
#define EventIOQueueStream_GroupState_Error          5
#define EventIOQueueStream_GroupState_FileWriting    6

//用于全局缓冲区的锁或信号量
class GBSemaphore
{
public:
	GBSemaphore(long long count = 0) : m_GroupAvailable(count) {}

	inline void notify()
	{
		std::unique_lock<std::mutex> lock(mtx);
		m_GroupAvailable++;
		cv.notify_one();
	}

	inline void wait()
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [&]() {return this->m_GroupAvailable > 0; });
		m_GroupAvailable--;
	}

private:
	std::mutex mtx;
	std::condition_variable cv;
	long long m_GroupAvailable;
};

//Alg level parallel manager task : event pool service fileout svc ends flag, all data outputted
extern GBSemaphore global_ALPManagerTask_JobRunOver_Semaphore;


#endif // GBMACROVAR_H
