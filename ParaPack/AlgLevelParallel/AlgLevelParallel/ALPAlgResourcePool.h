#ifndef SNIPER_ALP_ALG_RESOURCE_POOL_H
#define SNIPER_ALP_ALG_RESOURCE_POOL_H

#include <bitset>
#include <mutex>
#include <list>
#include <vector>
#include <string>
#include <boost/dynamic_bitset.hpp>
#include <tbb/concurrent_queue.h>
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/ALPAlgBase.h"
#include "SniperKernel/SniperLog.h"
#include "SniperKernel/Task.h"
#include "SniperKernel/gbmacrovar.h"

class ALPAlgResourcePool;
//������ʱ�洢�㷨��Դ�ع�����㷨����
class PoolAlgorithmProperty
{
public:
	PoolAlgorithmProperty(){}
	~PoolAlgorithmProperty(){}

private:
	friend class ALPAlgResourcePool;           //���㷨��Դ��������Ϊ��Ԫ��
	std::string m_AlgName;                     //�㷨����
        unsigned int m_MaxInstanceCount;           //������������������1�������n,�����1
};


//��������ɿ�¡�㷨ʵ��ÿ���������������һ�ε��µ���������
class ALPAlgResourcePool : public Task
{
public:
	ALPAlgResourcePool(const std::string& name);
	~ALPAlgResourcePool();

public:
	//�����㷨��
	bool start();
	//ֹͣ�㷨��
	bool stop() override final;
	//run this task
	bool run() override final;
	
protected:
	//the concrete task operations
	bool initialize() override final;
	bool finalize() override final;
	bool config() override final;
	bool execute() override final;
	//clear all svcs and algs
	void reset(){}


public:
	//���ô洢�㷨���Ե�JSON�ļ�����
	void setAlgsPropertyFileName(const std::string filename) {
		m_AlgsPropertyFileName = filename;
	}

	//��ö����㷨�б�
	std::list<ALPAlgBase*> GetTopAlgList();

        //create algorithm instance 
        ALPAlgBase* CreateAlgorithmInsance(const std::string& algName);
	//�����㷨ʵ��
	ALPReturnCode AllocateAlgorithmInstance(const std::string& name, ALPAlgBase*& tmpAlg);
	//�����㷨ʵ��
	ALPReturnCode RecycleAlgorithmInstance(const std::string& name, ALPAlgBase*& tmpAlg);


	//��ȡJSON�ļ����洢�㷨���������㷨������
	ALPReturnCode ParseAlgsPropertyJsonFile();
	//�����û��ű��������ַ������洢�㷨��Դ���������㷨������
	ALPReturnCode ParseUserScriptString();
	

private:
	//������Ӧ�������㷨ʵ��
	ALPReturnCode BuildAlgInstancesResRequirements();

private:
	const std::string m_EndEventAlg = "ALPDAGTunnelEventEndAlg";             //���DAGͨ����ǰ�������н������㷨

        typedef tbb::concurrent_bounded_queue<ALPAlgBase*> ALPConcurrentAlgQueue;
	std::map<std::string, ALPConcurrentAlgQueue*> m_AlgNameToAlgInstanceQueue;  //�㷨�������㷨ʵ�����е�ӳ��
	std::map<std::string, unsigned long> m_AlgNameToMaxInstancesCount;         //ÿ���㷨��Ӧ�����ʵ����Ŀ
	std::map<std::string, unsigned long> m_AlgNameToCreatedInstancesCount;     //ÿ���㷨�Ѿ�������ʵ����Ŀ

	std::string m_m_PoolAlgsPropertySet_String;    //�û��ű��ַ���
	std::string m_AlgsPropertyFileName;            //�洢�㷨���Ե�JSON�ļ�����
	std::vector<PoolAlgorithmProperty*> m_PoolAlgsPropertySet;  //������ʱ�洢�㷨��Դ�ع�����㷨����
 
        std::list<ALPAlgBase*>  m_PoolAlgInstanceList;   //point to all algorithms instances managed by this pool, used for initialize,finalize,start,stop.

   

	//�㷨�ֽ������ܱ���
	std::vector<std::string> m_TopAlgNames;     //�����㷨����

	std::list<ALPAlgBase*>  m_TopAlgPtrist;    //�����㷨����
	std::list<ALPAlgBase*>  m_SubAlgPtrList;    //���㷨����
	std::list<ALPAlgBase*>  m_LeafAlgPtrList;   //Ҷ���㷨����

};

#endif
