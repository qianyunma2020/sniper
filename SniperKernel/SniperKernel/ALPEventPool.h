#ifndef SNIPER_ALP_EVENT_POOL_H
#define SNIPER_ALP_EVENT_POOL_H

#include <string>
#include <atomic>
#include <vector>
#include <unordered_map>
#include "SniperKernel/Task.h"
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/ALPEventGroupQueue.h"
#include "SniperKernel/ALPFileIOSvc.h"

class ALPEventPool : public Task
{
public:
	ALPEventPool(const std::string& name);
	~ALPEventPool();
	void InitRelatedVar(const unsigned long int group, const unsigned long inThreads, const unsigned long outThreads, const std::string inputFilesName, const std::string outputFilesName, const std::string collectionNamesFilter);
        void InitSvcVar(ALPFileIOSvc* tmpFileInputSvc, ALPFileIOSvc* tmpFileOutputSvc);

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

/**********************************************************************************************************************
                               因输入线程，处理线程，输出线程均为多线程，故以下函数务必注意线程安全
**********************************************************************************************************************/
public:
	//为文件输入线程分配空闲的事例组
	ALPEventGroup* AllocateEmptyEventGroup(const std::string filename);

	//为DAGTunnel分配处于Ready状态的事例组
	ALPEventGroup* AllocateReadyEventGroup();

	/*为文件输出线程分配处理完成的事例组,引用参数filename用于形参与实参传值，
	以记录事例组对应文件名；优先查找filename所对应的队列*/
	ALPEventGroup* AllocateProcessedEventGroupFromBuffer(std::string& filename);        
	
	//回收空闲的事例组：在事例队列释放事例组后，直接添加新的空闲事例组
	bool RecycleFrontEventGroupFromBuffer(const std::string filename);

private:
	//为特定文件分配事例组队列,若已经存在则忽略
	ALPEventGroupQueue* AllocateEventGroupQueue(const std::string filename);

	//另起一个线程，用于将处于Processed状态的EventGroup从映射表转移到Buffer,以等待事例数据写到磁盘
	void ActivateTransferProcessedEventGroupToFileOutputBufferThread();

	//释放事例组转移线程占用的资源
	ALPReturnCode DeactivateTransferProcessedEventGroupToFileOutputBufferThread();

public:
	//文件输入线程完成当前事例组的输入工作，更新Ready状态的事例组数目信号量
	void PushReadyEventGroupSemaphore() { m_ReadyEventGroup_Semaphore.notify(); }
	//事例处理线程完成当前事例组的计算工作，更新Processed状态事例组数目的信号量，以供事例组转移线程将事例组加入到Buffer
	void PushProcessedEventGroupSemaphore() { m_ProcessedEventGroup_Semaphore.notify(); }

private:
	//对应处于Ready状态的事例组数目的信号量,事例处理线程要用
	GBSemaphore m_ReadyEventGroup_Semaphore;
	//对应处于Processed状态事例组数目的信号量，Processed事例组转移线程要用
	GBSemaphore m_ProcessedEventGroup_Semaphore;
	//对应处于Empty状态的事例组数目的信号量
	GBSemaphore m_EmptyEventGroup_Semaphore;
	//对应文件输出缓冲区中Processed状态事例组的信号量
	GBSemaphore m_ProcessedEG_BufferSemaphore;
	
	//指向空闲事例组的指针集合
	std::vector<ALPEventGroup*> m_EmptyEventGroupPtrSet;
	//分配空闲事例组时,保证线程安全的锁
	GBSemaphore m_EmptyEG_PtrSet_Lock;
	//文件名与事例队列的映射关系表
	std::unordered_map<std::string, ALPEventGroupQueue*> m_FileNameToEGQueue_Map; 
	//存储文件名称的集合，仅供随机种子调用
	std::vector<std::string> m_FileNames;
	//用于缓存Processed状态的EventGroup，以供文件输出线程将数据从Buffer写到文件
	std::unordered_map<std::string, ALPEventGroupQueue*> m_FileOutputBuffer;

	//事例组的最大数目
	unsigned long int m_EventGroup_MaxCount;
	//当前空闲事例组的数目
	std::atomic<unsigned long> m_EventGroup_EmptyStateCount;

	//用于将事例组转移到文件输出的Buffer，以供文件输出线程使用
	std::thread m_TransferEGToBufferThread;
	//转移线程的状态
	enum TransferThreadState { INACTIVE = 0, ACTIVE = 1, ERROR = 2 };   
	std::atomic<TransferThreadState> m_TransferThreadIsActive;


private:
	ALPFileIOSvc* m_FileInputSvc;                 //文件输入服务
	ALPFileIOSvc* m_FileOutputSvc;                //文件输出服务
	unsigned long m_InputThreadsCount;            //输入线程数目，默认为1   
	unsigned long m_OutputThreadsCount;           //输出线程数目，默认为1  
	std::string m_InputFilesNameString;           //输入文件名称集合字符串
	std::string m_OutputFilesNameString;          //输出文件名称列表
	std::string m_CollectionNamesFilterString;    //用户指定的Collection集合过滤器字符串
	
private:
	// following interfaces are not supported
	ALPEventPool(const ALPEventPool&);
	ALPEventPool& operator=(const ALPEventPool&);
};


#endif
