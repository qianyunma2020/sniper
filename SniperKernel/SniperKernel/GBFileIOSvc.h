#ifndef GBFileIOSvc_H
#define GBFileIOSvc_H
#include "SniperKernel/SvcBase.h"
#include "EventIOQueueStream.h"

class GBFileIOSvc : public SvcBase
{
public:
	 GBFileIOSvc(const std::string& name) :SvcBase(name) {}
	 ~GBFileIOSvc() {}
	 //���������ļ����뵽GlobalBuffer������
	 virtual long InputFileIntoEventGroup(unsigned long stepsize = 1, EventIOQueueStream* tmp_EventIOQueueStream=NULL) = 0;       
	 //��������GlobalBuffer������������ļ�
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
