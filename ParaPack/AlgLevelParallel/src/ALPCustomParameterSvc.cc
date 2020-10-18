#include "SniperKernel/SvcFactory.h"
#include "AlgLevelParallel/ALPCustomParameterSvc.h"

DECLARE_SERVICE(ALPCustomParameterSvc);

ALPCustomParameterSvc::ALPCustomParameterSvc(const std::string& name) : SvcBase(name)
{
    declProp("AlgsPropertyFieName", m_AlgsPropertyFieName);
    declProp("DAGTunnelsCount", m_DAGTunnelsCount);
    declProp("ThreadPoolThreadsCount", m_ThreadPool_ThreadsCount);
    declProp("EventPoolGroupCount", m_EventPool_GroupCount);
    declProp("EventPoolInputThreadsCount", m_EventPool_InputThreadsCount);
    declProp("EventPoolInputFilesNameList", m_EventPool_InputFilesNameString);
    declProp("EventPoolOutputFilesNameList", m_EventPool_OutputFilesNameString);
    declProp("EventPoolCollectionNamesFilterList", m_EventPool_CollectionNamesFilterString);

}
ALPCustomParameterSvc::~ALPCustomParameterSvc(){

}

