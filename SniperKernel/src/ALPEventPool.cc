#include <unistd.h>
#include <ctime>
#include "SniperKernel/ALPEventPool.h"
#include "SniperKernel/SniperLog.h"

ALPEventPool::ALPEventPool(const std::string& name) : Task(name)
{
	//初始化相关变量
	m_TransferThreadIsActive = INACTIVE;
	//信号量初始化
	m_EmptyEG_PtrSet_Lock.notify();

	//创建文件输入服务和文件输出服务
	//m_FileInputSvc =  dynamic_cast<ALPFileIOSvc*>(createSvc("ALPFileInputSvc"));
	//m_FileOutputSvc = dynamic_cast<ALPFileIOSvc*>(createSvc("ALPFileOutputSvc"));
}

ALPEventPool::~ALPEventPool()
{
	
}

void ALPEventPool::InitRelatedVar(const unsigned long int group, const unsigned long inThreads, const unsigned long outThreads, const std::string inputFilesName, const std::string outputFilesName, const std::string collectionNamesFilter) {
	//设置事例组数目
	if(group < EventPool_GroupDefaultCount) 
            m_EventGroup_MaxCount = EventPool_GroupDefaultCount;
        else
	    m_EventGroup_MaxCount = group;
	//为空闲事例组数目赋初值
	m_EventGroup_EmptyStateCount = group;
	//为文件输入线程数目赋初值
	m_InputThreadsCount = inThreads;
	//为文件输出线程赋初值
	m_OutputThreadsCount = outThreads;
	//设置输入文件名称集合
	m_InputFilesNameString = inputFilesName;
	//设置输出文件名称集合
	m_OutputFilesNameString = outputFilesName;
	//设置collection过滤器
	m_CollectionNamesFilterString = collectionNamesFilter;
}

void ALPEventPool::InitSvcVar(ALPFileIOSvc* tmpFileInputSvc, ALPFileIOSvc* tmpFileOutputSvc) {
    m_FileInputSvc = tmpFileInputSvc;
    m_FileOutputSvc = tmpFileOutputSvc;
}

bool ALPEventPool::initialize() {
	//添加最大数目的空闲事例组,更新相关信号量
	LogDebug << "Function ALPEventPool::initialize() imply step1." << std::endl;
	for (unsigned long int tmpCount = 0; tmpCount < m_EventGroup_MaxCount; tmpCount++) {
		ALPEventGroup* tmpEventGroup = new ALPEventGroup();
		m_EmptyEventGroupPtrSet.push_back(tmpEventGroup);
		m_EmptyEventGroup_Semaphore.notify();
	}
        LogDebug << "Function ALPEventPool::initialize() imply step2." << std::endl;

	//激活事例组转移线程
	m_TransferEGToBufferThread = std::thread([this]() {this->ActivateTransferProcessedEventGroupToFileOutputBufferThread(); });
        LogDebug << "Function ALPEventPool::initialize() imply step3." << std::endl;

	while (ACTIVE != m_TransferThreadIsActive) {
		if (ERROR == m_TransferThreadIsActive) {
			LogError << "Function ALPEventPool::initialize() implys flag m_TransferThreadIsActive is Error!" << std::endl;
			return false;
		}
		LogInfo << "Waiting for transfer thread to active..." << std::endl;
		sleep(1);
	}
        LogDebug << "Function ALPEventPool::initialize() imply step4." << std::endl;
	//初始化文件输入服务
	m_FileInputSvc->SetEventPool(this);
        LogDebug << "Function ALPEventPool::initialize() imply step4.1." << std::endl;
	m_FileInputSvc->SetInputThreadsCount(m_InputThreadsCount);
	m_FileInputSvc->SetInputFilesNameString(m_InputFilesNameString);
	m_FileInputSvc->SetCollectionNamesFilterString(m_CollectionNamesFilterString);
        LogDebug << "Function ALPEventPool::initialize() imply step4.2. m_InputFilesNameString="<< m_InputFilesNameString << std::endl;
	m_FileInputSvc->initialize();
        LogDebug << "Function ALPEventPool::initialize() imply step5." << std::endl;

	//初始化文件输出服务
	m_FileOutputSvc->SetEventPool(this);
	m_FileOutputSvc->SetOutputThreadsCount(m_OutputThreadsCount);
	m_FileOutputSvc->SetInputFilesNameString(m_InputFilesNameString);
	m_FileOutputSvc->SetOutputFilesNameString(m_OutputFilesNameString);
	m_FileOutputSvc->SetCollectionNamesFilterString(m_CollectionNamesFilterString);
        LogDebug << "Function ALPEventPool::initialize() imply step5.1. m_InputFilesNameString="<<m_InputFilesNameString<<",  m_OutputFilesNameString="<<m_OutputFilesNameString << std::endl;
	m_FileOutputSvc->initialize();  
        LogDebug << "Function ALPEventPool::initialize() imply step6." << std::endl;
        return true;
}

