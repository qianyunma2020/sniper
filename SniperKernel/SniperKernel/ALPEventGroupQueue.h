#ifndef SNIPER_ALP_EVENTGROUP_QUEUE_H
#define SNIPER_ALP_EVENTGROUP_QUEUE_H

#include <thread>
#include <deque>
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/ALPEventGroup.h"

//事例组队列
class ALPEventGroupQueue 
{
public:
    ALPEventGroupQueue();
    ~ALPEventGroupQueue();

public:
    //在队列尾端增加一个事例组,禁止文件输出Buffer调用次函数,该Buffer只能通过转移事例组来添加.
    void PushBackOneGroup(ALPEventGroup* tmpEventGroupPtr);

    //释放队列前端首个事例组指针
    bool PopFrontOneGroup();   
    
    //销毁队列头事例组，释放其占用的内存，仅供文件输出缓冲区中的事例组队列调用
    bool DestroyHeadGroup(); 

    //返回当前事例组数目
    unsigned long int QueueSize() const { return m_EventGroupQueue.size(); }
    //分配处于数据就绪状态的事例组,若没有数据就绪的事例组，则返回NULL
    ALPEventGroup* AllocateReadyEventGroup();
    /*用于将事例组转移到文件输出缓冲区：将当前事例组队列中处于Processed状态的事例组转移到目标队列,
      第一个参数为目标队列，第二个参数记录转移的事例组数目，引用以使得形参和实参直接进行值传递*/
    ALPReturnCode TransferProcessedEventGroupToFileOutputBuffer(ALPEventGroupQueue* tmpQueue, unsigned long int& count);
    //仅供文件输出缓冲区调用：分配处于数据已处理完成状态的事例组,若没有数据已处理完成的事例组，则返回NULL。
    ALPEventGroup* AllocateProcessedEventGroupFromBuffer();
    
private:
    //释放队列前端多个事例组指针
    bool PopFrontMultiGroup(std::deque<ALPEventGroup*>::iterator first, std::deque<ALPEventGroup*>::iterator last);
    //将多个事例组指针压入文件输出Buffer队列后端
    bool PushBackMultiGroupToBuffer(std::deque<ALPEventGroup*>::iterator first, std::deque<ALPEventGroup*>::iterator last);

public:
    bool next(unsigned int nsteps = 1, bool read = true);      //读取后方特定步长的事例
    bool prev(unsigned int nsteps = 1, bool read = true);      //读取前方特定步长的事例
    bool first(bool read = true);                              //读取第一个事例
    bool last(bool read = true);                               //读取最后一个事例
    bool reset();                                              //重置读取数据流
    void* get();                                               //获得事例

private:
    //保证事例组队列中插入、删除、查询特定状态的事例组时，迭代器有效的锁
    GBSemaphore m_EventGroup_IteratorValid_Lock;
    //转移Processed状态的事例组时，存在指向"需要转移的事例组范围"的迭代器，保证这个迭代器有效的锁
    GBSemaphore m_EventGroup_TransferIteratorValid_Lock;
    //每个指针指向一个事例组
    std::deque<ALPEventGroup*> m_EventGroupQueue;

private:
    // following interfaces are not supported
    ALPEventGroupQueue(const ALPEventGroupQueue&);
    ALPEventGroupQueue& operator=(const ALPEventGroupQueue&);
};


#endif
