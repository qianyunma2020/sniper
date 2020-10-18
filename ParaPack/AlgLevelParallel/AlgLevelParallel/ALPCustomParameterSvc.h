#ifndef SNIPER_ALP_CUSTOM_PARAMETER_SVC_H
#define SNIPER_ALP_CUSTOM_PARAMETER_SVC_H

#include <string>
#include <vector>
#include "SniperKernel/SvcBase.h"
#include "SniperKernel/gbmacrovar.h"

class ALPCustomParameterSvc final : public SvcBase
{
public:
    ALPCustomParameterSvc(const std::string& name);
    ~ALPCustomParameterSvc();
   
    std::string GetInputFilesNameString() { return m_EventPool_InputFilesNameString; }
    unsigned long GetInputThreadsCount()  { 
        if(m_EventPool_InputThreadsCount < 1)
            m_EventPool_InputThreadsCount = 1;
        return m_EventPool_InputThreadsCount; 
    }
    std::string GetOutputFilesNameString() { return m_EventPool_OutputFilesNameString; }
    std::string GetCollectionNamesFilterString() { return m_EventPool_CollectionNamesFilterString; }
    
    std::string GetAlgsPropertyFieName() { return m_AlgsPropertyFieName; } 
    long int GetThreadPoolThreadsCount() { 
        if(m_ThreadPool_ThreadsCount < 0) 
            m_ThreadPool_ThreadsCount = 1;
        return m_ThreadPool_ThreadsCount; 
    }
    unsigned long int GetEventPoolGroupCount() { 
        if(m_EventPool_GroupCount < EventPool_GroupDefaultCount)
            m_EventPool_GroupCount = EventPool_GroupDefaultCount;
        return m_EventPool_GroupCount; 
    }
    void SetEventPoolGroupCount(const unsigned long int count) { m_EventPool_GroupCount=count; }
    unsigned long int GetDAGTunnelsCount() { 
        if(m_DAGTunnelsCount < 1)
            m_DAGTunnelsCount = 1;
        return m_DAGTunnelsCount; 
    }
    void SetDAGTunnelsCount(const unsigned long int count) { m_DAGTunnelsCount=count; } 

public:
    bool initialize() { return true; }
    bool finalize() { return true; }
    void show(int indent) { return; }
     


private:
    std::string m_EventPool_InputFilesNameString;           //ÊäÈëÎÄ¼þÃû³Æ¼¯ºÏ×Ö·û´®
    long int m_EventPool_InputThreadsCount;             //ÊäÈëÏß³ÌÊýÄ¿£¬Ä¬ÈÏÎª1 
    std::string m_EventPool_OutputFilesNameString;          //Êä³öÎÄ¼þÃû³ÆÁÐ±í
    std::string m_EventPool_CollectionNamesFilterString;    //ÓÃ»§Ö¸¶¨µÄCollection¼¯ºÏ¹ýÂËÆ÷×Ö·û´®

    std::string m_AlgsPropertyFieName;                   //´æ´¢Ëã·¨ÊôÐÔµÄJSONÎÄ¼þÃû³Æ    
    long int m_ThreadPool_ThreadsCount;                  //Ïß³Ì³ØµÄÏß³ÌÊýÄ¿
    long int m_EventPool_GroupCount;            //ÊÂÀý³ØµÄÊÂÀý×éÊýÄ¿
    long int m_DAGTunnelsCount;                 //DAGÍ¨µÀÊýÄ¿ 

};

#endif
