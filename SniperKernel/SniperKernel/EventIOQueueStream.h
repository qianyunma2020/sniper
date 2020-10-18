#ifndef EventIOQueueStream_H
#define EventIOQueueStream_H

#include <deque>
#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <unistd.h>
#include "gbmacrovar.h"
#include "EventGroup.h"
#include "IInputStream.h"
#include "GBThreadFileContextSvc.h"


//流中对应事例数组数目的信号量类

class EventIOQueueStream  : public IInputStream
{
public:
     EventIOQueueStream(unsigned long EventIOQueueStream_GroupAmount = 1000);    
     ~EventIOQueueStream();

public:        
     void AddOneGroupTail();                                         //增加一个事例组
     void AddMultiGroupTail(unsigned long tmp_EventGroup_Count);     //增加多个事例组
     bool DeleteHeadGroup();                                         //释放队列首部事例组
     bool SafeDeleteHeadGroup();                                     //释放队列首部事例组

     EventGroup* PointerofGroupEmpty();                              //为文件输入线程寻找第一个空闲事例组 
     EventGroup* PointerOfGroupToRun();                              //为线程查找第一个处于Ready状态的事例组
     //为输出文件线程返回首个事例组的指针,因只有输出线程的插入和删除操作会改变索引位置，所以此函数的索引位置是线程安全的
     EventGroup* PointerOfHeadGroupToOutput();
     EventGroup* SafePointerOfHeadGroupToOutput();
     void UnlockHeadGroupToOutput();            //解决输出线程卡死的问题，由输入线程先没有数据，后更新相关变量引起	 

private:
    void DestroyHeadGroup();                                        //强制销毁队列首部事例组
     void ClearEventIOQueueStream();                                 //释放事例数据流
     void DestroyEventIOQueueStream();                               //强制销毁事例数据流  


public:
     bool next(unsigned int nsteps = 1, bool read = true);      //读取后方特定步长的事例
     bool prev(unsigned int nsteps = 1, bool read = true);      //读取前方特定步长的事例
     bool first(bool read = true);                              //读取第一个事例
     bool last(bool read = true);                               //读取最后一个事例
     bool reset();                                              //重置读取数据流
     void* get();                                               //获得事例

public:
     GBSemaphore m_EventGroup_EmptyState_Semaphore;             //对应处于Empty状态的事例组数目的信号量
     GBSemaphore m_EventGroup_ReadyState_Semaphore;             //对应处于Ready状态的事例组数目的信号量
private:
     GBSemaphore m_EventGroup_InputIteratorValid_Lock;          //查找等待输入的事例组时,保证迭代器有效的锁
     GBSemaphore m_EventGroup_StateIteratorValid_Lock;          //多线程竞争改变Ready状态事例组时,保证迭代器有效的锁 
     GBSemaphore m_ThreadGroup_MapTable_Lock;                   //对应线程与事例组索引映射表的信号锁
	 	 
private:
     std::atomic_ullong m_Thread_RefsCount;                            //当前引用EventIOQueueStream的线程数目
     std::deque<EventGroup*> m_EventIOQueueStream;                     //每个指针指向一组事例
     std::map<std::thread::id, EventGroup*> m_ThreadGroup_MapTable;    //线程与事例组索引映射表
     std::map<std::thread::id, GBThreadFileContextSvc*> m_ThreadFileContext_MapTable;   //线程与GBFileReadSvc对象映射表
     std::map<std::thread::id, unsigned long> m_ThreadFileIndex_MapTable;      //线程与文件索引位置映射表

private :
     // following interfaces are not supported
     EventIOQueueStream(const EventIOQueueStream&);
     EventIOQueueStream& operator=(const EventIOQueueStream&);
};
//标志是否还有事例需要输入GlobalBuffer
extern  bool Sniper_GlobalBuffer_EventInputtingFlag;
//与工作线程相关的GBFileReadSvc对象数组，为全局变量
extern std::vector<GBThreadFileContextSvc*> Sniper_FileReadSvc_PtrVector;

#endif // EventIOQueueStream_H
