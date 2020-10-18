#include "SniperKernel/AlgFactory.h"
#include "AlgLevelParallel/ALPDAGTunnelEventEndAlg.h"
#include "AlgLevelParallel/ALPDAGTunnel.h"

DECLARE_ALGORITHM(ALPDAGTunnelEventEndAlg);

void ALPDAGTunnelEventEndAlg::SetDAGTunnelPtr(ALPDAGTunnel* tunnel) 
{ 
    m_DAGTunnel = tunnel; 
}

bool ALPDAGTunnelEventEndAlg::execute() {
     //¸üÐÂÊÂÀý½áÊøµÄÐÅºÅÁ¿
     m_DAGTunnel->m_TunnelEventEnd_Semaphore.notify();
     return true;
}

bool ALPDAGTunnelEventEndAlg::execute(void* eventPtr) {
    m_DAGTunnel->m_TunnelEventEnd_Semaphore.notify();
    return true;
}

