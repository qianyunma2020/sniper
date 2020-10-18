#include "SniperKernel/ALPEventUnit.h"
#include "SniperKernel/SniperLog.h"

ALPEventUnit::ALPEventUnit(void* tmpEventDataPtr) {
    //LogDebug<<"Function ALPEventUnit::ALPEventUnit() says gouzao hello."<<std::endl;
    m_EventDataPtr = tmpEventDataPtr;
}

ALPEventUnit::ALPEventUnit(uint64_t tmpEventID, unsigned long int tmpRunID, void* tmpEventDataPtr)
{
    m_EventID = tmpEventID;
    m_RunID = tmpRunID;
    m_EventDataPtr = tmpEventDataPtr;
}

ALPEventUnit::~ALPEventUnit()
{
    m_EventDataPtr = NULL;
}

void ALPEventUnit::SetEventDataPtr(void* tmpDataPtr) { 
    // LogDebug<<"Function ALPEventUnit::SetEventData() says hello."<<std::endl;
    m_EventDataPtr = tmpDataPtr; 
    // LogDebug<<"Function ALPEventUnit::SetEventData() says byebye."<<std::endl;
}

