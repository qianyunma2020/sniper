#ifndef SNIPER_SINGLEMULT_THREADWRITER_H
#define SNIPER_SINGLEMULT_THREADWRITER_H

#include "TTree.h"
#include "SniperKernel/SniperPtr.h"
#include "SniperKernel/SniperContext.h"
#include "SniperKernel/Task.h"
#include "RootWriter/RootWriter.h"
#include "RootWriter/ParallelBufferMerger.h"

class SingleMultThreadWriter
{
public:
    SingleMultThreadWriter();
    ~SingleMultThreadWriter();

    static bool registerFile(const char* filename);
    static bool registerTree(Task* domain, const std::string& fullDirs, TTree* obj);
    static bool WriteFile();

};

#endif
