#include "SniperKernel/ALPIStateMachine.h"

std::ostream& operator<<(std::ostream& os, const ALPIStateMachine& state)
{       
        switch (state.GetState())
        {
        case DISABLED:         return os << "DISABLED";
        case CONFIGURED:   return os << "CONFIGURED";
        case INITIALIZED:  return os << "INITIALIZED";
        case RUNNING:      return os << "RUNNING";
        case ERROR:        return os << "ERROR";
        default:           return os;
        }
}

