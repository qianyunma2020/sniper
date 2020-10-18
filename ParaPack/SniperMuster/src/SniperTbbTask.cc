/* Copyright (C) 2018 Jiaheng Zou <zoujh@ihep.ac.cn>
   This file is part of SNiPER.
 
   SNiPER is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
 
   SNiPER is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.
 
   You should have received a copy of the GNU Lesser General Public License
   along with SNiPER.  If not, see <http://www.gnu.org/licenses/>. */
#include "SniperKernel/SniperLog.h"
#include "SniperTbbTask.h"
#include "MusterContext.h"
#include "SniperKernel/TaskWatchDog.h"
#include <iostream>

SniperTbbTask::SniperTbbTask(TaskWatchDog& snoopy)
    : m_first(true),
      m_snoopy(snoopy),
      m_context(MusterContext::instance())
{
}

SniperTbbTask::~SniperTbbTask()
{
    m_snoopy.finalize();
}

tbb::task* SniperTbbTask::execute()
{
    tbb::task* next = nullptr;
    LogDebug <<"FUnction SniperTbbTask::execute() begins."<<std::endl;
   
    if ( m_first ) {
        if ( ! m_snoopy.config() || ! m_snoopy.initialize() ) {
            return next;
        }
        m_first = false;
    }
    LogDebug <<"FUnction SniperTbbTask::execute() Step One."<<std::endl;   


    if ( m_context.doNext() && m_snoopy.run_once() )
    {
        LogDebug <<"FUnction SniperTbbTask::execute() Step Two."<<std::endl;
        recycle_as_continuation();
        next = this;
        LogDebug <<"FUnction SniperTbbTask::execute() Step Three."<<std::endl;
    }
    LogDebug <<"FUnction SniperTbbTask::execute() Step ends."<<std::endl;

    return next;
}
