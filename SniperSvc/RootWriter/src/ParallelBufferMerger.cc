#include <iostream>
#include <unistd.h>
#include "RootWriter/ParallelBufferMerger.h"
#include "SniperKernel/SvcFactory.h"
#include "SniperKernel/SniperLog.h"
#include "TROOT.h"

DECLARE_SERVICE(ParallelBufferMerger);

ParallelBufferMerger* ParallelBufferMerger::m_SingleInstance = new ParallelBufferMerger("ParallelBufferMerger");


ParallelBufferMerger::ParallelBufferMerger(const std::string& name) : SvcBase(name)
{ 
   declProp("Output", m_LogicToPhysical_FileNameMap);
 
   m_BufferList_Lock.notify();
   m_MergeFile_InitFlag_Lock.notify();
   m_MergeFile_InitFlag = false;

   m_MultiThreadMultFile_BufferList_Lock.notify();
   m_MultiThreadMultFile_InitFlag_Lock.notify();
   m_MultiThreadMultFile_InitFlag = false;

}

ParallelBufferMerger::~ParallelBufferMerger()
{

}

bool ParallelBufferMerger::initialize()
{
    if(m_MultiThreadMultFile_InitFlag) return true;
   
    ROOT::EnableThreadSafety(); 
    m_MultiThreadMultFile_InitFlag_Lock.wait();
    if(!m_MultiThreadMultFile_InitFlag)
    {
        for(std::map<std::string, std::string>::iterator it = m_LogicToPhysical_FileNameMap.begin(); it != m_LogicToPhysical_FileNameMap.end(); ++it)
        {
            CreateLogicPhysicalFileMap(it->first, (it->second).data());
        }

    }
    m_MultiThreadMultFile_InitFlag_Lock.notify();
    return true;
}

bool ParallelBufferMerger::finalize()
{
    return true;
}


ParallelBufferMerger* ParallelBufferMerger::GetSingleInstance()
{
   return m_SingleInstance;
}


void ParallelBufferMerger::SetVarMergeFileNameMap(FileMap tmpFileNameMap)
{
        FileMap::iterator beg = tmpFileNameMap.begin();
        FileMap::iterator end = tmpFileNameMap.end();
        m_LogicToPhysical_FileNameMap.insert(beg, end);
}



/*****************************************************************************SingleFileMultiThread********************************************************************************************/

void ParallelBufferMerger::SetSingleFileName(const char* filename)
{
    if(m_MergeFile_InitFlag)  return;
    
    m_MergeFile_InitFlag_Lock.wait();
    if(!m_MergeFile_InitFlag)
    {
        ROOT::EnableThreadSafety();
        m_BufferMerger_TFile = new ROOT::Experimental::TBufferMerger(filename);
        m_BufferMerger_TFile->SetAutoSave(1024);
        m_MergeFile_InitFlag =true;
    }
    m_MergeFile_InitFlag_Lock.notify();
    return;
}

void ParallelBufferMerger::BookThreadBufferSingleFile()
{
    std::thread::id tmp_ThreadID = std::this_thread::get_id();
    m_BufferList_Lock.wait();
    m_ThreadBuffer_List[tmp_ThreadID] = m_BufferMerger_TFile->GetFile();
    m_BufferList_Lock.notify();
}

void ParallelBufferMerger::WriteThreadBufferSingleFile()
{     
    std::thread::id tmp_ThreadID = std::this_thread::get_id();
    m_ThreadBuffer_List[tmp_ThreadID]->Write();
    m_BufferList_Lock.wait();
    m_ThreadBuffer_List[tmp_ThreadID]=NULL;
    m_ThreadBuffer_List.erase(tmp_ThreadID);

    LogInfo <<"ParallelBufferMerger::WriteThreadBuffer()  implys m_ThreadBuffer_List.size() is:"<<m_ThreadBuffer_List.size()<<std::endl;
    if(m_ThreadBuffer_List.empty())
    {
        LogInfo <<"ParallelBufferMerger::WriteThreadBuffer()  implys step 1."<<std::endl;
        delete  m_BufferMerger_TFile;
        LogInfo <<"ParallelBufferMerger::WriteThreadBuffer()  implys step 2."<<std::endl;
    }
    m_BufferList_Lock.notify();
}


