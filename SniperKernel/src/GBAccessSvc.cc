#include <iostream>
#include "SniperKernel/SvcFactory.h"
#include "SniperKernel/IInputStream.h"
#include "SniperKernel/GBAccessSvc.h"
#include "SniperKernel/SniperLog.h"

DECLARE_SERVICE(GBAccessSvc);

GBAccessSvc::GBAccessSvc(const std::string& name) : SvcBase(name)
{
}

GBAccessSvc::~GBAccessSvc()
{
}

bool GBAccessSvc::initialize()
{
    LogInfo << "Initializing GBAccessSvc..."<< std::endl;
    if (m_inputStream.find("/Event") != m_inputStream.end())
    {
         LogDebug  << "------------FUnction GBIOSvc::initialize() Found duplicated input stream: /Event.------------"<< std::endl;
         return false;
    }

    IInputStream* iStream = GetInputStream();
    if (!iStream) 
    {
         LogDebug <<"------------FUnction GBIOSvc::initialize() Warning: iStream is NULL!!!!!!!!!!!!!------------"<<std::endl;        
         return false;
    }
    m_inputStream.insert(std::make_pair("/Event", iStream));

    LogDebug << "------------GBIOSvc::initialize() Successfully initialized.------------"<< std::endl;
    return true;
}

bool GBAccessSvc::finalize()
{
   StreamMap::iterator it, end = m_inputStream.end();
 
    for (it = m_inputStream.begin(); it != end; ++it)
    {    it->second =NULL;     }

   /* StreamMap::iterator it, end = m_inputStream.end();
 
    for (it = m_inputStream.begin(); it != end; ++it)
    {   delete it->second;     }
    */
    LogDebug << "Successfully finalized."<< std::endl;
    return true;
}

IInputStream* GBAccessSvc::GetInputStream()
{
    LogDebug <<"Function GetInputStream begins."<<std::endl;
    return static_cast<IInputStream*>(Sniper_GlobalBuffer_Manager->m_GBEventQueue);
}
