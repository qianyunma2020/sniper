#include "SniperKernel/ALPEventGroup.h"
#include "SniperKernel/SniperLog.h"

ALPEventGroup::ALPEventGroup() {
	m_EventGroupState = EventIOQueueStream_GroupState_Empty;
	m_CurentFileName = "";
	//Ϊ��������λ�����ó�ֵ
	m_Inputting_EventIndex = 0;
	m_Running_EventIndex = 0;
	m_Outputting_EventIndex = 0;
}

ALPEventGroup::~ALPEventGroup() {

}

//�����������״̬
void ALPEventGroup::SetGroupState(long tmpState) {
	m_EventGroupState = tmpState;
}

//�����������״̬
long ALPEventGroup::GetGroupState() {
	return m_EventGroupState;
}

//ÿ��������Ŀ
unsigned long  ALPEventGroup::GetGroupSize() {
	return m_Inputting_EventIndex;
}

//�ж��������Ƿ�Ϊ��
bool ALPEventGroup::IsEmpty() {
	return (EventIOQueueStream_GroupState_Empty == m_EventGroupState);
}

//�ж��������Ƿ����ݾ���
bool ALPEventGroup::IsReady() {
	return (EventIOQueueStream_GroupState_Ready == m_EventGroupState);
}

//�������Ƿ��Ѿ��������
bool ALPEventGroup::IsProcessed() {
	//��������������������Ѵ��������������״̬
	if (m_Running_EventIndex == m_Inputting_EventIndex) {
		m_EventGroupState = EventIOQueueStream_GroupState_Processed;
	}
	//������ֻ���������ض���Ŀ���������
	return (EventIOQueueStream_GroupState_Processed == m_EventGroupState);
}

//�������Ƿ������ڴ���д״̬
bool ALPEventGroup::IsFileWriting() {
	return (EventIOQueueStream_GroupState_FileWriting == m_EventGroupState);
}

//�������Ƿ��Ѿ�������ļ� 
bool ALPEventGroup::IsRelease() {
	return (EventIOQueueStream_GroupState_Release == m_EventGroupState);
}

//�������Ƿ��ڴ���״̬
bool ALPEventGroup::IsError() {
	return (EventIOQueueStream_GroupState_Error == m_EventGroupState);
}

//���ص�ǰFileContext��Ӧ���ļ�����  
std::string ALPEventGroup::GetCurrentFileName() {
	return m_CurentFileName;
}

//���õ�ǰFileContext��Ӧ���ļ�����
void ALPEventGroup::SetCurrentFileName(const std::string tmpFileName) {
	m_CurentFileName = tmpFileName;
}

//�����������ڵĵ�ǰ����λ��
void ALPEventGroup::SetRunningEventIndex(unsigned long tmpIndex) {
	m_Running_EventIndex = tmpIndex;
}

//�����̻߳�ȡ��һ������
ALPEventUnit* ALPEventGroup::NextEvent(unsigned long nsteps, bool read) {
	//ֻ���õ�ǰ���е���������λ�ã�����������
	if (!read) {
		m_Running_EventIndex++;
		return nullptr;
	}
	//���õ�ǰ���е���������λ�ã�������������������ָ��
	if (m_Running_EventIndex < m_Inputting_EventIndex) {
		return m_EventPtr[m_Running_EventIndex++];
	}
	return nullptr;
}

//�����̻߳�ȡ��Ӧ��������һ������
ALPEventUnit* ALPEventGroup::PrevEvent(unsigned long nsteps, bool read) {
	//ֻ���õ�ǰ���е���������λ�ã�����������
	if (!read) {
		--m_Running_EventIndex;
		return nullptr;
	}
	//���õ�ǰ���е���������λ�ã�������������������ָ��
	if (m_Running_EventIndex < m_Inputting_EventIndex && m_Running_EventIndex>0) {
		return m_EventPtr[--m_Running_EventIndex];
	}
	return nullptr;
}

//�����̻߳�ȡ��Ӧ�������һ������
ALPEventUnit* ALPEventGroup::HeadEvent(bool read) {
	//ֻ���õ�ǰ���е���������λ�ã�����������
	if (!read) {
		m_Running_EventIndex = 0;
		return nullptr;
	}
	//���õ�ǰ���е���������λ�ã�������������������ָ��
	if (m_Inputting_EventIndex > 0) {
		m_Running_EventIndex = 0;
		return m_EventPtr[m_Running_EventIndex];
	}
	return nullptr;
}

//�����̻߳�ȡ��Ӧ���������һ������
ALPEventUnit* ALPEventGroup::TailEvent(bool read) {
	//ֻ���õ�ǰ���е���������λ�ã�����������
	if (!read) {
		m_Running_EventIndex = m_Inputting_EventIndex - 1;
		return nullptr;
	}

	//���õ�ǰ���е���������λ�ã�������������������ָ��
	if (m_Inputting_EventIndex > 0) {
		m_Running_EventIndex = m_Inputting_EventIndex - 1;
		return m_EventPtr[m_Running_EventIndex];
	}
	return nullptr;
}

//�����߳�������������
bool ALPEventGroup::EventToInput(void* tmpEventPtr) {
	//Խ���飬��ֹ�����СΪ0ʱ�ڴ����
	//LogDebug << "Function ALPEventGroup::EventToInput() says hello."<< std::endl;
	if (m_Inputting_EventIndex >= EventIOQueueStream_GroupSize) {
                LogDebug << "Function ALPEventGroup::EventToInput() step1.m_Inputting_EventIndex="<<m_Inputting_EventIndex << std::endl;
		return false;
	}
        //LogDebug << "Function ALPEventGroup::EventToInput() implys m_Inputting_EventIndex="<<m_Inputting_EventIndex<<", GroupSize="<<EventIOQueueStream_GroupSize << std::endl;
	//��������ָ��,���¼�������
	//LogDebug << "Function ALPEventGroup::EventToInput() step2.tmpEventPtr="<< std::endl;
	m_EventPtr[m_Inputting_EventIndex] = new ALPEventUnit(tmpEventPtr); 
	m_Inputting_EventIndex++;
        //LogDebug << "Function ALPEventGroup::EventToInput() says byebye." << std::endl; 
	
	return true;
}


//�����̻߳�ȡ����
ALPEventUnit* ALPEventGroup::EventToProcess() {
      //  LogDebug<<"Class ALPEventGroup function EventToProcess implys m_Running_EventIndex="<<m_Running_EventIndex<<", m_Inputting_EventIndex="<<m_Inputting_EventIndex<<std::endl;
	if (m_Running_EventIndex < m_Inputting_EventIndex) {
		return m_EventPtr[m_Running_EventIndex++];
	}
	return nullptr;
}

//����̻߳�ȡ���� 
ALPEventUnit* ALPEventGroup::EventToOutput(unsigned long nsteps) {
        //LogDebug<<"Class ALPEventGroup function EventToOutput implys m_Outputting_EventIndex="<<m_Outputting_EventIndex<<", m_Inputting_EventIndex="<<m_Inputting_EventIndex<<std::endl;
	if (m_Outputting_EventIndex < m_Inputting_EventIndex) {
		return m_EventPtr[m_Outputting_EventIndex++];
	}
        LogDebug<<"Class ALPEventGroup function EventToOutput implys return null ptr!"<<std::endl;
	return nullptr;
}


