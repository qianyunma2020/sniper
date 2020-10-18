#include "SniperKernel/GlobalBuffer.h"
#include "SniperKernel/SniperLog.h"

//执行GlobalBuffer对象的全局指针
GlobalBuffer* Sniper_GlobalBuffer_Manager=NULL;

GlobalBuffer::GlobalBuffer(GBFileIOSvc* tmp_FileInputSvc, GBFileIOSvc* tmp_FileOutputSvc, unsigned long EventIOQueueStream_GroupAmount)
{
    LogInfo <<"GloabalBuffer begins  ."<<std::endl;

    m_GBEventQueue = new EventIOQueueStream(EventIOQueueStream_GroupAmount);
    LogInfo <<"GloabalBuffer create EventIOQueueStream."<<std::endl;

    m_GBFileInputSvc = tmp_FileInputSvc;
    LogInfo <<"GloabalBuffer create FileInputSvc."<<std::endl;

    m_GBFileOutputSvc = tmp_FileOutputSvc;
    LogInfo <<"GlobalBuffer create FileOutputSvc."<<std::endl;
}

GlobalBuffer::~GlobalBuffer()
{
    LogInfo <<"GlobalBuffer XiGouHanShu begins."<<std::endl;
    if(m_GBEventQueue != NULL)
    {  delete m_GBEventQueue; m_GBEventQueue=NULL; }
    LogInfo <<"GlobalBuffer XiGouHanShu ends."<<std::endl;
}

EventIOQueueStream* GlobalBuffer::EventDataStream()
{
	return m_GBEventQueue;
}

/****************************************************************************************************************************
                           IO线程:从磁盘文件将事例读入全局缓冲区、将全局缓冲区中的事例输出到磁盘文件
*****************************************************************************************************************************/

void* ManageInputGlobalBuffer(void *arg)     //文件输入线程对应函数
{
    GlobalBuffer* tmp_GlobalBuffer = (GlobalBuffer*)arg;
    if (tmp_GlobalBuffer == NULL)
    {
	  LogError << "ManageInputGlobalBuffer Function Error: the pointer of GlobalBuffer is NULL. " << std::endl;
	  return NULL;
    }

    bool flag=true;          
    while (flag)
    {
          //将事例数据从文件读入输入输出事例流
          if(-1 == tmp_GlobalBuffer->m_GBFileInputSvc->InputFileIntoEventGroup(GlobalBuffer_StepSize, tmp_GlobalBuffer->m_GBEventQueue) )
                flag=false;			         
    }
    LogInfo << "GloabalBuffer FileInputSvc implys that all data is inputted already." << std::endl;
    //sleep(600);	
    return NULL;
}

void* ManageOutputGlobalBuffer(void *arg)      //输出文件线程对应函数
{
   // sleep(200);

    GlobalBuffer* tmp_GlobalBuffer = (GlobalBuffer*)arg;
    if (tmp_GlobalBuffer == NULL)
    {
         LogError << "ManageInputGlobalBuffer Function Error: the pointer of GlobalBuffer is NULL. " << std::endl;
	 return NULL;
    }

    if (1 == (tmp_GlobalBuffer->m_GBFileOutputSvc->OutputEventGroupIntoFile(GlobalBuffer_StepSize, tmp_GlobalBuffer->m_GBEventQueue)))
    {
         LogInfo << "GloabalBuffer FileOutputSvc implys that all data is outputted already." << std::endl;
    }
    
    for(int i=0;i<10*GlobalBuffer_Thread_MaxCount;i++)    
    {  tmp_GlobalBuffer->m_GBEventQueue->m_EventGroup_ReadyState_Semaphore.notify();  }

    return NULL;
}

//指向文件输入服务的指针     
GBIOManageFunPtr GlobalBuffer::m_GBInput_Function=ManageInputGlobalBuffer;
//指向文件输出服务的指针
GBIOManageFunPtr GlobalBuffer::m_GBOutput_Function=ManageOutputGlobalBuffer;

//管理输入输出线程
bool GlobalBuffer::ManageIOGlobalBuffer()
{
    //创建输入线程
    if(pthread_create(&m_InputThread_ID,NULL,m_GBInput_Function,(void*)this) !=0)
    { LogError <<"Function warning:Input thread create error!"<<std::endl; return false;}
    //创建输出线程
    if(pthread_create(&m_OututThread_ID,NULL,m_GBOutput_Function,(void*)this) !=0)
    { LogError <<"Function warning:Output thread create error!"<<std::endl; return false;}

    pthread_detach(m_InputThread_ID);
    pthread_detach(m_OututThread_ID);

    return true;
}
