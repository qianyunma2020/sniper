#include "AlgLevelParallel/ALPDAGTunnelEventContext.h"
#include "SniperKernel/SniperLog.h"

ALPDAGTunnelEventContext::ALPDAGTunnelEventContext()
{
        LogDebug<<"Class ALPDAGTunnelEventContext function ALPDAGTunnelEventContext() says hello."<<std::endl;
	m_EventUnitPtr = NULL;
	m_InputFileName = "";
}

ALPDAGTunnelEventContext::~ALPDAGTunnelEventContext()
{
        LogDebug<<"Class ALPDAGTunnelEventContext function ~ALPDAGTunnelEventContext() says hello."<<std::endl;
	m_EventUnitPtr = NULL;
	m_InputFileName = "";
}

//�ж��ļ��Ƿ�������
bool ALPDAGTunnelEventContext::FileOccurAlteration(const std::string filename) {
        LogDebug<<"Class ALPDAGTunnelEventContext function FileOccurAlteration says hello."<<std::endl;
	return filename != m_InputFileName;
}

//�����ļ������Ļ���
ALPReturnCode ALPDAGTunnelEventContext::SetFileContext(const std::string filename) {
        LogDebug<<"Class ALPDAGTunnelEventContext function SetFileContext says hello."<<std::endl;
	m_InputFileName = filename;
}

//�����ļ������Ļ���
void ALPDAGTunnelEventContext::ResetFileContext() {
        LogDebug<<"Class ALPDAGTunnelEventContext function ResetFileContext says hello."<<std::endl;
	m_InputFileName = "";
}

//�������������Ļ���
ALPReturnCode ALPDAGTunnelEventContext::SetCurrentEventContext(ALPEventUnit* tmpEventUnitPtr) {
        //LogDebug<<"Class ALPDAGTunnelEventContext function SetCurrentEventContext step1."<<std::endl;
	m_EventUnitPtr = tmpEventUnitPtr;
        //LogDebug<<"Class ALPDAGTunnelEventContext function SetCurrentEventContext step2."<<std::endl;
}

//�������������Ļ���
ALPReturnCode ALPDAGTunnelEventContext::ReSetEventContext() {
        //LogDebug<<"Class ALPDAGTunnelEventContext function ReSetEventContext() ssays hello."<<std::endl;
	m_EventUnitPtr = NULL;
}

//��ȡ��ǰ�������ݵ�ָ��
void* ALPDAGTunnelEventContext::GetEventDataPtr() {
	//��������Ƿ�Ϊ��ֵ
        //LogDebug<<"Class ALPDAGTunnelEventContext function GetEventDataPtr says hello."<<std::endl;

	if (NULL == m_EventUnitPtr) {
		LogError << "Function ALPDAGTunnelEventContext::GetEventDataPtr() implys Pointer is NULL !" << std::endl;
		return NULL;
	}
	else
	{
                //LogDebug<<"Class ALPDAGTunnelEventContext function GetEventDataPtr step1."<<std::endl;              
		return m_EventUnitPtr->GetEventDataPtr();
	}
}
