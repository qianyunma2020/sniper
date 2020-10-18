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
	//��ʼ�������Ļ�����ִ�м��㣬����DAG������ݽ���״̬
	tbb::task* execute() override;  

private:
	ALPAlgBase* m_AlgorithmPtr;   //ָ��ǰ�㷨ʵ����ָ��
	void* m_EventDataPtr;         //ָ���������ݵ�ָ��
	std::function<ALPReturnCode()> m_ActivateOutputDataNodesSet;   //����������ݶ�ӦDAG���ݽ���״̬

};



tbb::task* ALPAlgorithmTask::execute()
{
	//ִ�м��㲿��
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

	//����������ݶ�ӦDAG���ݽ���״̬��
	m_ActivateOutputDataNodesSet();

	return nullptr;
}

#endif
