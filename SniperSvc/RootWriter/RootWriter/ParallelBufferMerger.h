#ifndef SNIPER_PARALLEL_BUFFER_MERGER_SVC_H
#define SNIPER_PARALLEL_BUFFER_MERGER_SVC_H

#include <boost/python.hpp>
#include <map>
#include <memory>
#include <thread>
#include <utility>
#include <ROOT/TBufferMerger.hxx>
#include "SniperKernel/gbmacrovar.h"
#include "SniperKernel/SvcBase.h"

class TFile;
class TTree;
class TNtuple;
class TDirectory;

using namespace ROOT::Experimental;

class ParallelBufferMerger : public SvcBase
{
public:
    ParallelBufferMerger(const std::string& name);
    ~ParallelBufferMerger();
    
    bool initialize();
    bool finalize();
    static ParallelBufferMerger* GetSingleInstance();            //获取类对象实例
    typedef std::map<std::string, std::string> FileMap;
    void SetVarMergeFileNameMap(FileMap tmpFileNameMap);

private:
    static  ParallelBufferMerger* m_SingleInstance;                                                 //类对应静态实例

/****************************************************************************SingleFileMultThread**********************************************************************************************/
public:
    void BookThreadBufferSingleFile();                            //获取线程对应的Buffer
    void WriteThreadBufferSingleFile();                           //将线程保存的内容写出
    void SetSingleFileName(const char* filename);                 //在第一次调用时创建TBufferMerger,并设置输出文件路经

private:
    std::map<std::thread::id, std::shared_ptr<ROOT::Experimental::TBufferMergerFile> > m_ThreadBuffer_List;   //线程Buffer连表
    GBSemaphore m_BufferList_Lock;                                //线程Buffer链表对应的锁
    GBSemaphore m_MergeFile_InitFlag_Lock;                        //初始化标志对应的锁
    bool m_MergeFile_InitFlag;                                    //MergeFile初始化标志
    ROOT::Experimental::TBufferMerger* m_BufferMerger_TFile;      //TBufferMerge指针

/***************************************************************************MultiFileMultThread************************************************************************************************/
public:
    void BookThreadBufferMultiFile(const std::string& Key);
    void WriteThreadBufferMultiFile(const std::string& Key);
    template<class Type>
    bool CreateThreadBufferToTreeMap(const std::string& Key, Type* obj); 

private:
    //fKey: the tag of a file
    bool CreateLogicPhysicalFileMap(const std::string& Key, const char* filename);
private:
    std::map<std::string, std::string> m_LogicToPhysical_FileNameMap;            //fKey -> TFile
    std::map<std::string, ROOT::Experimental::TBufferMerger*> m_KeyToMergeTFile;
    std::map< std::pair<std::thread::id,const std::string>, std::shared_ptr<ROOT::Experimental::TBufferMergerFile> > m_MultiThreadMultFile_BufferList;   //多文件多线程Buffer链表
    GBSemaphore m_MultiThreadMultFile_BufferList_Lock;                                               //多文件多线程Buffer链表对应的锁
    GBSemaphore m_MultiThreadMultFile_InitFlag_Lock;                                                 //多文件多线程初始化标志对应的锁
    bool m_MultiThreadMultFile_InitFlag;                                                             //多文件多线程MergeFile初始化标志

};

template<class Type>
bool ParallelBufferMerger::CreateThreadBufferToTreeMap(const std::string& Key, Type* obj)
{
    std::thread::id tmp_ThreadID = std::this_thread::get_id();
    TDirectory* pDir = m_MultiThreadMultFile_BufferList[make_pair(tmp_ThreadID,Key)].get();
    obj->SetDirectory(pDir);
    return (pDir!=0);
}



#endif
