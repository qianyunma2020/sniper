#ifndef GBFileIOSvc_H
#define GBFileIOSvc_H
#include "SniperKernel/SvcBase.h"
#include "EventIOQueueStream.h"

class GBFileIOSvc : public SvcBase
{
public:
	 GBFileIOSvc(const std::string& name) :SvcBase(name) {}
	 ~GBFileIOSvc() {}
	 //将事例从文件读入到GlobalBuffer事例组
	 virtual long InputFileIntoEventGroup(unsigned long stepsize = 1, EventIOQueueStream* tmp_EventIOQueueStream=NULL) = 0;       
	 //将事例从GlobalBuffer事例组输出到文件
	 virtual long OutputEventGroupIntoFile(unsigned long stepsize = 1, EventIOQueueStream* tmp_EventIOQueueStream=NULL) = 0;	       
};

inline long GBFileIOSvc::InputFileIntoEventGroup(unsigned long stepsize, EventIOQueueStream* tmp_EventIOQueueStream)
{
     return 0;
}

inline long GBFileIOSvc::OutputEventGroupIntoFile(unsigned long stepsize, EventIOQueueStream* tmp_EventIOQueueStream)
{
	 return 0;
}

#endif // GBFileIOSvc_H
