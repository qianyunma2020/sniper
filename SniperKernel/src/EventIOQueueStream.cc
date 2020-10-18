#include "SniperKernel/EventIOQueueStream.h"
#include "SniperKernel/SniperLog.h"

/****************************************************************************************************************************
                                          事例数据流添加、删除、清空、销毁事例组的相关操作
*****************************************************************************************************************************/
//标志是否还有事例需要输入GlobalBuffer
bool Sniper_GlobalBuffer_EventInputtingFlag = true;
//与工作线程相关的GBFileReadSvc对象数组，为全局变量
std::vector<GBThreadFileContextSvc*> Sniper_FileReadSvc_PtrVector;

EventIOQueueStream::EventIOQueueStream(unsigned long EventIOQueueStream_GroupAmount)
{
    m_EventGroup_InputIteratorValid_Lock.notify();
    m_EventGroup_StateIteratorValid_Lock.notify();	
    m_ThreadGroup_MapTable_Lock.notify();
    //引用EventIOQueueStream的线程数目
    m_Thread_RefsCount = 0;
    //默认添加特定数目的事例组
    if(EventIOQueueStream_GroupAmount < EventIOQueueStream_GroupMinAmount) { EventIOQueueStream_GroupAmount = EventIOQueueStream_GroupMinAmount; }
    AddMultiGroupTail(EventIOQueueStream_GroupAmount);
}

EventIOQueueStream::~EventIOQueueStream()
{
    LogInfo <<"EventIOQueueStream XiGouHanShu begins."<<std::endl;
    DestroyEventIOQueueStream(); 
    LogInfo <<"EventIOQueueStream XiGouHanShu ends."<<std::endl;
}


/*
void EventIOQueueStream::Initial(unsigned long EventIOQueueStream_GroupAmount)           //初始化
{
    //当前可用的事例组数目
    m_EventGroup_StateIteratorValid_Lock.notify();
    m_ThreadGroup_MapTable_Lock.notify();
    //引用EventIOQueueStream的线程数目
    m_Thread_RefsCount=0;
    //默认添加10个分组
    AddMultiGroupTail(EventIOQueueStream_GroupAmount);
    //同一个文件对应的事例组链表，如果该文件只占用一个事例组，则对应链表为空
}
*/


void EventIOQueueStream::AddOneGroupTail()    //增加一个事例组
{
    //加锁以保证迭代器有效性
    m_EventGroup_InputIteratorValid_Lock.wait();
    m_EventGroup_StateIteratorValid_Lock.wait();
    //建立并初始化事例组, 标识事例组的状态,将事例IO流的指针指向新建的事例组
    EventGroup* tmp_EventGroup= new EventGroup;
    tmp_EventGroup->SetGroupState(EventIOQueueStream_GroupState_Empty);
    m_EventIOQueueStream.push_back(tmp_EventGroup);
    m_EventGroup_EmptyState_Semaphore.notify();	
    //解锁
    m_EventGroup_InputIteratorValid_Lock.notify();
    m_EventGroup_StateIteratorValid_Lock.notify();
    return;         
}

void EventIOQueueStream::AddMultiGroupTail(unsigned long tmp_EventGroup_Count)     //增加多个事例组
{
    //指定数目为零、事例组数目已达到最大则返回空
    if( 0==tmp_EventGroup_Count )
    { LogInfo <<"Function  is warning: the count to be added is Zero!"<<std::endl; return;}   
    //加锁以保证迭代器有效性
    m_EventGroup_InputIteratorValid_Lock.wait();
    m_EventGroup_StateIteratorValid_Lock.wait();
    //建立并初始化事例组, 标识事例组的状态,将事例IO流的指针指向新建的事例组
    for(unsigned long tmp_count=0; tmp_count<tmp_EventGroup_Count; tmp_count++)
    {		
	   EventGroup *tmp_EventGroup= new EventGroup;
	   tmp_EventGroup->SetGroupState(EventIOQueueStream_GroupState_Empty);
           m_EventIOQueueStream.push_back(tmp_EventGroup);     
	   m_EventGroup_EmptyState_Semaphore.notify();       
    }
    //解锁
    m_EventGroup_InputIteratorValid_Lock.notify();
    m_EventGroup_StateIteratorValid_Lock.notify();

    LogInfo << "Function AddMultiGroupTail now group count is:" << m_EventIOQueueStream.size() << std::endl;
    return;
}


