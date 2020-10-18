#ifndef GLOBALBUFFER_H
#define GLOBALBUFFER_H

#include <atomic>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include "gbmacrovar.h"
#include "EventIOQueueStream.h"
#include "GBFileIOSvc.h"

//IO函数指针
typedef void* (*GBIOManageFunPtr)(void*);

class GlobalBuffer
{
public:
    GlobalBuffer(GBFileIOSvc* tmp_InputTaskSvc=NULL, GBFileIOSvc* tmp_OutputTaskSvc=NULL, unsigned long EventIOQueueStream_GroupAmount = 1000);
    ~GlobalBuffer();

public:    
    bool ManageIOGlobalBuffer();                       //管理输入输出线程    
    EventIOQueueStream* EventDataStream();             //返回事例数据流 
   
public:
    //指向输入线程对应函数的指针
    static GBIOManageFunPtr m_GBInput_Function;
    //指向输出线程对应函数的指针
    static GBIOManageFunPtr m_GBOutput_Function;
    //事例数据流
    EventIOQueueStream* m_GBEventQueue;
	//指向文件输入服务的指针
    GBFileIOSvc* m_GBFileInputSvc;
	//指向文件输出服务的指针
    GBFileIOSvc* m_GBFileOutputSvc;

private:
    pthread_t m_InputThread_ID;                       //输入线程ID
    pthread_t m_OututThread_ID;                       //输出线程ID

private :
    // following interfaces are not supported
    GlobalBuffer(const GlobalBuffer&);
    GlobalBuffer& operator=(const GlobalBuffer&);
};

//指向全局缓冲区的指针，唯一一个
extern  GlobalBuffer* Sniper_GlobalBuffer_Manager;

#endif // GLOBALBUFFER_H