bool ALPEventPool::finalize() {
	//释放转移线程占用的资源
	LogDebug<<"Class ALPEventPool function finalize says hello."<<std::endl;
	if (ALPReturnCode::FAILURE == DeactivateTransferProcessedEventGroupToFileOutputBufferThread()) {
		LogWarn << "Function ALPEventPool::finalize() implys transfer thread couldn't be deactivated." << std::endl;
	}
	LogInfo << "Waiting for transfer thread to finish, and free resources." << std::endl;
	m_ProcessedEventGroup_Semaphore.notify();
	m_TransferEGToBufferThread.join();
	if (ERROR == m_TransferThreadIsActive) {
		LogError << "Transfer thread happens Error!" << std::endl;
	}
        LogDebug<<"Class ALPEventPool function finalize step1."<<std::endl;

	//清空映射表
	for (auto elementPtr : m_FileNameToEGQueue_Map) {
                LogDebug<<"Class ALPEventPool function finalize step2."<<std::endl;
		delete elementPtr.second;
                LogDebug<<"Class ALPEventPool function finalize step3."<<std::endl;
	}
	m_FileNameToEGQueue_Map.clear();
        LogDebug<<"Class ALPEventPool function finalize step4."<<std::endl;

	//删除所有空闲事例组对象
	for (long int tmpCount = m_EmptyEventGroupPtrSet.size() - 1; 0 <= tmpCount; tmpCount--) {
                LogDebug<<"Class ALPEventPool function finalize step5. tmpCount="<<tmpCount<<std::endl;
		delete m_EmptyEventGroupPtrSet[tmpCount];
                LogDebug<<"Class ALPEventPool function finalize step6."<<std::endl;
	}
	m_EmptyEventGroupPtrSet.clear();
        LogDebug<<"Class ALPEventPool function finalize says byebye."<<std::endl;
        return true;
}

/*****************************************************************************************************************************
                             因输入线程，处理线程，输出线程均为多线程，故以下函数务必注意线程安全
*****************************************************************************************************************************/

//为特定文件分配事例组队列,若已经存在则忽略
ALPEventGroupQueue* ALPEventPool::AllocateEventGroupQueue(const std::string filename) {
        LogDebug<<"Function ALPEventPool::AllocateEventGroupQueue() says hello."<<std::endl;
	//如果该文件名第一次出现
	if (0 == m_FileNameToEGQueue_Map.count(filename)) {
		//为映射表建立初始化的Queue对象
		ALPEventGroupQueue* tmpQueue = new ALPEventGroupQueue();
		m_FileNameToEGQueue_Map[filename] = tmpQueue;
		//更新文件名称以供随机种子函数调用
		m_FileNames.push_back(filename);
		//为缓冲区建立初始化的Queue对象
		ALPEventGroupQueue* tmpBufferQueue = new ALPEventGroupQueue();
		m_FileOutputBuffer[filename] = tmpBufferQueue;
	}
	
	return m_FileNameToEGQueue_Map[filename];
}

