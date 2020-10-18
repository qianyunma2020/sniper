#include "SniperKernel/ALPEventGroupQueue.h"
#include "SniperKernel/SniperLog.h"

ALPEventGroupQueue::ALPEventGroupQueue() {
    //信号量锁的初始化
    m_EventGroup_IteratorValid_Lock.notify();
    m_EventGroup_TransferIteratorValid_Lock.notify();
}

ALPEventGroupQueue::~ALPEventGroupQueue() {
    for (auto element : m_EventGroupQueue) {
        delete element;
    }
    m_EventGroupQueue.clear();
}

//在队列尾端增加一个事例组,禁止文件输出Buffer调用次函数,该Buffer只能通过转移事例组来添加
void ALPEventGroupQueue::PushBackOneGroup(ALPEventGroup* tmpEventGroupPtr) {
    //需要保证转移迭代器有效,加锁
    m_EventGroup_TransferIteratorValid_Lock.wait();
    m_EventGroup_IteratorValid_Lock.wait();
    //压入事例组指针
    m_EventGroupQueue.push_back(tmpEventGroupPtr);
    //释放锁
    m_EventGroup_IteratorValid_Lock.notify();
    m_EventGroup_TransferIteratorValid_Lock.notify();
}

//释放队列前端首个事例组指针
bool ALPEventGroupQueue::PopFrontOneGroup() {
    m_EventGroup_IteratorValid_Lock.wait();
    m_EventGroupQueue.pop_front();
    m_EventGroup_IteratorValid_Lock.notify();
    return true;
}



//销毁队列头事例组，释放其占用的内存，仅供文件输出缓冲区中的事例组队列调用
bool ALPEventGroupQueue::DestroyHeadGroup() {
    m_EventGroup_IteratorValid_Lock.wait();
    //销毁事例组占用的内存空间
    delete m_EventGroupQueue.front();
    m_EventGroupQueue.pop_front();
    m_EventGroup_IteratorValid_Lock.notify();
    return true;
}

//分配处于数据就绪状态的事例组,若没有数据就绪的事例组，则返回NULL
ALPEventGroup* ALPEventGroupQueue::AllocateReadyEventGroup() {
    ALPEventGroup* tmpEGPtr = NULL;
    m_EventGroup_IteratorValid_Lock.wait();
    //查找Ready状态的事例组，若未查找到则返回NULL
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

//释放队列前端多个事例组指针
bool ALPEventGroupQueue::PopFrontMultiGroup(std::deque<ALPEventGroup*>::iterator first, std::deque<ALPEventGroup*>::iterator last) {
    m_EventGroup_IteratorValid_Lock.wait();
    m_EventGroupQueue.erase(first, last);
    m_EventGroup_IteratorValid_Lock.notify();
    return true;
}

//将多个事例组指针压入文件输出Buffer队列后端
bool ALPEventGroupQueue::PushBackMultiGroupToBuffer(std::deque<ALPEventGroup*>::iterator first, std::deque<ALPEventGroup*>::iterator last) {
    m_EventGroup_IteratorValid_Lock.wait();
    m_EventGroupQueue.insert(m_EventGroupQueue.end(), first, last);
    m_EventGroup_IteratorValid_Lock.notify();
    return true;
}

/*用于将事例组转移到文件输出缓冲区：将当前事例组队列中处于Processed状态的事例组转移到目标队列,
  第一个参数为目标队列，第二个参数记录转移的事例组数目，引用以使得形参和实参直接进行值传递*/
ALPReturnCode ALPEventGroupQueue::TransferProcessedEventGroupToFileOutputBuffer(ALPEventGroupQueue* tmpQueue, unsigned long int& count) {
    //需要保证转移迭代器有效
    m_EventGroup_TransferIteratorValid_Lock.wait();
    std::deque<ALPEventGroup*>::iterator first = m_EventGroupQueue.begin();
    std::deque<ALPEventGroup*>::iterator last = m_EventGroupQueue.begin();
    //获取队列前端连续处于Processed状态事例组的迭代器范围
    while ( (last!=m_EventGroupQueue.end()) && (*last)->IsProcessed() )  
    {
        last++;
        count++;
    }
    if (first != last) {
        //转移Processed状态事例组
        tmpQueue->PushBackMultiGroupToBuffer(first, last);
        PopFrontMultiGroup(first, last);
    }
    m_EventGroup_TransferIteratorValid_Lock.notify();
    return ALPReturnCode::SUCCESS;
}

//仅供文件输出缓冲区调用：分配处于数据已处理完成状态的事例组,若没有数据已处理完成的事例组，则返回NULL。
ALPEventGroup* ALPEventGroupQueue::AllocateProcessedEventGroupFromBuffer() {
    //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer says hello."<<std::endl;
    ALPEventGroup* tmpEGPtr = NULL;
    //按文件中数据原本的顺序分配待输出的事例组,返回首个处于Processed状态的事例组
    m_EventGroup_IteratorValid_Lock.wait();
    //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer step1."<<std::endl;
    for (auto element : m_EventGroupQueue) {
        if (element->IsRelease()) {
            //继续查找
            //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer step2."<<std::endl;
            continue;
        }else if (element->IsFileWriting()) {
            //另一个输出线程正在写该文件，则返回null指针，当前线程尝试查找其他文件队列，避免多个线程同时写一个文件
            //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer step3."<<std::endl;
            m_EventGroup_IteratorValid_Lock.notify();
            return NULL;
        }else if (element->IsProcessed()) {
            //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer step4."<<std::endl;
            //找到处于Processed状态的事例组
            tmpEGPtr = element;
            //更新事例组状态
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
    //释放锁，返回指针
    //LogDebug<<"Class ALPEventGroupQueue function AllocateProcessedEventGroupFromBuffer says byebye."<<std::endl;
    m_EventGroup_IteratorValid_Lock.notify();
    return tmpEGPtr;
}