bool EventIOQueueStream::DeleteHeadGroup()
{
    //加锁以保证迭代器有效性
    m_EventGroup_InputIteratorValid_Lock.wait();
    m_EventGroup_StateIteratorValid_Lock.wait();
    //删除事例组、指向事例组的指针及对应状态                
    delete m_EventIOQueueStream.front();
    m_EventIOQueueStream.pop_front();
    //解锁
    m_EventGroup_InputIteratorValid_Lock.notify();
    m_EventGroup_StateIteratorValid_Lock.notify();
    return true;
}



bool EventIOQueueStream::SafeDeleteHeadGroup()
{
	//加锁以保证迭代器有效性
	m_EventGroup_InputIteratorValid_Lock.wait();
	m_EventGroup_StateIteratorValid_Lock.wait();
	//若该事例组状态不是Release，则提示无法删除，直接返回	
	std::deque<EventGroup*>::iterator tmp_EventGroup_Iterator = m_EventIOQueueStream.begin();
	if( EventIOQueueStream_GroupState_Release == (*tmp_EventGroup_Iterator)->GetGroupState() )
	{		
		//删除事例组、指向事例组的指针及对应状态		
		delete *tmp_EventGroup_Iterator;				
		m_EventIOQueueStream.pop_front();
		//解锁
		m_EventGroup_InputIteratorValid_Lock.notify();
		m_EventGroup_StateIteratorValid_Lock.notify();
		return true;
	}
	else
	{
		//解锁
		m_EventGroup_InputIteratorValid_Lock.notify();
		m_EventGroup_StateIteratorValid_Lock.notify();
		LogInfo << "Function EventIOQueueStream::DeleteHeadGroup() warning: the state of this EventGroup is not release, can't be delete now! If you want, you can destroy it." << std::endl; 
		return false;
	}
}

void EventIOQueueStream::DestroyHeadGroup()                //销毁队列首部事例组
{
	//加锁以保证迭代器有效性
	m_EventGroup_InputIteratorValid_Lock.wait();
	m_EventGroup_StateIteratorValid_Lock.wait();

	std::deque<EventGroup*>::iterator tmp_EventGroup_Iterator = m_EventIOQueueStream.begin();
	//若该事例组处于工作状态
	if (EventIOQueueStream_GroupState_Empty == (*tmp_EventGroup_Iterator)->GetGroupState())
		m_EventGroup_EmptyState_Semaphore.wait();
	else if (EventIOQueueStream_GroupState_Ready == (*tmp_EventGroup_Iterator)->GetGroupState())
		m_EventGroup_ReadyState_Semaphore.wait();
	else if (EventIOQueueStream_GroupState_Processed == (*tmp_EventGroup_Iterator)->GetGroupState())
		(*tmp_EventGroup_Iterator)->m_ProcessedState_Semaphore.wait();
	(*tmp_EventGroup_Iterator)->SetGroupState(EventIOQueueStream_GroupState_Release);			
	delete *tmp_EventGroup_Iterator;
	m_EventIOQueueStream.pop_front();	

	//解锁
	m_EventGroup_InputIteratorValid_Lock.notify();
	m_EventGroup_StateIteratorValid_Lock.notify();
	return;	
}

void EventIOQueueStream::ClearEventIOQueueStream()                  //释放事例数据流
{
	//加锁以保证迭代器有效性
	m_EventGroup_InputIteratorValid_Lock.wait();
	m_EventGroup_StateIteratorValid_Lock.wait();

	for(std::deque<EventGroup*>::iterator tmp_EventGroup_Iterator = m_EventIOQueueStream.begin(); tmp_EventGroup_Iterator < m_EventIOQueueStream.end(); )
	{
		tmp_EventGroup_Iterator++;			
		if( DeleteHeadGroup() )
			AddOneGroupTail();			
		else
		{
			//解锁
			m_EventGroup_InputIteratorValid_Lock.notify();
			m_EventGroup_StateIteratorValid_Lock.notify();
			LogInfo << "Function EventIOQueueStream::ClearEventIOQueueStream() implys Release state event group in head are free." << std::endl;
			return;
		}
	}

	//解锁
	m_EventGroup_InputIteratorValid_Lock.notify();
	m_EventGroup_StateIteratorValid_Lock.notify();
	LogInfo << "Function EventIOQueueStream::ClearEventIOQueueStream() implys all event group are free." << std::endl;
	return ;
}

