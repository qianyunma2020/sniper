#include <unistd.h>
#include <ctime>
#include "SniperKernel/ALPEventPool.h"
#include "SniperKernel/SniperLog.h"

ALPEventPool::ALPEventPool(const std::string& name) : Task(name)
{
	//��ʼ����ر���
	m_TransferThreadIsActive = INACTIVE;
	//�ź�����ʼ��
	m_EmptyEG_PtrSet_Lock.notify();

	//�����ļ����������ļ��������
	//m_FileInputSvc =  dynamic_cast<ALPFileIOSvc*>(createSvc("ALPFileInputSvc"));
	//m_FileOutputSvc = dynamic_cast<ALPFileIOSvc*>(createSvc("ALPFileOutputSvc"));
}

ALPEventPool::~ALPEventPool()
{
	
}

void ALPEventPool::InitRelatedVar(const unsigned long int group, const unsigned long inThreads, const unsigned long outThreads, const std::string inputFilesName, const std::string outputFilesName, const std::string collectionNamesFilter) {
	//������������Ŀ
	if(group < EventPool_GroupDefaultCount) 
            m_EventGroup_MaxCount = EventPool_GroupDefaultCount;
        else
	    m_EventGroup_MaxCount = group;
	//Ϊ������������Ŀ����ֵ
	m_EventGroup_EmptyStateCount = group;
	//Ϊ�ļ������߳���Ŀ����ֵ
	m_InputThreadsCount = inThreads;
	//Ϊ�ļ�����̸߳���ֵ
	m_OutputThreadsCount = outThreads;
	//���������ļ����Ƽ���
	m_InputFilesNameString = inputFilesName;
	//��������ļ����Ƽ���
	m_OutputFilesNameString = outputFilesName;
	//����collection������
	m_CollectionNamesFilterString = collectionNamesFilter;
}

void ALPEventPool::InitSvcVar(ALPFileIOSvc* tmpFileInputSvc, ALPFileIOSvc* tmpFileOutputSvc) {
    m_FileInputSvc = tmpFileInputSvc;
    m_FileOutputSvc = tmpFileOutputSvc;
}

