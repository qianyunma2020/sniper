#ifndef SNIPER_ALP_EVENTGROUP_QUEUE_H
#define SNIPER_ALP_EVENTGROUP_QUEUE_H

#include <thread>
#include <deque>
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/ALPEventGroup.h"

//���������
class ALPEventGroupQueue 
{
public:
    ALPEventGroupQueue();
    ~ALPEventGroupQueue();

public:
    //�ڶ���β������һ��������,��ֹ�ļ����Buffer���ôκ���,��Bufferֻ��ͨ��ת�������������.
    void PushBackOneGroup(ALPEventGroup* tmpEventGroupPtr);

    //�ͷŶ���ǰ���׸�������ָ��
    bool PopFrontOneGroup();   
    
    //���ٶ���ͷ�����飬�ͷ���ռ�õ��ڴ棬�����ļ�����������е���������е���
    bool DestroyHeadGroup(); 

    //���ص�ǰ��������Ŀ
    unsigned long int QueueSize() const { return m_EventGroupQueue.size(); }
    //���䴦�����ݾ���״̬��������,��û�����ݾ����������飬�򷵻�NULL
    ALPEventGroup* AllocateReadyEventGroup();
    /*���ڽ�������ת�Ƶ��ļ����������������ǰ����������д���Processed״̬��������ת�Ƶ�Ŀ�����,
      ��һ������ΪĿ����У��ڶ���������¼ת�Ƶ���������Ŀ��������ʹ���βκ�ʵ��ֱ�ӽ���ֵ����*/
    ALPReturnCode TransferProcessedEventGroupToFileOutputBuffer(ALPEventGroupQueue* tmpQueue, unsigned long int& count);
    //�����ļ�������������ã����䴦�������Ѵ������״̬��������,��û�������Ѵ�����ɵ������飬�򷵻�NULL��
    ALPEventGroup* AllocateProcessedEventGroupFromBuffer();
    
private:
    //�ͷŶ���ǰ�˶��������ָ��
    bool PopFrontMultiGroup(std::deque<ALPEventGroup*>::iterator first, std::deque<ALPEventGroup*>::iterator last);
    //�����������ָ��ѹ���ļ����Buffer���к��
    bool PushBackMultiGroupToBuffer(std::deque<ALPEventGroup*>::iterator first, std::deque<ALPEventGroup*>::iterator last);

public:
    bool next(unsigned int nsteps = 1, bool read = true);      //��ȡ���ض�����������
    bool prev(unsigned int nsteps = 1, bool read = true);      //��ȡǰ���ض�����������
    bool first(bool read = true);                              //��ȡ��һ������
    bool last(bool read = true);                               //��ȡ���һ������
    bool reset();                                              //���ö�ȡ������
    void* get();                                               //�������

private:
    //��֤����������в��롢ɾ������ѯ�ض�״̬��������ʱ����������Ч����
    GBSemaphore m_EventGroup_IteratorValid_Lock;
    //ת��Processed״̬��������ʱ������ָ��"��Ҫת�Ƶ������鷶Χ"�ĵ���������֤�����������Ч����
    GBSemaphore m_EventGroup_TransferIteratorValid_Lock;
    //ÿ��ָ��ָ��һ��������
    std::deque<ALPEventGroup*> m_EventGroupQueue;

private:
    // following interfaces are not supported
    ALPEventGroupQueue(const ALPEventGroupQueue&);
    ALPEventGroupQueue& operator=(const ALPEventGroupQueue&);
};


#endif