void EventIOQueueStream::DestroyEventIOQueueStream()                    //强制销毁事例数据流
{
    LogDebug <<"EventIOQueueStream's Function DestroyEventIOQueueStream for looop begins."<<std::endl;
    //加锁以保证迭代器有效性
    m_EventGroup_InputIteratorValid_Lock.wait();
    m_EventGroup_StateIteratorValid_Lock.wait();

    for(std::deque<EventGroup*>::iterator tmp_EventGroup_Iterator = m_EventIOQueueStream.begin(); tmp_EventGroup_Iterator<m_EventIOQueueStream.end(); )
    {
          delete *tmp_EventGroup_Iterator;
	  tmp_EventGroup_Iterator++;
          m_EventIOQueueStream.pop_front();
    }
    LogDebug <<"EventIOQueueStream's Function DestroyEventIOQueueStream for loop ends."<<std::endl;
   
    //解锁
    m_EventGroup_InputIteratorValid_Lock.notify();
    m_EventGroup_StateIteratorValid_Lock.notify();

    //引用EventIOQueueStream的线程数目
    m_Thread_RefsCount=0;
    //同一个文件对应的事例组链表，如果该文件只占用一个事例组，则对应链表为空
    LogDebug <<"EventIOQueueStream's Function DestroyEventIOQueueStream ends."<< std::endl;
}

//为文件输入线程寻找第一个空闲事例组 
EventGroup* EventIOQueueStream::PointerofGroupEmpty()
{
	m_EventGroup_EmptyState_Semaphore.wait();
	EventGroup* tmp_EventGroup_Ptr = NULL;
	m_EventGroup_InputIteratorValid_Lock.wait();
	for (std::deque<EventGroup*>::iterator tmp_EventGroup_Iterator = m_EventIOQueueStream.begin(); tmp_EventGroup_Iterator < m_EventIOQueueStream.end(); tmp_EventGroup_Iterator++)
	{
		if (EventIOQueueStream_GroupState_Empty == (*tmp_EventGroup_Iterator)->GetGroupState())
		{
                        LogInfo << "Function EventIOQueueStream::PointerofGroupEmpty() Index is:"<<tmp_EventGroup_Iterator-m_EventIOQueueStream.begin()<<std::endl;
			tmp_EventGroup_Ptr = *tmp_EventGroup_Iterator;			
			m_EventGroup_InputIteratorValid_Lock.notify();
			return tmp_EventGroup_Ptr;
		}
	}
	m_EventGroup_InputIteratorValid_Lock.notify();
	return NULL;
}

//为新线程查找首个可用的事例组。若返回NULL, 则标识所有数据处理完成，线程将结束运行。
EventGroup* EventIOQueueStream::PointerOfGroupToRun() 
{
	m_EventGroup_ReadyState_Semaphore.wait();
	EventGroup* tmp_EventGroup_Ptr = NULL;
	m_EventGroup_StateIteratorValid_Lock.wait();
	for(std::deque<EventGroup*>::iterator tmp_EventGroup_Iterator = m_EventIOQueueStream.begin(); tmp_EventGroup_Iterator<m_EventIOQueueStream.end(); tmp_EventGroup_Iterator++)
	{
		if( EventIOQueueStream_GroupState_Ready == (*tmp_EventGroup_Iterator)->GetGroupState() )
		{
                        LogInfo << "Function EventIOQueueStream::PointerOfGroupToRun()  Index is:"<<tmp_EventGroup_Iterator-m_EventIOQueueStream.begin()<<std::endl;
			tmp_EventGroup_Ptr = *tmp_EventGroup_Iterator;
			tmp_EventGroup_Ptr->SetGroupState(EventIOQueueStream_GroupState_Running);
			m_EventGroup_StateIteratorValid_Lock.notify();
			return tmp_EventGroup_Ptr;
		}
	}
	m_EventGroup_StateIteratorValid_Lock.notify();
	return NULL;
}

