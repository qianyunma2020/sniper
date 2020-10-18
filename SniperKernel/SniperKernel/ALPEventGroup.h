#ifndef SNIPER_ALP_EVENTGROUP_H
#define SNIPER_ALP_EVENTGROUP_H

#include <atomic>
#include <string>
#include "SniperKernel/gbmacrovar.h"
#include"SniperKernel/ALPEventUnit.h"

//事例组类
class ALPEventGroup
{
public:
    ALPEventGroup();
    ~ALPEventGroup();

public:
    ALPEventUnit* NextEvent(unsigned long nsteps = 1, bool read = true);     //处理线程获取下一个事例
    ALPEventUnit* PrevEvent(unsigned long nsteps = 1, bool read = true);     //处理线程获取对应事例组上一个事例
    ALPEventUnit* HeadEvent(bool read = true);                               //处理线程获取对应事例组第一个事例
    ALPEventUnit* TailEvent(bool read = true);                               //处理线程获取对应事例组最后一个事例

    bool EventToInput(void* tmpEventPtr);                           //输入线程设置事例数据
    ALPEventUnit* EventToProcess();                                 //处理线程获取事例
    ALPEventUnit* EventToOutput(unsigned long nsteps = 1);          //输出线程获取事例 

public:
    void SetGroupState(long tmpState);                         //设置事例组的状态
    long GetGroupState();                                      //返回事例组的状态
    void SetCurrentFileName(const std::string tmpFileName);    //设置当前FileContext对应的文件名称
    std::string GetCurrentFileName();                          //返回当前FileContext对应的文件名称  

    unsigned long  GetGroupSize();                             //每组事例数目
    bool IsEmpty();                                            //判断事例组是否为空
    bool IsReady();                                            //判断事例组是否数据就绪
    bool IsProcessed();                                        //事例组是否已经处理完成
    bool IsFileWriting();                                      //事例组是否正处于磁盘写状态
    bool IsRelease();                                          //事例组是否已经输出到文件 
    bool IsError();                                            //事例组是否处于错误状态

    
private:
    void SetRunningEventIndex(unsigned long tmpIndex);     //设置事例组内的当前事例位置

private:
    ALPEventUnit* m_EventPtr[EventIOQueueStream_GroupSize];         //指向事例的指针数组
    std::atomic<long>    m_EventGroupState;                         //标识该事例组当前状态
    unsigned long m_Inputting_EventIndex;                           //记录文件输入线程的事例索引位置
    unsigned long m_Running_EventIndex;                             //记录事例处理线程的事例索引位置
    unsigned long m_Outputting_EventIndex;                          //记录文件输出线程的事例索引位置
    std::string m_CurentFileName;                                   //事例组当前FileContext对应的文件名称,用于延迟加载

private:
    // following interfaces are not supported
    ALPEventGroup(const ALPEventGroup&);
    ALPEventGroup& operator=(const ALPEventGroup&);
};

#endif // !SNIPER_ALP_EVENTGROUP_H
