#ifndef SNIPER_ALP_ALG_BASE_H
#define SNIPER_ALP_ALG_BASE_H

#include <vector>
#include <string>
#include <algorithm>
#include "SniperKernel/ALPIStateMachine.h"
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/SniperLog.h"
#include "SniperKernel/AlgBase.h"

class ALPAlgBase : public AlgBase, public ALPIStateMachine
{
public:
	ALPAlgBase(const std::string& name);
	virtual ~ALPAlgBase() {}

	//����execute()
	virtual bool execute() override { return true; }  
	virtual bool execute(void* eventPtr) = 0;
	//�㷨����
	const std::string& type() const { return m_AlgType; }
	void  setType(const std::string& type) { m_AlgType = type; }
        void show(int indent) {}
	
	
public:
	//�㷨�Ƿ�Ϊ�������㷨
	virtual bool IsReEntrant() const { return false; }
	//�㷨�Ƿ�Ϊ�ɿ�¡�㷨
	virtual bool IsClonable()  const { return true; }
	//�㷨�Ƿ�Ϊ���ӹ�ϵ���е�Ҷ���㷨
	virtual bool IsLeafNode()  const { return true; }


public:
	//���������༰�������������㷨�����ߵ�initialize()����
	ALPReturnCode InitializeRelatedAlgTool();
	//���������༰�䴴�������㷨�����ߵ�reInitialize()����
	ALPReturnCode ReInitializeRelatedAlgTool();
	//���������༰�������������㷨�����ߵ�start()����
	ALPReturnCode StartRelatedAlgTool();
	//���������༰�������������㷨�����ߵ�reStart()����
	ALPReturnCode ReStartRelatedAlgTool();
	//���þ����㷨��execute()����
	ALPReturnCode ExecuteRelatedAlgTool();
	//���þ����㷨��execute(void* eventPtr)����
	ALPReturnCode ExecuteRelatedAlgTool(void* eventPtr);
	//���������༰�������������㷨�����ߵ�stop()����
	ALPReturnCode StopRelatedAlgTool();
	//���������༰�������������㷨�����ߵ�finalize()����
	ALPReturnCode FinalizeRelatedAlgTool();
	
	//virtual bool initialize() = 0;
	bool reInitialize();
	bool start();
	bool reStart();
	//virtual bool execute() = 0;
	bool stop();
	//virtual bool finalize() = 0;

	
protected:
	std::string m_AlgType;                 //�㷨����,���Ա�ʶ�㷨���������������
};

#endif