//为输出文件线程返回首个事例组的指针,因只有输出线程的插入和删除操作会改变索引位置，所以此函数的索引位置是线程安全的
EventGroup* EventIOQueueStream::PointerOfHeadGroupToOutput()
{
   if ((!Sniper_GlobalBuffer_EventInputtingFlag) && (EventIOQueueStream_GroupState_Empty == m_EventIOQueueStream.front()->GetGroupState()))
   {
        LogInfo<<"All data is processed and outputted now. The job will ends." <<std::endl;
        return NULL;
   }

   m_EventIOQueueStream.front()->m_ProcessedState_Semaphore.wait();
   if(EventIOQueueStream_GroupState_Empty == m_EventIOQueueStream.front()->GetGroupState())
   {
       return NULL;
   }
   
   return m_EventIOQueueStream.front();
}
//为输出文件线程返回首个事例组的指针,因只有输出线程的插入和删除操作会改变索引位置，所以此函数的索引位置是线程安全的
EventGroup* EventIOQueueStream::SafePointerOfHeadGroupToOutput()
{	
	if ((!Sniper_GlobalBuffer_EventInputtingFlag) && (EventIOQueueStream_GroupState_Empty == m_EventIOQueueStream[0]->GetGroupState()))
	{
		LogInfo<<"All data is processed and outputted now. The job will ends." <<std::endl;
                LogDebug<< "EventIOQueueStream::PointerOfHeadGroupToOutput() Function Sniper_GlobalBuffer_EventInputtingFlag="<<Sniper_GlobalBuffer_EventInputtingFlag<<",  m_EventIOQueueStream[0]->GetGroupState()="<<m_EventIOQueueStream[0]->GetGroupState()<<".  "<<std::endl;
		return NULL;
	}
        
      //  LogInfo << "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): this group state is "<<m_EventIOQueueStream[0]->GetGroupState()<<std::endl;      

        std::deque<EventGroup*>::iterator tmp_EventGroup_Iterator = m_EventIOQueueStream.begin();
        EventGroup* tmp_EventGroup_Ptr = *tmp_EventGroup_Iterator;
        //LogInfo << "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): this group state is "<<tmp_EventGroup_Ptr->GetGroupState()<<std::endl;
        tmp_EventGroup_Ptr->m_ProcessedState_Semaphore.wait();
        //LogInfo<< "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): the count of event is:" <<tmp_EventGroup_Ptr->GetGroupSize()<<std::endl;
        //LogInfo << "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): this group state is "<<tmp_EventGroup_Ptr->GetGroupState()<<std::endl;
        if(EventIOQueueStream_GroupState_Empty == tmp_EventGroup_Ptr->GetGroupState())
        {
             return NULL;
        }
     
        return tmp_EventGroup_Ptr;  
/*
        LogInfo << "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): this group state is "<<m_EventIOQueueStream[0]->GetGroupState()<<std::endl;
        LogInfo << "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): group 22  state is "<<m_EventIOQueueStream[22]->GetGroupState()<<std::endl;
        LogInfo << "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): group 23  state is "<<m_EventIOQueueStream[23]->GetGroupState()<<std::endl;
        LogInfo << "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): the size of queue stream is: "<<m_EventIOQueueStream.size()<<std::endl;
	m_EventIOQueueStream[0]->m_ProcessedState_Semaphore.wait();
        LogInfo << "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): the count of event is:" <<m_EventIOQueueStream[0]->GetGroupSize()<<std::endl;
        LogInfo << "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): the count of group 22 event is:" <<m_EventIOQueueStream[22]->GetGroupSize()<<std::endl;
        LogInfo << "Function EventIOQueueStream::PointerOfHeadGroupToOutput(): the count of group 23 event is:" <<m_EventIOQueueStream[23]->GetGroupSize()<<std::endl;
	return m_EventIOQueueStream[0];     */

}


