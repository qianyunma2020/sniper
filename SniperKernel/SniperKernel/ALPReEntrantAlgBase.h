#ifndef SNIPER_ALP_REENTRANT_ALG_BASE_H
#define SNIPER_ALP_REENTRANT_ALG_BASE_H

#include <cstdint>
#include "SniperKernel/ALPAlgBase.h"

class ALPDAGTunnelEventContext;

#define const ReEntrant

//只能传入以下三种类型的参数
typedef ALPDAGTunnelEventContext ReEntrant_EventView;   //传入事例上下文环境
typedef uint64_t   ReEntrant_EventID;                          //传入事例ID
                                                               //显示传入数据


class ALPReEntrantAlgBase : public ALPAlgBase
{
public:
	ALPReEntrantAlgBase(const std::string& name) :ALPAlgBase(name) { m_AlgType = "ReEntrantAlg"; }
	virtual ~ALPReEntrantAlgBase();
	//算法为可重入算法
	bool IsReEntrant() const { return true; }
	bool IsClonable()  const { return false; }

public:
	//除形参传入的变量外，所有方法不得使用extern外部变量


private:
	//所有的私有变量必须为常数,可以使用常量指针，但禁止使用指针常量


};


#endif
