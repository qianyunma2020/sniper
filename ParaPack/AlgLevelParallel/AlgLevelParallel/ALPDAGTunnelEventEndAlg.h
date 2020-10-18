#ifndef SNIPER_ALP_DAG_TUNNEL_EVENT_ENDALG_H
#define SNIPER_ALP_DAG_TUNNEL_EVENT_ENDALG_H

#include "SniperKernel/ALPAlgBase.h"

class ALPDAGTunnel;
//DAG Tunnel想要结束当前事例时调用的工具算法
class ALPDAGTunnelEventEndAlg : public ALPAlgBase
{
public:
	ALPDAGTunnelEventEndAlg(const std::string& name) :ALPAlgBase("ALPDAGTunnelEventEndAlg") {}
	~ALPDAGTunnelEventEndAlg(){}
        void SetDAGTunnelPtr(ALPDAGTunnel* tunnel); 
	//算法是否为可重入算法
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