void EventIOQueueStream::UnlockHeadGroupToOutput()            //解决输出线程卡死的问题，由输入线程先没有数据，后更新相关变量引起 
{
        EventGroup* tmp_EventGroup_Ptr = NULL;
        m_EventGroup_InputIteratorValid_Lock.wait();
        for (std::deque<EventGroup*>::iterator tmp_EventGroup_Iterator = m_EventIOQueueStream.begin(); tmp_EventGroup_Iterator < m_EventIOQueueStream.end(); tmp_EventGroup_Iterator++)
        {
                if (EventIOQueueStream_GroupState_Empty == (*tmp_EventGroup_Iterator)->GetGroupState())
                {
                        tmp_EventGroup_Ptr = *tmp_EventGroup_Iterator;
                        tmp_EventGroup_Ptr->m_ProcessedState_Semaphore.notify();
                }
        }
        m_EventGroup_InputIteratorValid_Lock.notify();

}



/****************************************************************************************************************************
                                        线程从事例数据流中读取事例数据
                               以下方法中返回的事例指针由事例组号和事例组内的当前事例位置共同决定
*****************************************************************************************************************************/

bool EventIOQueueStream::next(unsigned int stepsize, bool read)//读取后方特定步长的事例
{
        LogDebug <<"Function EventIOQueueStream::next() comes in."<<std::endl;
        
        //当前线程是否为新加入的线程
        std::thread::id tmp_ThreadID=std::this_thread::get_id();
        std::map<std::thread::id, EventGroup*>::iterator tmp_Iterator=m_ThreadGroup_MapTable.find(tmp_ThreadID);
        if(tmp_Iterator != m_ThreadGroup_MapTable.end())
        {
            //不是新的线程，则继续访问对应事例组
            EventGroup* tmp_EventGroupPtr = tmp_Iterator->second;
            LogDebug <<"The ID of this thread is:"<<tmp_ThreadID<<" , Address of Group Ptr is: "<< tmp_EventGroupPtr <<std::endl;
            if( tmp_EventGroupPtr->IsFinished() )
            {
                LogDebug <<"Address of Group Ptr is: "<< tmp_EventGroupPtr <<" Step 1."<<std::endl;
                //如果当前事例组已经处理完成，则封存现有事例组，
		tmp_EventGroupPtr->SetGroupState(EventIOQueueStream_GroupState_Processed);
		tmp_EventGroupPtr->m_ProcessedState_Semaphore.notify();
                LogInfo <<"Thread"<<tmp_ThreadID<<" finished this group, no event any more."<<std::endl;
                LogDebug <<"Address of Group Ptr is: "<< tmp_EventGroupPtr <<" Step 2."<<std::endl;

                
		        //寻找新的可用事例组,当没有事例输入时，则线程统一阻塞在此，最后由Global Buffer统一释放
	        tmp_EventGroupPtr = PointerOfGroupToRun();
                if(NULL == tmp_EventGroupPtr)
                {
                    //没有可用的事例组，则删除映射表中对应线程ID
                    m_ThreadGroup_MapTable_Lock.wait();
                    m_ThreadGroup_MapTable.erase(tmp_ThreadID);
                    m_ThreadGroup_MapTable_Lock.notify();
                    LogWarn <<"Function  Warning:No ready state data for this thread!"<<std::endl;
                    return false;
                }
                
                LogDebug <<"Address of Group Ptr is: "<< tmp_EventGroupPtr <<" Step 3."<<std::endl;               
                //更新映射表
                m_ThreadGroup_MapTable[tmp_ThreadID]= tmp_EventGroupPtr;


                LogDebug <<"Function EventIOQueueStream::next() step 4 wolfflag."<<std::endl; 
		//更新线程对应的文件上下文环境
		unsigned long tmp_FileIndex = tmp_EventGroupPtr->GetCurrentFileIndex();
                LogDebug <<"Function EventIOQueueStream::next() step 5 wolfflag."<<std::endl;
		if (tmp_FileIndex != m_ThreadFileIndex_MapTable[tmp_ThreadID])
		{
                        LogDebug <<"Function EventIOQueueStream::next() step 6 wolfflag."<<std::endl;
			GBThreadFileContextSvc* tmp_FileContextSvc = m_ThreadFileContext_MapTable[tmp_ThreadID];
			tmp_FileContextSvc->SetThreadFileContext(tmp_FileIndex, false);
			m_ThreadFileIndex_MapTable[tmp_ThreadID] = tmp_FileIndex;
                        LogDebug <<"Function EventIOQueueStream::next() step 7 wolfflag."<<std::endl;
		}

                LogDebug <<"Function EventIOQueueStream::next() step 8 wolfflag."<<std::endl;   
	


							
                //读取第一个事例
                LogInfo <<"Old Thread:"<<tmp_ThreadID<<", Address of Group Ptr is:"<< tmp_EventGroupPtr <<"read First event successful."<<std::endl;
                return tmp_EventGroupPtr->FirstEventInGroup(read);
            }
            else {
                //返回下一个事例
                return tmp_EventGroupPtr->NextEventInGroup(stepsize, read);                
            }
        }
        else
        {
			//新的线程加入，则查找新的待处理事例组
            LogInfo <<"Function EventIOQueueStream::next() implys a new thread appears."<<std::endl;

            EventGroup* tmp_EventGroupPtr = PointerOfGroupToRun();
            if(NULL == tmp_EventGroupPtr)
            {
                  LogWarn <<"Function Warn: All data is processed, there is no data any more!"<<std::endl; 
		  return false;
	    }           
            //更新映射表，建立映射，返回下一个事例
            m_ThreadGroup_MapTable_Lock.wait();
            m_ThreadGroup_MapTable.insert(make_pair(tmp_ThreadID, tmp_EventGroupPtr));
			


	    //更新线程对应的文件上下文环境
            GBThreadFileContextSvc* tmp_FileContextSvc = Sniper_FileReadSvc_PtrVector.back();
	    Sniper_FileReadSvc_PtrVector.pop_back();
	    m_ThreadFileContext_MapTable.insert(make_pair(tmp_ThreadID, tmp_FileContextSvc));
	    unsigned long tmp_FileIndex = tmp_EventGroupPtr->GetCurrentFileIndex();

            LogDebug <<"Function EventIOQueueStream::next() step 1 wolfblueflag.FileIndex="<<tmp_FileIndex<<std::endl;
	    tmp_FileContextSvc->SetThreadFileContext(tmp_FileIndex, true);
            LogDebug <<"Function EventIOQueueStream::next() step 2 wolfblueflag."<<std::endl;

	    m_ThreadFileIndex_MapTable.insert(make_pair(tmp_ThreadID, tmp_FileIndex));






						
            m_ThreadGroup_MapTable_Lock.notify();
            //读取第一个事例
            return tmp_EventGroupPtr->FirstEventInGroup(read);
            LogInfo <<"New Thread:"<<tmp_ThreadID<<"read First event successful."<<std::endl;
        }
        LogWarn <<"Function EventIOQueueStream::next() return false. Warning!"<<std::endl;
        return false;
}

