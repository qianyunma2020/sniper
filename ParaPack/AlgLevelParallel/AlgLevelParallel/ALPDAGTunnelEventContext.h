#ifndef SNIPER_ALP_DAG_TUNNEL_EVENT_CONTEXT_H
#define SNIPER_ALP_DAG_TUNNEL_EVENT_CONTEXT_H

#include <cstdint>
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/ALPEventUnit.h"

//DAG通道当前的事例上下文环境
class ALPDAGTunnelEventContext
{
public:
	ALPDAGTunnelEventContext();
	~ALPDAGTunnelEventContext();
	
	//判断文件是否发生更改
	bool FileOccurAlteration(const std::string filename);
	//设置文件上下文环境
	ALPReturnCode SetFileContext(const std::string filename);
	//重设文件上下文环境
	void ResetFileContext();

	//设置事例上下文环境
	ALPReturnCode SetCurrentEventContext(ALPEventUnit* tmpEventUnitPtr);
	//重设事例上下文环境
	ALPReturnCode ReSetEventContext();

	//获取当前事例数据的指针
	void* GetEventDataPtr();

private:
	ALPEventUnit* m_EventUnitPtr;   //指向事例单元的指针
	std::string m_InputFileName;    //输入文件名称

};


#endif
