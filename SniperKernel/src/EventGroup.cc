#include <cstring>
#include "SniperKernel/EventGroup.h"
#include "SniperKernel/SniperLog.h"

EventGroup::EventGroup()
{
     LogDebug <<"EventGroup GouzaoHanshu Begins."<<std::endl;
     m_EventCount=0;
     m_ProcessedEventCount=0;//已经处理的事例个数
     m_CurrentEvent_Index=0;
     m_OutputEvent_Index = -1;
     //缓冲区，用于存储一组事例数据
     memset(m_EventPtr,0,EventIOQueueStream_GroupSize*sizeof(void*));
     m_EventGroupState = EventIOQueueStream_GroupState_Empty;
	 m_CurrentFile_Index = -1;
     LogDebug <<"EventGroup GouzaoHanshu ends."<<std::endl;
}

EventGroup::~EventGroup()
{
     LogDebug <<"EventGroup XiGouHanshu begins."<<std::endl;
     //释放事例组中指针指向事例所占的存储空间
     for(unsigned long tmp_count=0; tmp_count < m_EventCount; tmp_count++)
     {
         // delete m_EventPtr[tmp_count];
         m_EventPtr[tmp_count]=NULL;
         //LogDebug <<"EventGroup function free number "<<tmp_count<<" event."<<std::endl;
     }
     m_EventCount = 0;
     m_ProcessedEventCount = 0;
     m_CurrentEvent_Index = 0;
     m_OutputEvent_Index = -1;
	 m_CurrentFile_Index = -1;
     LogDebug <<"EventGroup XiGouHanshu ends."<<std::endl;
}


//初始化事例数据
void EventGroup::SetEventData(void* tmp_ElementPtr)
{
     LogDebug <<"Function EventGroup::SetEventData begins " << this << "  " << tmp_ElementPtr <<std::endl;
     // if ( NULL == tmp_ElementPtr)   return false;
     // if (EventIOQueueStream_GroupSize <= m_EventCount) return false;
     m_EventPtr[m_EventCount]=tmp_ElementPtr;
     m_EventCount++;
     return ;
}

//设置事例组的状态
void EventGroup::SetGroupState(long tmp_State)                 
{
     m_EventGroupState = tmp_State;
}

//返回事例组的状态
long EventGroup::GetGroupState()                               
{
     return m_EventGroupState;
}

//每组事例数目
unsigned long  EventGroup::GetGroupSize()   
{
     return m_EventCount;
}

//判断事例组是否为空
bool EventGroup::IsEmpty()                               
{
	return ( EventIOQueueStream_GroupState_Empty == m_EventGroupState );
}

//事例组是否已经处理完成
bool EventGroup::IsFinished()
{
	if (m_CurrentEvent_Index + 1 >= m_EventCount)
	{
		m_EventGroupState = EventIOQueueStream_GroupState_Processed;
		return true;
	}
     return false;
}

//设置当前FileContext对应的文件索引号
void EventGroup::SetCurrentFileIndex(long tmp_FileIndex)
{
	m_CurrentFile_Index = tmp_FileIndex;
}

//返回当前FileContext对应的文件索引号
long EventGroup::GetCurrentFileIndex()
{
	return m_CurrentFile_Index;
}

//处理线程获取下一个事例
bool EventGroup::NextEventInGroup(unsigned long nsteps, bool read)
{
     LogDebug <<"m_CurrentEvent_Index:"<<m_CurrentEvent_Index<<"    nsteps:"<<nsteps<<"   m_EventCount:"<<m_EventCount<<"  m_ProcessedEventCount:"<<m_ProcessedEventCount<<"   read:"<<read<<std::endl;
     //如果事例组全部事例已经处理完毕，则事例组等待输出，不再接受访问
     if ((m_CurrentEvent_Index +nsteps<m_EventCount) && (m_ProcessedEventCount<=m_EventCount))
     {
          m_CurrentEvent_Index = m_CurrentEvent_Index + nsteps;
          if(read)
               m_ProcessedEventCount++;
          return true;
     }
     LogWarn <<"Warning! Warning! EventGroup::next() is false now!"<<std::endl;
     return false;
}

//处理线程获取对应事例组上一个事例
 bool EventGroup::PrevEventInGroup(unsigned long nsteps, bool read)
 {
     if(m_CurrentEvent_Index -nsteps>0 && (m_ProcessedEventCount<m_EventCount))
     {
           m_CurrentEvent_Index = m_CurrentEvent_Index - nsteps;
           if(read)
                 m_ProcessedEventCount++;
           return true;
      }
      return false;
 }

 //处理线程获取对应事例组第一个事例
 bool EventGroup::FirstEventInGroup(bool read)
 {
     LogDebug <<"EventGroup::first() comes in now!"<<std::endl;
     if(m_EventCount>0)
     {
         m_CurrentEvent_Index = 0;
         if(read)
               m_ProcessedEventCount++;
         LogInfo <<"EventGroup::first() return true."<<std::endl;
         return true;
      }
      LogWarn <<"EventGroup::first() return false. Warning!"<<std::endl;
      return false;
 }

 //处理线程获取对应事例组最后一个事例
 bool EventGroup::LastEventInGroup(bool read)
 {
     if(m_EventCount>0)
     {
         m_CurrentEvent_Index = m_EventCount-1;
         if(read)
                 m_ProcessedEventCount++;
         return true;
     }
     return false;
 }

 // 处理线程获取事例
 void* EventGroup::GetEventInGroup()
 {
     LogDebug <<"Function EventGroup::GetEventInGroup() says hello."<<std::endl;
     return m_EventPtr[m_CurrentEvent_Index];
 }

 //设置当前事例的索引
 void EventGroup::SetCurrentEventIndex(unsigned long index)    
 {
	 m_CurrentEvent_Index = index;
 }
 
 //输出线程获取事例
 void* EventGroup::EventToOutput(unsigned long nsteps)                                           
 {
	 if (m_OutputEvent_Index + nsteps<m_EventCount)
	 {
		 m_OutputEvent_Index = m_OutputEvent_Index + nsteps;		 
                 LogDebug <<"Function EventGroup::EventToOutput() m_OutputEvent_Index="<<m_OutputEvent_Index<<",    nsteps="<<nsteps<<".   "<<std::endl;
		 LogDebug <<"Function EventGroup::EventToOutput " << this << "  " << m_EventPtr[m_OutputEvent_Index] <<std::endl;
		 return m_EventPtr[m_OutputEvent_Index];
	 }
	 LogWarn << "Warning! Warning! EventGroup::EventToOutput() is false now!" << std::endl;
	 return NULL;
 }