bool EventIOQueueStream::prev(unsigned int stepsize, bool read)//读取前方特定步长的事例
{
        //当前线程是否为新加入的线程
        std::thread::id tmp_ThreadID=std::this_thread::get_id();
        std::map<std::thread::id, EventGroup*>::iterator tmp_Iterator=m_ThreadGroup_MapTable.find(tmp_ThreadID);
        if(tmp_Iterator != m_ThreadGroup_MapTable.end())
        {
            //不是新的线程，则继续访问对应事例组			
            LogDebug <<"The ID of this thread is:"<<tmp_ThreadID<<"."<<std::endl;
            return tmp_Iterator->second->PrevEventInGroup(stepsize, read);
        }
        else
        {
            //为保护数据安全，只在同一个事例组内才能访问前方事例，新的线程则返回false
            return false;
        }
        return false;
}

bool EventIOQueueStream::first(bool read)//读取第一个事例
{
        //当前线程是否为新加入的线程
        std::thread::id tmp_ThreadID=std::this_thread::get_id();
        std::map<std::thread::id, EventGroup*>::iterator tmp_Iterator=m_ThreadGroup_MapTable.find(tmp_ThreadID);
        if(tmp_Iterator != m_ThreadGroup_MapTable.end())
        {
            //不是新的线程，则继续访问对应事例组的第一个事例			
            LogDebug <<"The ID of this thread is:"<<tmp_ThreadID<<"."<<std::endl;
            return tmp_Iterator->second->FirstEventInGroup(read);
        }
        else
        {
            //新的线程加入，则查找新的待处理事例组 	        
			EventGroup* tmp_EventGroupPtr = PointerOfGroupToRun();
            if(NULL == tmp_EventGroupPtr)
            { 
		          LogWarn <<"Function EventIOQueueStream::first() Warning: All data is processed, there is no data any more!"<<std::endl; 		          
		          return false;
	        }           	        
            m_ThreadGroup_MapTable_Lock.wait();
	        m_ThreadGroup_MapTable.insert(make_pair(tmp_ThreadID, tmp_EventGroupPtr));



			//更新线程对应的文件上下文环境
			GBThreadFileContextSvc* tmp_FileContextSvc = Sniper_FileReadSvc_PtrVector.back();
			Sniper_FileReadSvc_PtrVector.pop_back();
			m_ThreadFileContext_MapTable.insert(make_pair(tmp_ThreadID, tmp_FileContextSvc));
			unsigned long tmp_FileIndex = tmp_EventGroupPtr->GetCurrentFileIndex();
			tmp_FileContextSvc->SetThreadFileContext(tmp_FileIndex, true);			
			m_ThreadFileIndex_MapTable.insert(make_pair(tmp_ThreadID, tmp_FileIndex));
			


            m_ThreadGroup_MapTable_Lock.notify();
            //访问第一个事例
            return tmp_EventGroupPtr->FirstEventInGroup(read);
        }
        return false;
}

