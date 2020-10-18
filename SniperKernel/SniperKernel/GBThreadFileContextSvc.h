#ifndef GBThreadFileContextSvc_H
#define GBThreadFileContextSvc_H
#include "SniperKernel/SvcBase.h"

class GBThreadFileContextSvc : public SvcBase
{
public:
	GBThreadFileContextSvc(const std::string& name) :SvcBase(name) {}
	~GBThreadFileContextSvc() {}
	//���������Ļ���Ϊ��Ӧ�ļ�
	virtual bool SetThreadFileContext(unsigned long fileindex = 0, bool initflag = false) = 0;
};

inline bool GBThreadFileContextSvc::SetThreadFileContext(unsigned long fileindex, bool initflag)
{
	return true;
}

#endif // GBThreadFileContextSvc_H
