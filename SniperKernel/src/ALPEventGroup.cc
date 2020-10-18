#include "SniperKernel/ALPEventGroup.h"
#include "SniperKernel/SniperLog.h"

ALPEventGroup::ALPEventGroup() {
	m_EventGroupState = EventIOQueueStream_GroupState_Empty;
	m_CurentFileName = "";
	//为事例索引位置设置初值
	m_Inputting_EventIndex = 0;
	m_Running_EventIndex = 0;
	m_Outputting_EventIndex = 0;
}

ALPEventGroup::~ALPEventGroup() {

}

//设置事例组的状态
void ALPEventGroup::SetGroupState(long tmpState) {
	m_EventGroupState = tmpState;
}

//返回事例组的状态
long ALPEventGroup::GetGroupState() {
	return m_EventGroupState;
}

//每组事例数目
unsigned long  ALPEventGroup::GetGroupSize() {
	return m_Inputting_EventIndex;
}

//判断事例组是否为空
bool ALPEventGroup::IsEmpty() {
	return (EventIOQueueStream_GroupState_Empty == m_EventGroupState);
}

//判断事例组是否数据就绪
bool ALPEventGroup::IsReady() {
	return (EventIOQueueStream_GroupState_Ready == m_EventGroupState);
}

//事例组是否已经处理完成
bool ALPEventGroup::IsProcessed() {
	//如果事例组所有事例均已处理，则更新事例组状态
	if (m_Running_EventIndex == m_Inputting_EventIndex) {
		m_EventGroupState = EventIOQueueStream_GroupState_Processed;
	}
	//包括了只处理其中特定数目事例的情况
	return (EventIOQueueStream_GroupState_Processed == m_EventGroupState);
}

//事例组是否正处于磁盘写状态
bool ALPEventGroup::IsFileWriting() {
	return (EventIOQueueStream_GroupState_FileWriting == m_EventGroupState);
}

//事例组是否已经输出到文件 
bool ALPEventGroup::IsRelease() {
	return (EventIOQueueStream_GroupState_Release == m_EventGroupState);
}

//事例组是否处于错误状态
bool ALPEventGroup::IsError() {
	return (EventIOQueueStream_GroupState_Error == m_EventGroupState);
}

//返回当前FileContext对应的文件名称  
std::string ALPEventGroup::GetCurrentFileName() {
	return m_CurentFileName;
}

//设置当前FileContext对应的文件名称
void ALPEventGroup::SetCurrentFileName(const std::string tmpFileName) {
	m_CurentFileName = tmpFileName;
}

//设置事例组内的当前事例位置
void ALPEventGroup::SetRunningEventIndex(unsigned long tmpIndex) {
	m_Running_EventIndex = tmpIndex;
}

//处理线程获取下一个事例
ALPEventUnit* ALPEventGroup::NextEvent(unsigned long nsteps, bool read) {
	//只重置当前运行的事例索引位置，不返回数据
	if (!read) {
		m_Running_EventIndex++;
		return nullptr;
	}
	//重置当前运行的事例索引位置，并返回所需事例数据指针
	if (m_Running_EventIndex < m_Inputting_EventIndex) {
		return m_EventPtr[m_Running_EventIndex++];
	}
	return nullptr;
}

//处理线程获取对应事例组上一个事例
ALPEventUnit* ALPEventGroup::PrevEvent(unsigned long nsteps, bool read) {
	//只重置当前运行的事例索引位置，不返回数据
	if (!read) {
		--m_Running_EventIndex;
		return nullptr;
	}
	//重置当前运行的事例索引位置，并返回所需事例数据指针
	if (m_Running_EventIndex < m_Inputting_EventIndex && m_Running_EventIndex>0) {
		return m_EventPtr[--m_Running_EventIndex];
	}
	return nullptr;
}

//处理线程获取对应事例组第一个事例
ALPEventUnit* ALPEventGroup::HeadEvent(bool read) {
	//只重置当前运行的事例索引位置，不返回数据
	if (!read) {
		m_Running_EventIndex = 0;
		return nullptr;
	}
	//重置当前运行的事例索引位置，并返回所需事例数据指针
	if (m_Inputting_EventIndex > 0) {
		m_Running_EventIndex = 0;
		return m_EventPtr[m_Running_EventIndex];
	}
	return nullptr;
}

//处理线程获取对应事例组最后一个事例
ALPEventUnit* ALPEventGroup::TailEvent(bool read) {
	//只重置当前运行的事例索引位置，不返回数据
	if (!read) {
		m_Running_EventIndex = m_Inputting_EventIndex - 1;
		return nullptr;
	}

	//重置当前运行的事例索引位置，并返回所需事例数据指针
	if (m_Inputting_EventIndex > 0) {
		m_Running_EventIndex = m_Inputting_EventIndex - 1;
		return m_EventPtr[m_Running_EventIndex];
	}
	return nullptr;
}

//输入线程设置事例数据
bool ALPEventGroup::EventToInput(void* tmpEventPtr) {
	//越界检查，防止数组大小为0时内存崩溃
	//LogDebug << "Function ALPEventGroup::EventToInput() says hello."<< std::endl;
	if (m_Inputting_EventIndex >= EventIOQueueStream_GroupSize) {
                LogDebug << "Function ALPEventGroup::EventToInput() step1.m_Inputting_EventIndex="<<m_Inputting_EventIndex << std::endl;
		return false;
	}
        //LogDebug << "Function ALPEventGroup::EventToInput() implys m_Inputting_EventIndex="<<m_Inputting_EventIndex<<", GroupSize="<<EventIOQueueStream_GroupSize << std::endl;
	//设置事例指针,更新计数变量
	//LogDebug << "Function ALPEventGroup::EventToInput() step2.tmpEventPtr="<< std::endl;
	m_EventPtr[m_Inputting_EventIndex] = new ALPEventUnit(tmpEventPtr); 
	m_Inputting_EventIndex++;
        //LogDebug << "Function ALPEventGroup::EventToInput() says byebye." << std::endl; 
	
	return true;
}


//处理线程获取事例
ALPEventUnit* ALPEventGroup::EventToProcess() {
      //  LogDebug<<"Class ALPEventGroup function EventToProcess implys m_Running_EventIndex="<<m_Running_EventIndex<<", m_Inputting_EventIndex="<<m_Inputting_EventIndex<<std::endl;
	if (m_Running_EventIndex < m_Inputting_EventIndex) {
		return m_EventPtr[m_Running_EventIndex++];
	}
	return nullptr;
}

//输出线程获取事例 
ALPEventUnit* ALPEventGroup::EventToOutput(unsigned long nsteps) {
        //LogDebug<<"Class ALPEventGroup function EventToOutput implys m_Outputting_EventIndex="<<m_Outputting_EventIndex<<", m_Inputting_EventIndex="<<m_Inputting_EventIndex<<std::endl;
	if (m_Outputting_EventIndex < m_Inputting_EventIndex) {
		return m_EventPtr[m_Outputting_EventIndex++];
	}
        LogDebug<<"Class ALPEventGroup function EventToOutput implys return null ptr!"<<std::endl;
	return nullptr;
}


