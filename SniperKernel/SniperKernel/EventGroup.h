#ifndef EVENTGROUP_H
#define EVENTGROUP_H
#include <atomic>
#include <iostream>
#include "gbmacrovar.h"


//事例组类
class EventGroup
{
public:
    EventGroup();
    ~EventGroup();

public:
    bool NextEventInGroup(unsigned long nsteps=1, bool read=true);   //处理线程获取下一个事例
    bool PrevEventInGroup(unsigned long nsteps=1, bool read=true);   //处理线程获取对应事例组上一个事例
    bool FirstEventInGroup(bool read=true);                         //处理线程获取对应事例组第一个事例
    bool LastEventInGroup(bool read=true);                          //处理线程获取对应事例组最后一个事例
    void* GetEventInGroup();                                        // 处理线程获取事例
    void* EventToOutput(unsigned long nsteps=1);                    //输出线程获取事例 

public:    
    void SetEventData(void* tmp_ElementPtr);            //初始化事例数据
    void SetGroupState(long tmp_State);                 //设置事例组的状态
    long GetGroupState();                               //返回事例组的状态
    unsigned long  GetGroupSize();                      //每组事例数目
    bool IsEmpty();                                     //判断事例组是否为空
    bool IsFinished();                                  //事例组是否已经处理完成
	void SetCurrentFileIndex(long tmp_FileIndex);       //设置当前FileContext对应的文件索引号
	long GetCurrentFileIndex();                         //返回当前FileContext对应的文件索引号  
	
public:
    GBSemaphore m_ProcessedState_Semaphore;             // 对应处于Processed状态的事例组数目的信号量

private:
    void SetCurrentEventIndex(unsigned long index);     //设置当前事例的索引

private:
    void* m_EventPtr[EventIOQueueStream_GroupSize];                //指向事例的指针数组
    std::atomic<long>    m_EventGroupState;                        //标识该事例组当前状态
    unsigned long m_EventCount;                                    //标识该事例组实际拥有的事例数目
    unsigned long m_ProcessedEventCount;                           //已经处理的事例数目
    unsigned long m_CurrentEvent_Index;                            //事例组内的当前事例位置
    long long m_OutputEvent_Index;                                 //记录文件输出线程的事例位置
	long m_CurrentFile_Index;                                      //事例组当前FileContext对应的文件索引号,用于延迟加载
};

#endif // EVENTGROUP_H