/****************************************************************************MultiFileMultiThread************************************************************************************************/

bool ParallelBufferMerger::CreateLogicPhysicalFileMap(const std::string& Key, const char* filename)
{
    std::map<std::string, ROOT::Experimental::TBufferMerger*>::iterator it = m_KeyToMergeTFile.find(Key);
    if ( it == m_KeyToMergeTFile.end() )
    {
      //  const char* preDir = gDirectory->GetPath();
        LogInfo <<"ParallelBufferMerger::CreateLogicPhysicalFileMap()  parameter Key="<<Key<<",  filename="<<filename<<".  "<<std::endl;
        m_KeyToMergeTFile[Key] = new ROOT::Experimental::TBufferMerger(filename);
        LogInfo <<"ParallelBufferMerger::CreateLogicPhysicalFileMap() size of m_KeyToMergeTFile is:"<<m_KeyToMergeTFile.size()<<std::endl;
        if(NULL == m_KeyToMergeTFile[Key]) LogInfo<<"ParallelBufferMerger::CreateLogicPhysicalFileMap() m_KeyToMergeTFile[Key] is NULL!"<<std::endl;
        m_KeyToMergeTFile[Key]->SetAutoSave(1024);
      //  gDirectory->cd(preDir);
        return true;
    }
    return false;
}

void ParallelBufferMerger::BookThreadBufferMultiFile(const std::string& Key)
{
    std::thread::id tmp_ThreadID = std::this_thread::get_id();
    m_MultiThreadMultFile_BufferList_Lock.wait();
    LogInfo <<"ParallelBufferMerger::BookThreadBufferMultiFile()  parameter Key="<<Key<<", size of m_KeyToMergeTFile is"<<m_KeyToMergeTFile.size()<<" ."<<std::endl;
    if(NULL == m_KeyToMergeTFile[Key]) LogInfo<<"ParallelBufferMerger::BookThreadBufferMultiFile() m_KeyToMergeTFile[Key] is NULL!"<<std::endl;
    m_MultiThreadMultFile_BufferList[std::make_pair(tmp_ThreadID,Key)] = m_KeyToMergeTFile[Key]->GetFile();
    m_MultiThreadMultFile_BufferList_Lock.notify();    
}

/*
template<class Type>
bool ParallelBufferMerger::CreateThreadBufferToTreeMap(const std::string& Key, Type* obj)
{
    std::thread::id tmp_ThreadID = std::this_thread::get_id();
    TDirectory* pDir = m_MultiThreadMultFile_BufferList[make_pair(tmp_ThreadID,Key)].get();
    obj->SetDirectory(pDir);
    return (pDir!=0);     
}
*/

void ParallelBufferMerger::WriteThreadBufferMultiFile(const std::string& Key)
{
    std::thread::id tmp_ThreadID = std::this_thread::get_id();
    m_MultiThreadMultFile_BufferList[std::make_pair(tmp_ThreadID,Key)]->Write();

    m_MultiThreadMultFile_BufferList_Lock.wait();
    m_MultiThreadMultFile_BufferList[std::make_pair(tmp_ThreadID,Key)] = NULL;
    m_MultiThreadMultFile_BufferList.erase(std::make_pair(tmp_ThreadID,Key));

    LogInfo <<"ParallelBufferMerger::WriteThreadBufferMultiFile()  implys m_ThreadBuffer_List.size() is:"<<m_MultiThreadMultFile_BufferList.size()<<std::endl;
    if(m_MultiThreadMultFile_BufferList.empty())
    {
        LogInfo <<"ParallelBufferMerger::WriteThreadBufferMultiFile()  implys step 1."<<std::endl;
         
        for(std::map<std::string, ROOT::Experimental::TBufferMerger*>::iterator tmp_Iterator = m_KeyToMergeTFile.begin(); tmp_Iterator != m_KeyToMergeTFile.end(); tmp_Iterator++)
        {
            delete  tmp_Iterator->second;
        }
        m_KeyToMergeTFile.clear();
        LogInfo <<"ParallelBufferMerger::WriteThreadBufferMultiFile()  implys step 2."<<std::endl;
    }   
    m_MultiThreadMultFile_BufferList_Lock.notify();
}

