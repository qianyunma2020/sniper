#ifndef SNIPER_ALP_DAG_MANAGER_TASK_H
#define SNIPER_ALP_DAG_MANAGER_TASK_H

#include <string>
#include <vector>
#include "SniperKernel/Task.h"
#include "SniperKernel/ALPEventPool.h"
#include "SniperKernel/gbmacrovar.h"
#include "ALPDAG.h"
#include "ALPDAGTunnel.h"
#include "ALPAlgResourcePool.h"
#include "ALPThreadPool.h"
#include "ALPCustomParameterSvc.h"


class ALPManagerTask final : public Task
{
public:
	ALPManagerTask(const std::string& name);
	~ALPManagerTask();

	// override of base class
	bool config() override;
        bool run() override; 
	bool initialize() override;
	bool finalize() override;
 
        //static ALPCustomParameterSvc& CustomParameterSvcInsance() { return m_CustomParameterSvc; }

private:
	ALPAlgResourcePool* m_AlgResourcePool;               //指向算法资源池的指针
	ALPThreadPool* m_ThreadPool;                         //指向线程池的指针
	ALPEventPool* m_EventPool;                           //指向事例池的指针
	ALPDAG* m_DAG;                                       //执行全局DAG的指针
	std::vector<ALPDAGTunnel*> m_DAGTunnelsVector;       //DAG通道指针集合

private:
        ALPCustomParameterSvc* m_CustomParameterSvc;     //svc to custom related parameter

/*       std::string m_AlgsPropertyFieName;                   //存储算法属性的JSON文件名称
        unsigned long int m_ThreadPool_ThreadsCount;         //线程池的线程数目
        unsigned long int m_EventPool_GroupCount;            //事例池的事例组数目
        unsigned int m_DAGTunnelsCount;                      //DAG通道数目        

	unsigned long m_EventPool_InputThreadsCount;             //输入线程数目，默认为1   
	unsigned long m_EventPool_OutputThreadsCount;            //输出线程数目，默认为1  
	std::string m_EventPool_InputFilesNameString;           //输入文件名称集合字符串
	std::string m_EventPool_OutputFilesNameString;          //输出文件名称列表
	std::string m_EventPool_CollectionNamesFilterString;    //用户指定的Collection集合过滤器字符串
*/

};


#endif
