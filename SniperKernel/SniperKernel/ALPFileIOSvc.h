#ifndef SNIPER_ALP_FILE_IOSVC_H
#define SNIPER_ALP_FILE_IOSVC_H

#include <string>
#include "SniperKernel/SvcBase.h"

class ALPEventPool;
class ALPFileIOSvc : public SvcBase
{
public:
	ALPFileIOSvc(const std::string& name) : SvcBase(name) {}
	~ALPFileIOSvc(){}
    
	virtual void SetEventPool(ALPEventPool* eventpool) = 0;
	virtual void SetInputThreadsCount(const unsigned int count = 1) = 0;
	virtual void SetOutputThreadsCount(const unsigned int count = 1) = 0;
	virtual void SetInputFilesNameString(const std::string names) = 0;
	virtual void SetOutputFilesNameString(const std::string names) = 0;
	virtual void SetCollectionNamesFilterString(const std::string filter) = 0;

};

inline void ALPFileIOSvc::SetEventPool(ALPEventPool* eventpool) {
    return;
}

inline void ALPFileIOSvc::SetInputThreadsCount(const unsigned int count) {
    return;
}

inline void ALPFileIOSvc::SetOutputThreadsCount(const unsigned int count) {
    return;
}

inline void ALPFileIOSvc::SetInputFilesNameString(const std::string names) {
    return;
}

inline void ALPFileIOSvc::SetOutputFilesNameString(const std::string names) {
    return;
}

inline void ALPFileIOSvc::SetCollectionNamesFilterString(const std::string filter) {
    return;
}


#endif
