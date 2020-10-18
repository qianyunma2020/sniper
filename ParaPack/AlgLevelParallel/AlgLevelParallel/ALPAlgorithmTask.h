#ifndef SNIPER_ALP_ALGORITHM_TASK_H
#define SNIPER_ALP_ALGORITHM_TASK_H

#include <functional>
#include <string>
#include <tbb/task.h>
#include "SniperKernel/ALPAlgBase.h"
#include "ALPDAGTunnelEventContext.h"
#include "SniperKernel/SniperLog.h"

class ALPAlgorithmTask : public tbb::task
{
public:
	ALPAlgorithmTask(ALPAlgBase* alg, void* evtPtr, std::function<ALPReturnCode()> activateOutputDataNodesSet)
		:m_AlgorithmPtr(alg), m_EventDataPtr(evtPtr), m_ActivateOutputDataNodesSet(activateOutputDataNodesSet) {
		
	}
	~ALPAlgorithmTask() {

	}
	//初始化上下文环境，执行计算，更新DAG输出数据结点的状态
	tbb::task* execute() override;  

private:
	ALPAlgBase* m_AlgorithmPtr;   //指向当前算法实例的指针
	void* m_EventDataPtr;         //指向事例数据的指针
	std::function<ALPReturnCode()> m_ActivateOutputDataNodesSet;   //激活输出数据对应DAG数据结点的状态

};



tbb::task* ALPAlgorithmTask::execute()
{
	//执行计算部分
	try
	{
               // LogDebug<<"Class ALPAlgorithmTask function execute() imply step1."<<std::endl;
                if(NULL==m_AlgorithmPtr) 
                    LogError<<"Class ALPAlgorithmTask function execute() error: alg ptr is null!"<<std::endl;
                //LogDebug<<"Class ALPAlgorithmTask function execute() imply step2."<<std::endl;
		m_AlgorithmPtr->ExecuteRelatedAlgTool(m_EventDataPtr);
                //LogDebug<<"Class ALPAlgorithmTask function execute() imply step3."<<std::endl;
	}
	catch (const std::exception& e)
	{
		LogError << "Function ALPAlgorithmTask::execute() throws exception." << std::endl;
	}

	//更新输出数据对应DAG数据结点的状态，
	m_ActivateOutputDataNodesSet();

	return nullptr;
}

#endif
