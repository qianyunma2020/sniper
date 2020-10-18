#include <cstdint>
#include  "SniperKernel/ALPDAGAlgorithmNodeState.h"

constexpr unsigned short transition(ALPDAGAlgorithmNodeState::AlgState oldstate, ALPDAGAlgorithmNodeState::AlgState newstate)
{
    return static_cast<unsigned short>(oldstate) * 256 + static_cast<unsigned short>(newstate);
}

//reset algorithm node status
void ALPDAGAlgorithmNodeState::ResetAlgNodeState() {
    m_AlgorithmState = INITIAL; 
}

//…Ë÷√À„∑®◊¥Ã¨
ALPReturnCode ALPDAGAlgorithmNodeState::SetAlgNodeState(ALPDAGAlgorithmNodeState::AlgState newState) {
    //LogDebug<<"Class ALPDAGAlgorithmNodeState function SetAlgState value: m_AlgorithmState="<<m_AlgorithmState<<", newState="<<newState<<"."<<std::endl;
    switch (transition(m_AlgorithmState, newState)) {
    /*case transition(INITIAL, CONTROLREADY):
    case transition(CONTROLREADY, DATAREADY):*/
    case transition(INITIAL, DATAREADY):
    case transition(DATAREADY, RUNNING):
    case transition(RUNNING, ERROR):
    case transition(RUNNING, EVTACCEPTED):
    case transition(RUNNING, EVTREJECTED):
        m_AlgorithmState = newState;
        return ALPReturnCode::SUCCESS;
    default:
        LogError << "Algorithm state transition from " << m_AlgorithmState
            << "  to  " << newState << "is illegal!" << std::endl;
        m_AlgorithmState = ERROR;
        return ALPReturnCode::FAILURE;
    }
}


constexpr unsigned short transitionNodes(ALPDAGAlgNodesState::AlgState oldstate, ALPDAGAlgNodesState::AlgState newstate)
{
    return static_cast<unsigned short>(oldstate) * 256 + static_cast<unsigned short>(newstate);
}



ALPReturnCode ALPDAGAlgNodesState::SetAlgState(unsigned long tmpAlgIndex, ALPDAGAlgNodesState::AlgState newState)
{
    if (tmpAlgIndex >= m_AlgsStates.size()) {
        LogError << "Index of Algrithom is out of bound." << std::endl;
        return ALPReturnCode::FAILURE;
    }
    switch (transitionNodes(m_AlgsStates[tmpAlgIndex], newState)) {
    /*case transitionNodes(INITIAL, CONTROLREADY):
    case transitionNodes(CONTROLREADY, DATAREADY):*/
    case transitionNodes(INITIAL, DATAREADY):
    case transitionNodes(DATAREADY, RUNNING):
    case transitionNodes(RUNNING, ERROR):
    case transitionNodes(RUNNING, EVTACCEPTED):
    case transitionNodes(RUNNING, EVTREJECTED):
        m_AlgsStates[tmpAlgIndex] = newState;
        return ALPReturnCode::SUCCESS;
    default:
        LogError << "Algorithm index is :" << tmpAlgIndex << ", the state transition from " << m_AlgsStates[tmpAlgIndex]
            << "  to  " << newState << "is illegal!" << std::endl;
        m_AlgsStates[tmpAlgIndex] = ERROR;
        return ALPReturnCode::FAILURE;
    }

}
