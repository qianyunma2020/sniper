#ifndef GBAccessSvc_H
#define GBAccessSvc_H 0

#include "SniperKernel/SvcBase.h"
#include "IInputSvc.h"
#include "GlobalBuffer.h"

class GBAccessSvc : public SvcBase, public IInputSvc 
{
public:
    GBAccessSvc(const std::string& name);
    ~GBAccessSvc();
    bool initialize();
    bool finalize();

private:
    IInputStream* GetInputStream();
};
extern  GlobalBuffer* Sniper_GlobalBuffer_Manager;

#endif
