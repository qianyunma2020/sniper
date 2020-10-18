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
	ALPAlgResourcePool* m_AlgResourcePool;               //ָ���㷨��Դ�ص�ָ��
	ALPThreadPool* m_ThreadPool;                         //ָ���̳߳ص�ָ��
	ALPEventPool* m_EventPool;                           //ָ�������ص�ָ��
	ALPDAG* m_DAG;                                       //ִ��ȫ��DAG��ָ��
	std::vector<ALPDAGTunnel*> m_DAGTunnelsVector;       //DAGͨ��ָ�뼯��

private:
        ALPCustomParameterSvc* m_CustomParameterSvc;     //svc to custom related parameter

/*       std::string m_AlgsPropertyFieName;                   //�洢�㷨���Ե�JSON�ļ�����
        unsigned long int m_ThreadPool_ThreadsCount;         //�̳߳ص��߳���Ŀ
        unsigned long int m_EventPool_GroupCount;            //�����ص���������Ŀ
        unsigned int m_DAGTunnelsCount;                      //DAGͨ����Ŀ        

	unsigned long m_EventPool_InputThreadsCount;             //�����߳���Ŀ��Ĭ��Ϊ1   
	unsigned long m_EventPool_OutputThreadsCount;            //����߳���Ŀ��Ĭ��Ϊ1  
	std::string m_EventPool_InputFilesNameString;           //�����ļ����Ƽ����ַ���
	std::string m_EventPool_OutputFilesNameString;          //����ļ������б�
	std::string m_EventPool_CollectionNamesFilterString;    //�û�ָ����Collection���Ϲ������ַ���
*/

};


#endif