bool EventIOQueueStream::last(bool read)//读取最后一个事例
{
    //当前线程是否为新加入的线程
    std::thread::id tmp_ThreadID=std::this_thread::get_id();
    std::map<std::thread::id, EventGroup*>::iterator tmp_Iterator=m_ThreadGroup_MapTable.find(tmp_ThreadID);
    if(tmp_Iterator != m_ThreadGroup_MapTable.end())
    {
        //不是新的线程，则继续访问对应事例组
        LogDebug <<"The ID of this thread is:"<<tmp_ThreadID<<"."<<std::endl;
        return tmp_Iterator->second->LastEventInGroup(read);
    }
    else
    {
        //新的线程加入，则查找新的待处理事例组        
		EventGroup* tmp_EventGroupPtr = PointerOfGroupToRun();
        if(NULL == tmp_EventGroupPtr)
        { 
		      LogWarn <<"Function Error:All data is processed, there is no data any more!"<<std::endl; 
		      return false;
	    }
        //更新映射表，建立映射，访问数据 
        m_ThreadGroup_MapTable_Lock.wait();
	    m_ThreadGroup_MapTable.insert(make_pair(tmp_ThreadID, tmp_EventGroupPtr));



		//更新线程对应的文件上下文环境
		GBThreadFileContextSvc* tmp_FileContextSvc = Sniper_FileReadSvc_PtrVector.back();
		Sniper_FileReadSvc_PtrVector.pop_back();
		m_ThreadFileContext_MapTable.insert(make_pair(tmp_ThreadID, tmp_FileContextSvc));
		unsigned long tmp_FileIndex = tmp_EventGroupPtr->GetCurrentFileIndex();
		tmp_FileContextSvc->SetThreadFileContext(tmp_FileIndex, true);
		m_ThreadFileIndex_MapTable.insert(make_pair(tmp_ThreadID, tmp_FileIndex));



        m_ThreadGroup_MapTable_Lock.notify();

        return tmp_EventGroupPtr->LastEventInGroup(read);
    }
    return false;
}

bool EventIOQueueStream::reset()//重置读取数据流
{
    return IInputStream::reset();
}

void* EventIOQueueStream::get()//获得事例
{   
    LogDebug <<"Function EventIOQueueStream::get() comes in ."<<std::endl; 
    std::thread::id tmp_ThreadID=std::this_thread::get_id();	
    return (m_ThreadGroup_MapTable[tmp_ThreadID]->GetEventInGroup());
}
