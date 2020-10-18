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

//判断文件是否发生更改
bool ALPDAGTunnelEventContext::FileOccurAlteration(const std::string filename) {
        LogDebug<<"Class ALPDAGTunnelEventContext function FileOccurAlteration says hello."<<std::endl;
	return filename != m_InputFileName;
}

//设置文件上下文环境
ALPReturnCode ALPDAGTunnelEventContext::SetFileContext(const std::string filename) {
        LogDebug<<"Class ALPDAGTunnelEventContext function SetFileContext says hello."<<std::endl;
	m_InputFileName = filename;
}

//重设文件上下文环境
void ALPDAGTunnelEventContext::ResetFileContext() {
        LogDebug<<"Class ALPDAGTunnelEventContext function ResetFileContext says hello."<<std::endl;
	m_InputFileName = "";
}

//设置事例上下文环境
ALPReturnCode ALPDAGTunnelEventContext::SetCurrentEventContext(ALPEventUnit* tmpEventUnitPtr) {
        //LogDebug<<"Class ALPDAGTunnelEventContext function SetCurrentEventContext step1."<<std::endl;
	m_EventUnitPtr = tmpEventUnitPtr;
        //LogDebug<<"Class ALPDAGTunnelEventContext function SetCurrentEventContext step2."<<std::endl;
}

//重设事例上下文环境
ALPReturnCode ALPDAGTunnelEventContext::ReSetEventContext() {
        //LogDebug<<"Class ALPDAGTunnelEventContext function ReSetEventContext() ssays hello."<<std::endl;
	m_EventUnitPtr = NULL;
}

//获取当前事例数据的指针
void* ALPDAGTunnelEventContext::GetEventDataPtr() {
	//检测事例是否为空值
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