bool ALPEventPool::initialize() {
	//��������Ŀ�Ŀ���������,��������ź���
	LogDebug << "Function ALPEventPool::initialize() imply step1." << std::endl;
	for (unsigned long int tmpCount = 0; tmpCount < m_EventGroup_MaxCount; tmpCount++) {
		ALPEventGroup* tmpEventGroup = new ALPEventGroup();
		m_EmptyEventGroupPtrSet.push_back(tmpEventGroup);
		m_EmptyEventGroup_Semaphore.notify();
	}
        LogDebug << "Function ALPEventPool::initialize() imply step2." << std::endl;

	//����������ת���߳�
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
	//��ʼ���ļ��������
	m_FileInputSvc->SetEventPool(this);
        LogDebug << "Function ALPEventPool::initialize() imply step4.1." << std::endl;
	m_FileInputSvc->SetInputThreadsCount(m_InputThreadsCount);
	m_FileInputSvc->SetInputFilesNameString(m_InputFilesNameString);
	m_FileInputSvc->SetCollectionNamesFilterString(m_CollectionNamesFilterString);
        LogDebug << "Function ALPEventPool::initialize() imply step4.2. m_InputFilesNameString="<< m_InputFilesNameString << std::endl;
	m_FileInputSvc->initialize();
        LogDebug << "Function ALPEventPool::initialize() imply step5." << std::endl;

	//��ʼ���ļ��������
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
	//�ͷ�ת���߳�ռ�õ���Դ
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

	//���ӳ���
	for (auto elementPtr : m_FileNameToEGQueue_Map) {
                LogDebug<<"Class ALPEventPool function finalize step2."<<std::endl;
		delete elementPtr.second;
                LogDebug<<"Class ALPEventPool function finalize step3."<<std::endl;
	}
	m_FileNameToEGQueue_Map.clear();
        LogDebug<<"Class ALPEventPool function finalize step4."<<std::endl;

	//ɾ�����п������������
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
                             �������̣߳������̣߳�����߳̾�Ϊ���̣߳������º������ע���̰߳�ȫ
*****************************************************************************************************************************/

//Ϊ�ض��ļ��������������,���Ѿ����������
ALPEventGroupQueue* ALPEventPool::AllocateEventGroupQueue(const std::string filename) {
        LogDebug<<"Function ALPEventPool::AllocateEventGroupQueue() says hello."<<std::endl;
	//������ļ�����һ�γ���
	if (0 == m_FileNameToEGQueue_Map.count(filename)) {
		//Ϊӳ�������ʼ����Queue����
		ALPEventGroupQueue* tmpQueue = new ALPEventGroupQueue();
		m_FileNameToEGQueue_Map[filename] = tmpQueue;
		//�����ļ������Թ�������Ӻ�������
		m_FileNames.push_back(filename);
		//Ϊ������������ʼ����Queue����
		ALPEventGroupQueue* tmpBufferQueue = new ALPEventGroupQueue();
		m_FileOutputBuffer[filename] = tmpBufferQueue;
	}
	
	return m_FileNameToEGQueue_Map[filename];
}

//Ϊ�ļ������̷߳�����е������飬
ALPEventGroup* ALPEventPool::AllocateEmptyEventGroup(const std::string filename) {
        LogDebug<<"Function ALPEventPool::AllocateEmptyEventGroup() says hello."<<std::endl;
	//����ǰû�п��������飬��ȴ�
	m_EmptyEventGroup_Semaphore.wait();
        LogDebug<<"Function ALPEventPool::AllocateEmptyEventGroup() step1."<<std::endl;
	//�������������,�������߳̿��ܴ��ڶ�����ʼ�����֤�̰߳�ȫ
	m_EmptyEG_PtrSet_Lock.wait();
	ALPEventGroup* tmpEventGroupPtr = m_EmptyEventGroupPtrSet.back();
	m_EmptyEventGroupPtrSet.pop_back();
        LogDebug<<"Function ALPEventPool::AllocateEmptyEventGroup() step2."<<std::endl;
	//�������������δ���䣬��������
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

//ΪDAGTunnel���䴦��Ready״̬��������
ALPEventGroup* ALPEventPool::AllocateReadyEventGroup() {
        LogDebug<<"Function ALPEventPool::AllocateReadyEventGroup() says hello."<<std::endl; 
	//�����ڴ���Ready״̬�������飬��ʼ����
	m_ReadyEventGroup_Semaphore.wait();
        LogDebug<<"Function ALPEventPool::AllocateReadyEventGroup() step1."<<std::endl;

	//�ı����ӵ�ֵ�����������
	std::srand(time(0));
	const unsigned long int tmpRandomIndex = std::rand() % m_FileNames.size();
	//�����������������
	ALPEventGroup* tmpEventGroupPtr = NULL;
	//�����λ�ÿ�ʼ�������
	for (unsigned long int index = tmpRandomIndex; index < m_FileNames.size(); index++) {
		//�������ض����������,���ҵ�Ready״̬��������ʱ������ֹ����
		tmpEventGroupPtr = m_FileNameToEGQueue_Map[m_FileNames[index]]->AllocateReadyEventGroup();
		if (NULL != tmpEventGroupPtr)
			break;
	}
        LogDebug<<"Function ALPEventPool::AllocateReadyEventGroup() step2."<<std::endl;
	//�Ѿ����ҵ��򷵻�
	if (NULL != tmpEventGroupPtr)
		return tmpEventGroupPtr;
	//�����λ�ÿ�ʼ��ǰ����
	for (unsigned long int index = 0; index < tmpRandomIndex; index++) {
		//��ǰ�����ض����������,���ҵ�Ready״̬��������ʱ������ֹ����
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


//����һ���̣߳����ڽ�����Processed״̬��EventGroup��ӳ���ת�Ƶ�Buffer,�Եȴ���������д������
void ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() {
        LogDebug << "Function ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() implys step1." <<std::endl;
        m_TransferThreadIsActive = ACTIVE;
	while (ACTIVE == m_TransferThreadIsActive) {
		//�ȴ�Processed״̬�����������
		LogDebug << "Function ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() implys step2." <<std::endl;
		m_ProcessedEventGroup_Semaphore.wait();
		//��¼ת�Ƶ���������Ŀ
		unsigned long int sum = 0;
		//�������ж��У�ת�ƴ���Processed״̬��������
		LogDebug << "Function ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() implys step3." <<std::endl;
		for (auto elementPtr : m_FileNameToEGQueue_Map) {
			unsigned long int count = 0;
			elementPtr.second->TransferProcessedEventGroupToFileOutputBuffer(m_FileOutputBuffer[elementPtr.first],count);
			sum = sum + count;
		}
                LogDebug << "Function ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() implys step4." <<std::endl;
		//���»������ź���
		for (unsigned long int tmpCount = 0; tmpCount < sum; tmpCount++) {
			m_ProcessedEG_BufferSemaphore.notify();
		}
                LogDebug << "Function ALPEventPool::ActivateTransferProcessedEventGroupToFileOutputBufferThread() implys step5." <<std::endl;
	}
	LogInfo << "Event group transfer thread ends." << std::endl;
	return;
}

//�ͷ�������ת���߳�ռ�õ���Դ
ALPReturnCode ALPEventPool::DeactivateTransferProcessedEventGroupToFileOutputBufferThread() {
	//����Ƿ���������ȴ����
	for (auto elementPtr : m_FileOutputBuffer) {
		if (0 != elementPtr.second->QueueSize()) {
			LogInfo << "Function ALPEventPool::DeactivateTransferProcessedEventGroupToFileOutputBufferThread() file output buffer is not empty, can't free Resouces!" << std::endl;
			m_TransferThreadIsActive = ERROR;
			return ALPReturnCode::FAILURE;
		}
	}
	//�ͷ���Դ
	for (auto elementPtr : m_FileOutputBuffer) {
		delete elementPtr.second;
	}
	m_FileOutputBuffer.clear();
	m_TransferThreadIsActive = INACTIVE;
	return ALPReturnCode::SUCCESS;
}

/*Ϊ�ļ�����̷߳��䴦����ɵ�������,���ò���filename�����β���ʵ�δ�ֵ��
�Լ�¼�������Ӧ�ļ��������Ȳ���filename����Ӧ�Ķ���*/
ALPEventGroup* ALPEventPool::AllocateProcessedEventGroupFromBuffer(std::string& filename) {
        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer says hello.filename="<<filename<<std::endl;
	//�����ź���
	m_ProcessedEG_BufferSemaphore.wait();
        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step1.m_FileOutputBuffer["<<filename<<"].size()="<<m_FileOutputBuffer[filename]->QueueSize()<<std::endl;
	//���Ȳ��ң����������ļ��л�
	ALPEventGroup* tmpEventGroup = m_FileOutputBuffer[filename]->AllocateProcessedEventGroupFromBuffer();
        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step2."<<std::endl;
	if (NULL != tmpEventGroup) {
		//���ش������������
		LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step3."<<std::endl;
		return tmpEventGroup;
	}
	else {
		//���ų��ղŲ��ҵ��ļ���������������µĿ�����
		LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step4."<<std::endl;
		for (auto elementPtr : m_FileOutputBuffer) {
                        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step5."<<std::endl;
			tmpEventGroup = elementPtr.second->AllocateProcessedEventGroupFromBuffer();
                        LogDebug<<"Class ALPEventPool AllocateProcessedEventGroupFromBuffer step6."<<std::endl;
			if (NULL != tmpEventGroup) {
				//���ش������������,�������ļ�����
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

//���տ��е������飺�����������ͷ��������ֱ������µĿ���������
bool ALPEventPool::RecycleFrontEventGroupFromBuffer(const std::string filename) {
	//�����ļ�����������ض��ļ����ж���������
	m_FileOutputBuffer[filename]->DestroyHeadGroup();

	//����ж����ж���̴߳�ȡ���ʼ�����֤�̰߳�ȫ
	m_EmptyEG_PtrSet_Lock.wait();
	ALPEventGroup* tmpEventGroupPtr = new ALPEventGroup();
	m_EmptyEventGroupPtrSet.push_back(tmpEventGroupPtr);
	m_EventGroup_EmptyStateCount++;
	//�����ź���
	m_EmptyEventGroup_Semaphore.notify();
	m_EmptyEG_PtrSet_Lock.notify();

	/*����������Ƿ�������������Ϊempty��״̬������ǣ���˵�������̡߳�����̡߳�����ѭ���߳�
	  �Ѿ�û���κ�������Ҫ�����ˣ�������ź������������ڵȴ����ݵ�����ѭ�������̣߳�
	  �Լ����ڵȴ�������̣߳�ִ����β�������������˳������׼��
	*/
	if (m_EventGroup_MaxCount == m_EventGroup_EmptyStateCount) {
		for (unsigned int count = 0; count < GlobalBuffer_Thread_MaxCount; count++) {
			m_ReadyEventGroup_Semaphore.notify();
			m_ProcessedEG_BufferSemaphore.notify();
		}
	}
	return true;
}
