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

/**********************************************************************************************************************
                               �������̣߳������̣߳�����߳̾�Ϊ���̣߳������º������ע���̰߳�ȫ
**********************************************************************************************************************/
public:
	//Ϊ�ļ������̷߳�����е�������
	ALPEventGroup* AllocateEmptyEventGroup(const std::string filename);

	//ΪDAGTunnel���䴦��Ready״̬��������
	ALPEventGroup* AllocateReadyEventGroup();

	/*Ϊ�ļ�����̷߳��䴦����ɵ�������,���ò���filename�����β���ʵ�δ�ֵ��
	�Լ�¼�������Ӧ�ļ��������Ȳ���filename����Ӧ�Ķ���*/
	ALPEventGroup* AllocateProcessedEventGroupFromBuffer(std::string& filename);        
	
	//���տ��е������飺�����������ͷ��������ֱ������µĿ���������
	bool RecycleFrontEventGroupFromBuffer(const std::string filename);

private:
	//Ϊ�ض��ļ��������������,���Ѿ����������
	ALPEventGroupQueue* AllocateEventGroupQueue(const std::string filename);

	//����һ���̣߳����ڽ�����Processed״̬��EventGroup��ӳ���ת�Ƶ�Buffer,�Եȴ���������д������
	void ActivateTransferProcessedEventGroupToFileOutputBufferThread();

	//�ͷ�������ת���߳�ռ�õ���Դ
	ALPReturnCode DeactivateTransferProcessedEventGroupToFileOutputBufferThread();

public:
	//�ļ������߳���ɵ�ǰ����������빤��������Ready״̬����������Ŀ�ź���
	void PushReadyEventGroupSemaphore() { m_ReadyEventGroup_Semaphore.notify(); }
	//���������߳���ɵ�ǰ������ļ��㹤��������Processed״̬��������Ŀ���ź������Թ�������ת���߳̽���������뵽Buffer
	void PushProcessedEventGroupSemaphore() { m_ProcessedEventGroup_Semaphore.notify(); }

private:
	//��Ӧ����Ready״̬����������Ŀ���ź���,���������߳�Ҫ��
	GBSemaphore m_ReadyEventGroup_Semaphore;
	//��Ӧ����Processed״̬��������Ŀ���ź�����Processed������ת���߳�Ҫ��
	GBSemaphore m_ProcessedEventGroup_Semaphore;
	//��Ӧ����Empty״̬����������Ŀ���ź���
	GBSemaphore m_EmptyEventGroup_Semaphore;
	//��Ӧ�ļ������������Processed״̬��������ź���
	GBSemaphore m_ProcessedEG_BufferSemaphore;
	
	//ָ������������ָ�뼯��
	std::vector<ALPEventGroup*> m_EmptyEventGroupPtrSet;
	//�������������ʱ,��֤�̰߳�ȫ����
	GBSemaphore m_EmptyEG_PtrSet_Lock;
	//�ļ������������е�ӳ���ϵ��
	std::unordered_map<std::string, ALPEventGroupQueue*> m_FileNameToEGQueue_Map; 
	//�洢�ļ����Ƶļ��ϣ�����������ӵ���
	std::vector<std::string> m_FileNames;
	//���ڻ���Processed״̬��EventGroup���Թ��ļ�����߳̽����ݴ�Bufferд���ļ�
	std::unordered_map<std::string, ALPEventGroupQueue*> m_FileOutputBuffer;

	//������������Ŀ
	unsigned long int m_EventGroup_MaxCount;
	//��ǰ�������������Ŀ
	std::atomic<unsigned long> m_EventGroup_EmptyStateCount;

	//���ڽ�������ת�Ƶ��ļ������Buffer���Թ��ļ�����߳�ʹ��
	std::thread m_TransferEGToBufferThread;
	//ת���̵߳�״̬
	enum TransferThreadState { INACTIVE = 0, ACTIVE = 1, ERROR = 2 };   
	std::atomic<TransferThreadState> m_TransferThreadIsActive;


private:
	ALPFileIOSvc* m_FileInputSvc;                 //�ļ��������
	ALPFileIOSvc* m_FileOutputSvc;                //�ļ��������
	unsigned long m_InputThreadsCount;            //�����߳���Ŀ��Ĭ��Ϊ1   
	unsigned long m_OutputThreadsCount;           //����߳���Ŀ��Ĭ��Ϊ1  
	std::string m_InputFilesNameString;           //�����ļ����Ƽ����ַ���
	std::string m_OutputFilesNameString;          //����ļ������б�
	std::string m_CollectionNamesFilterString;    //�û�ָ����Collection���Ϲ������ַ���
	
private:
	// following interfaces are not supported
	ALPEventPool(const ALPEventPool&);
	ALPEventPool& operator=(const ALPEventPool&);
};


#endif
