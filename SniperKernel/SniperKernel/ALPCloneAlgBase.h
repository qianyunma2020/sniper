#ifndef SNIPER_ALP_CLONE_ALG_BASE_H
#define SNIPER_ALP_CLONE_ALG_BASE_H

#include "SniperKernel/ALPAlgBase.h"

class ALPCloneAlgBase : public ALPAlgBase
{
public:
	ALPCloneAlgBase(const std::string& name) :ALPAlgBase(name) { m_AlgType = "CloneAlg"; }
	virtual ~ALPCloneAlgBase();
	//算法为可克隆算法
	bool IsReEntrant() const { return false; }
	bool IsClonable()  const { return true;  }

public:
	//所有方法的形参必须是线程安全的,且禁止使用extern变量 

private:
	//子类所有变量必须为私有变量

};


#endif // !SNIPER_ALP_CLONE_ALG_BASE_H