//为文件输入线程分配空闲的事例组，
ALPEventGroup* ALPEventPool::AllocateEmptyEventGroup(const std::string filename) {
        LogDebug<<"Function ALPEventPool::AllocateEmptyEventGroup() says hello."<<std::endl;
	//若当前没有空闲事例组，则等待
	m_EmptyEventGroup_Semaphore.wait();
        LogDebug<<"Function ALPEventPool::AllocateEmptyEventGroup() step1."<<std::endl;
	//分配空闲事例组,因输入线程可能存在多个，故加锁保证线程安全
	m_EmptyEG_PtrSet_Lock.wait();
	ALPEventGroup* tmpEventGroupPtr = m_EmptyEventGroupPtrSet.back();
	m_EmptyEventGroupPtrSet.pop_back();
        LogDebug<<"Function ALPEventPool::AllocateEmptyEventGroup() step2."<<std::endl;
	//若事例组队列尚未分配，则分配队列
	if (0 == m_FileNameToEGQueue_Map.count(filename)) {
		AllocateEventGroupQueue(filename);
	}
        LogDebug<<"Function ALPEventPool::AllocateEmptyEventGroup() step3."<<std::endl;
	m_FileNameToEGQueue_Map[filename]->PushBackOneGroup(tmpEventGroupPtr);
	m_EventGroup_EmptyStateCount--;
	m_EmptyEG_PtrSet_Lock.notify();
        LogDebug<<"Function ALPEventPool::AllocateEmptyEventGroup() step4."<<std::endl;
	return tmpEventGroupPtr;
}

//为DAGTunnel分配处于Ready状态的事例组
ALPEventGroup* ALPEventPool::AllocateReadyEventGroup() {
        LogDebug<<"Function ALPEventPool::AllocateReadyEventGroup() says hello."<<std::endl; 
	//若存在处于Ready状态的事例组，则开始查找
	m_ReadyEventGroup_Semaphore.wait();
        LogDebug<<"Function ALPEventPool::AllocateReadyEventGroup() step1."<<std::endl;

	//改变种子的值以生成随机数
	std::srand(time(0));
	const unsigned long int tmpRandomIndex = std::rand() % m_FileNames.size();
	//遍历所有事例组队列
	ALPEventGroup* tmpEventGroupPtr = NULL;
	//从随机位置开始往后遍历
	for (unsigned long int index = tmpRandomIndex; index < m_FileNames.size(); index++) {
		//向后遍历特定事例组队列,查找到Ready状态的事例组时，则终止查找
		tmpEventGroupPtr = m_FileNameToEGQueue_Map[m_FileNames[index]]->AllocateReadyEventGroup();
		if (NULL != tmpEventGroupPtr)
			break;
	}
        LogDebug<<"Function ALPEventPool::AllocateReadyEventGroup() step2."<<std::endl;
	//已经查找到则返回
	if (NULL != tmpEventGroupPtr)
		return tmpEventGroupPtr;
	//从随机位置开始往前遍历
	for (unsigned long int index = 0; index < tmpRandomIndex; index++) {
		//向前遍历特定事例组队列,查找到Ready状态的事例组时，则终止查找
		tmpEventGroupPtr = m_FileNameToEGQueue_Map[m_FileNames[index]]->AllocateReadyEventGroup();
		if (NULL != tmpEventGroupPtr)
			break;
	}
        LogDebug<<"Function ALPEventPool::AllocateReadyEventGroup() step3."<<std::endl;
	if (NULL == tmpEventGroupPtr) {
		LogWarn << "Function ALPEventPool::AllocateReadyEventGroup() occurs Error! It can't find ready state event group , so it will return NULL!" << std::endl;
		return NULL;
	}
        LogDebug<<"Function ALPEventPool::AllocateReadyEventGroup() says byebye."<<std::endl;
	return tmpEventGroupPtr;
}


//另起一个线程，用于将处于Processed状态的EventGroup从映射表转移到Buffer,以等待事例数据写到磁盘
void ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() {
        LogDebug << "Function ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() implys step1." <<std::endl;
        m_TransferThreadIsActive = ACTIVE;
	while (ACTIVE == m_TransferThreadIsActive) {
		//等待Processed状态的事例组出现
		LogDebug << "Function ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() implys step2." <<std::endl;
		m_ProcessedEventGroup_Semaphore.wait();
		//记录转移的事例组数目
		unsigned long int sum = 0;
		//遍历所有队列，转移处于Processed状态的事例组
		LogDebug << "Function ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() implys step3." <<std::endl;
		for (auto elementPtr : m_FileNameToEGQueue_Map) {
			unsigned long int count = 0;
			elementPtr.second->TransferProcessedEventGroupToFileOutputBuffer(m_FileOutputBuffer[elementPtr.first],count);
			sum = sum + count;
		}
                LogDebug << "Function ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() implys step4." <<std::endl;
		//更新缓冲区信号量
		for (unsigned long int tmpCount = 0; tmpCount < sum; tmpCount++) {
			m_ProcessedEG_BufferSemaphore.notify();
		}
                LogDebug << "Function ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() implys step5." <<std::endl;
	}
	LogInfo << "Event group transfer thread ends." << std::endl;
	return;
}

