#include "SniperKernel/ALPEventGroupQueue.h"
#include "SniperKernel/SniperLog.h"

ALPEventGroupQueue::ALPEventGroupQueue() {
    //�ź������ĳ�ʼ��
    m_EventGroup_IteratorValid_Lock.notify();
    m_EventGroup_TransferIteratorValid_Lock.notify();
}

ALPEventGroupQueue::~ALPEventGroupQueue() {
    for (auto element : m_EventGroupQueue) {
        delete element;
    }
    m_EventGroupQueue.clear();
}

//�ڶ���β������һ��������,��ֹ�ļ����Buffer���ôκ���,��Bufferֻ��ͨ��ת�������������
void ALPEventGroupQueue::PushBackOneGroup(ALPEventGroup* tmpEventGroupPtr) {
    //��Ҫ��֤ת�Ƶ�������Ч,����
    m_EventGroup_TransferIteratorValid_Lock.wait();
    m_EventGroup_IteratorValid_Lock.wait();
    //ѹ��������ָ��
    m_EventGroupQueue.push_back(tmpEventGroupPtr);
    //�ͷ���
    m_EventGroup_IteratorValid_Lock.notify();
    m_EventGroup_TransferIteratorValid_Lock.notify();
}

//�ͷŶ���ǰ���׸�������ָ��
bool ALPEventGroupQueue::PopFrontOneGroup() {
    m_EventGroup_IteratorValid_Lock.wait();
    m_EventGroupQueue.pop_front();
    m_EventGroup_IteratorValid_Lock.notify();
    return true;
}



//���ٶ���ͷ�����飬�ͷ���ռ�õ��ڴ棬�����ļ�����������е���������е���
bool ALPEventGroupQueue::DestroyHeadGroup() {
    m_EventGroup_IteratorValid_Lock.wait();
    //����������ռ�õ��ڴ�ռ�
    delete m_EventGroupQueue.front();
    m_EventGroupQueue.pop_front();
    m_EventGroup_IteratorValid_Lock.notify();
    return true;
}

//���䴦�����ݾ���״̬��������,��û�����ݾ����������飬�򷵻�NULL
ALPEventGroup* ALPEventGroupQueue::AllocateReadyEventGroup() {
    ALPEventGroup* tmpEGPtr = NULL;
    m_EventGroup_IteratorValid_Lock.wait();
    //����Ready״̬�������飬��δ���ҵ��򷵻�NULL
    for (auto element : m_EventGroupQueue) {
        if (element->IsReady()) {
            tmpEGPtr = element;
            tmpEGPtr->SetGroupState(EventIOQueueStream_GroupState_Running);
            break;
        }     
    }
    m_EventGroup_IteratorValid_Lock.notify();
    return tmpEGPtr;
}

//�ͷŶ���ǰ�˶��������ָ��
bool ALPEventGroupQueue::PopFrontMultiGroup(std::deque<ALPEventGroup*>::iterator first, std::deque<ALPEventGroup*>::iterator last) {
    m_EventGroup_IteratorValid_Lock.wait();
    m_EventGroupQueue.erase(first, last);
    m_EventGroup_IteratorValid_Lock.notify();
    return true;
}

//�����������ָ��ѹ���ļ����Buffer���к��
bool ALPEventGroupQueue::PushBackMultiGroupToBuffer(std::deque<ALPEventGroup*>::iterator first, std::deque<ALPEventGroup*>::iterator last) {
    m_EventGroup_IteratorValid_Lock.wait();
    m_EventGroupQueue.insert(m_EventGroupQueue.end(), first, last);
    m_EventGroup_IteratorValid_Lock.notify();
    return true;
}

/*���ڽ�������ת�Ƶ��ļ����������������ǰ����������д���Processed״̬��������ת�Ƶ�Ŀ�����,
  ��һ������ΪĿ����У��ڶ���������¼ת�Ƶ���������Ŀ��������ʹ���βκ�ʵ��ֱ�ӽ���ֵ����*/
ALPReturnCode ALPEventGroupQueue::TransferProcessedEventGroupToFileOutputBuffer(ALPEventGroupQueue* tmpQueue, unsigned long int& count) {
    //��Ҫ��֤ת�Ƶ�������Ч
    m_EventGroup_TransferIteratorValid_Lock.wait();
    std::deque<ALPEventGroup*>::iterator first = m_EventGroupQueue.begin();
    std::deque<ALPEventGroup*>::iterator last = m_EventGroupQueue.begin();
    //��ȡ����ǰ����������Processed״̬������ĵ�������Χ
    while ( (last!=m_EventGroupQueue.end()) && (*last)->IsProcessed() )  
    {
        last++;
        count++;
    }
    if (first != last) {
        //ת��Processed״̬������
        tmpQueue->PushBackMultiGroupToBuffer(first, last);
        PopFrontMultiGroup(first, last);
    }
    m_EventGroup_TransferIteratorValid_Lock.notify();
    return ALPReturnCode::SUCCESS;
}

//�����ļ�������������ã����䴦�������Ѵ������״̬��������,��û�������Ѵ�����ɵ������飬�򷵻�NULL��
ALPEventGroup* ALPEventGroupQueue::AllocateProcessedEventGroupFromBuffer() {
    //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer says hello."<<std::endl;
    ALPEventGroup* tmpEGPtr = NULL;
    //���ļ�������ԭ����˳�����������������,�����׸�����Processed״̬��������
    m_EventGroup_IteratorValid_Lock.wait();
    //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer step1."<<std::endl;
    for (auto element : m_EventGroupQueue) {
        if (element->IsRelease()) {
            //��������
            //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer step2."<<std::endl;
            continue;
        }else if (element->IsFileWriting()) {
            //��һ������߳�����д���ļ����򷵻�nullָ�룬��ǰ�̳߳��Բ��������ļ����У��������߳�ͬʱдһ���ļ�
            //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer step3."<<std::endl;
            m_EventGroup_IteratorValid_Lock.notify();
            return NULL;
        }else if (element->IsProcessed()) {
            //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer step4."<<std::endl;
            //�ҵ�����Processed״̬��������
            tmpEGPtr = element;
            //����������״̬
            tmpEGPtr->SetGroupState(EventIOQueueStream_GroupState_FileWriting);
            //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer step5."<<std::endl;
            break;
        }else{
            LogError << "Function ALPEventGroupQueue::AllocateProcessedEventGroupFromBuffer() implys event group state error!" << std::endl;
            m_EventGroup_IteratorValid_Lock.notify();
            //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer step6."<<std::endl;
            return NULL;
        }
    }
    //�ͷ���������ָ��
    //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer says byebye."<<std::endl;
    m_EventGroup_IteratorValid_Lock.notify();
    return tmpEGPtr;
}


