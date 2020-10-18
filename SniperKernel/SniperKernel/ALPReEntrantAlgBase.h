#ifndef SNIPER_ALP_REENTRANT_ALG_BASE_H
#define SNIPER_ALP_REENTRANT_ALG_BASE_H

#include <cstdint>
#include "SniperKernel/ALPAlgBase.h"

class ALPDAGTunnelEventContext;

#define const ReEntrant

//ֻ�ܴ��������������͵Ĳ���
typedef ALPDAGTunnelEventContext ReEntrant_EventView;   //�������������Ļ���
typedef uint64_t   ReEntrant_EventID;                          //��������ID
                                                               //��ʾ��������


class ALPReEntrantAlgBase : public ALPAlgBase
{
public:
	ALPReEntrantAlgBase(const std::string& name) :ALPAlgBase(name) { m_AlgType = "ReEntrantAlg"; }
	virtual ~ALPReEntrantAlgBase();
	//�㷨Ϊ�������㷨
	bool IsReEntrant() const { return true; }
	bool IsClonable()  const { return false; }

public:
	//���βδ���ı����⣬���з�������ʹ��extern�ⲿ����


private:
	//���е�˽�б�������Ϊ����,����ʹ�ó���ָ�룬����ֹʹ��ָ�볣��


};


#endif