//释放事例组转移线程占用的资源
ALPReturnCode ALPEventPool::DeactivateTransferProcessedEventGroupToFileOutputBufferThread() {
	//检查是否有事例组等待输出
	for (auto elementPtr : m_FileOutputBuffer) {
		if (0 != elementPtr.second->QueueSize()) {
			LogInfo << "Function ALPEventPool::DeactivateTransferProcessedEventGroupToFileOutputBufferThread() file output buffer is not empty, can't free Resouces!" << std::endl;
			m_TransferThreadIsActive = ERROR;
			return ALPReturnCode::FAILURE;
		}
	}
	//释放资源
	for (auto elementPtr : m_FileOutputBuffer) {
		delete elementPtr.second;
	}
	m_FileOutputBuffer.clear();
	m_TransferThreadIsActive = INACTIVE;
	return ALPReturnCode::SUCCESS;
}

/*为文件输出线程分配处理完成的事例组,引用参数filename用于形参与实参传值，
以记录事例组对应文件名；优先查找filename所对应的队列*/
ALPEventGroup* ALPEventPool::AllocateProcessedEventGroupFromBuffer(std::string& filename) {
        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer says hello.filename="<<filename<<std::endl;
	//更新信号量
	m_ProcessedEG_BufferSemaphore.wait();
        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step1.m_FileOutputBuffer["<<filename<<"].size()="<<m_FileOutputBuffer[filename]->QueueSize()<<std::endl;
	//优先查找，尽量减少文件切换
	ALPEventGroup* tmpEventGroup = m_FileOutputBuffer[filename]->AllocateProcessedEventGroupFromBuffer();
        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step2."<<std::endl;
	if (NULL != tmpEventGroup) {
		//返回待输出的事例组
		LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step3."<<std::endl;
		return tmpEventGroup;
	}
	else {
		//不排除刚才查找的文件，因有事例组更新的可能性
		LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step4."<<std::endl;
		for (auto elementPtr : m_FileOutputBuffer) {
                        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step5."<<std::endl;
			tmpEventGroup = elementPtr.second->AllocateProcessedEventGroupFromBuffer();
                        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step6."<<std::endl;
			if (NULL != tmpEventGroup) {
				//返回待输出的事例组,并更新文件名称
				filename = elementPtr.first;
				return tmpEventGroup;
			}
                        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step7."<<std::endl;
		}	
                LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step8."<<std::endl;
	}
	LogWarn << "Function ALPEventPool::AllocateProcessedEventGroupFromBuffer() can't find processed event group! Error!" << std::endl;
	return NULL;
}

//回收空闲的事例组：在事例队列释放事例组后，直接添加新的空闲事例组
bool ALPEventPool::RecycleFrontEventGroupFromBuffer(const std::string filename) {
	//销毁文件输出缓冲区特定文件队列队首事例组
	m_FileOutputBuffer[filename]->DestroyHeadGroup();

	//因空闲队列有多个线程存取，故加锁保证线程安全
	m_EmptyEG_PtrSet_Lock.wait();
	ALPEventGroup* tmpEventGroupPtr = new ALPEventGroup();
	m_EmptyEventGroupPtrSet.push_back(tmpEventGroupPtr);
	m_EventGroup_EmptyStateCount++;
	//更新信号量
	m_EmptyEventGroup_Semaphore.notify();
	m_EmptyEG_PtrSet_Lock.notify();

	/*检测事例池是否处于所有事例组为empty的状态，如果是，则说明输入线程、输出线程、事例循环线程
	  已经没有任何数据需要处理了，则更新信号量，唤醒正在等待数据的事例循环处理线程，
	  以及正在等待的输出线程，执行收尾清理工作，做好退出程序的准备
	*/
	if (m_EventGroup_MaxCount == m_EventGroup_EmptyStateCount) {
		for (unsigned int count = 0; count < GlobalBuffer_Thread_MaxCount; count++) {
			m_ReadyEventGroup_Semaphore.notify();
			m_ProcessedEG_BufferSemaphore.notify();
		}
	}
	return true;
}
