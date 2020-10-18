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

	//重载execute()
	virtual bool execute() override { return true; }  
	virtual bool execute(void* eventPtr) = 0;
	//算法类型
	const std::string& type() const { return m_AlgType; }
	void  setType(const std::string& type) { m_AlgType = type; }
        void show(int indent) {}
	
	
public:
	//算法是否为可重入算法
	virtual bool IsReEntrant() const { return false; }
	//算法是否为可克隆算法
	virtual bool IsClonable()  const { return true; }
	//算法是否为父子关系树中的叶子算法
	virtual bool IsLeafNode()  const { return true; }


public:
	//调用派生类及其所创建的子算法及工具的initialize()方法
	ALPReturnCode InitializeRelatedAlgTool();
	//调用派生类及其创建的子算法及工具的reInitialize()方法
	ALPReturnCode ReInitializeRelatedAlgTool();
	//调用派生类及其所创建的子算法及工具的start()方法
	ALPReturnCode StartRelatedAlgTool();
	//调用派生类及其所创建的子算法及工具的reStart()方法
	ALPReturnCode ReStartRelatedAlgTool();
	//调用具体算法的execute()方法
	ALPReturnCode ExecuteRelatedAlgTool();
	//调用具体算法的execute(void* eventPtr)方法
	ALPReturnCode ExecuteRelatedAlgTool(void* eventPtr);
	//调用派生类及其所创建的子算法及工具的stop()方法
	ALPReturnCode StopRelatedAlgTool();
	//调用派生类及其所创建的子算法及工具的finalize()方法
	ALPReturnCode FinalizeRelatedAlgTool();
	
	//virtual bool initialize() = 0;
	bool reInitialize();
	bool start();
	bool reStart();
	//virtual bool execute() = 0;
	bool stop();
	//virtual bool finalize() = 0;

	
protected:
	std::string m_AlgType;                 //算法类型,用以标识算法所代表的物理意义
};

#endif
