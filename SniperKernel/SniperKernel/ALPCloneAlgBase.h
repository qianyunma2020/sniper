#ifndef SNIPER_ALP_CLONE_ALG_BASE_H
#define SNIPER_ALP_CLONE_ALG_BASE_H

#include "SniperKernel/ALPAlgBase.h"

class ALPCloneAlgBase : public ALPAlgBase
{
public:
	ALPCloneAlgBase(const std::string& name) :ALPAlgBase(name) { m_AlgType = "CloneAlg"; }
	virtual ~ALPCloneAlgBase();
	//�㷨Ϊ�ɿ�¡�㷨
	bool IsReEntrant() const { return false; }
	bool IsClonable()  const { return true;  }

public:
	//���з������βα������̰߳�ȫ��,�ҽ�ֹʹ��extern���� 

private:
	//�������б�������Ϊ˽�б���

};


#endif // !SNIPER_ALP_CLONE_ALG_BASE_H
