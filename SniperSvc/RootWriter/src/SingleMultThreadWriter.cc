#include "RootWriter/SingleMultThreadWriter.h"

SingleMultThreadWriter::SingleMultThreadWriter()
{

}

SingleMultThreadWriter::~SingleMultThreadWriter()
{

}

bool SingleMultThreadWriter::registerFile(const char* filename)
{ 
    if(sniper_context.check(Sniper::SysMode::MT))
    {
        ParallelBufferMerger*  tmp_ParallelBufferMerger = ParallelBufferMerger::GetSingleInstance();
        tmp_ParallelBufferMerger->SetSingleFileName(filename);
        LogInfo << "SingleMultThreadWriter::registerFile() Function BookThreadBuffer begins." <<std::endl;
        tmp_ParallelBufferMerger->BookThreadBufferSingleFile();
        LogInfo << "SingleMultThreadWriter::registerFile()  Function BookThreadBuffer ends." <<std::endl;
    }

    return true;
}

bool SingleMultThreadWriter::registerTree(Task* domain, const std::string& fullDirs, TTree* obj)
{
    if(sniper_context.check(Sniper::SysMode::BASIC))
    {
        SniperPtr<RootWriter> m_rw(domain, "RootWriter");
        if ( ! m_rw.valid() )
        {
            LogError << "SingleMultThreadWriter::registerTree() Function failed to get RootWriter instance!" << std::endl;
            return false;
        }
        m_rw->attach(fullDirs, obj);
    }    

    return true;    
}

bool SingleMultThreadWriter::WriteFile()
{
    if(sniper_context.check(Sniper::SysMode::MT))
    {
        LogInfo << "SingleMultThreadWriter::WriteFile() function WriteThreadBuffer() begins." <<std::endl;
        ParallelBufferMerger*  tmp_ParallelBufferMerger = ParallelBufferMerger::GetSingleInstance();
        tmp_ParallelBufferMerger->WriteThreadBufferSingleFile();
        LogInfo << "SingleMultThreadWriter::WriteFile() function WriteThreadBuffer() ends." <<std::endl;
    }

    return true;
}
