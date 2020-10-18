#ifndef SNIPER_ALP_DAG_TUNNEL_EVENT_ENDALG_H
#define SNIPER_ALP_DAG_TUNNEL_EVENT_ENDALG_H

#include "SniperKernel/ALPAlgBase.h"

class ALPDAGTunnel;
//DAG Tunnel��Ҫ������ǰ����ʱ���õĹ����㷨
class ALPDAGTunnelEventEndAlg : public ALPAlgBase
{
public:
	ALPDAGTunnelEventEndAlg(const std::string& name) :ALPAlgBase("ALPDAGTunnelEventEndAlg") {}
	~ALPDAGTunnelEventEndAlg(){}
        void SetDAGTunnelPtr(ALPDAGTunnel* tunnel); 
	//�㷨�Ƿ�Ϊ�������㷨
	bool IsReEntrant() const { return false; }
	bool IsClonable() const { return true; }
        bool IsLeafNode()  const { return true; }
	bool initialize() { return true; }
	bool execute();
        bool execute(void* eventPtr);
	bool finalize() { return true; }
	
private:
	ALPDAGTunnel* m_DAGTunnel;
};


#endif
