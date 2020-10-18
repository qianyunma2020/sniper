/* Copyright (C) 2018
   Jiaheng Zou <zoujh@ihep.ac.cn> Tao Lin <lintao@ihep.ac.cn>
   Weidong Li <liwd@ihep.ac.cn> Xingtao Huang <huangxt@sdu.edu.cn>
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

#ifndef Hello_h
#define Hello_h

#include "SniperKernel/AlgBase.h"
#include <vector>
#include <map>

class HelloAlg: public AlgBase
{
    public:
        HelloAlg(const std::string& name);

        ~HelloAlg();

        bool initialize();
        bool execute();
        bool finalize();

    private:
        int m_count;

        std::string                 m_string;
        std::vector<int>            m_vector_int;
        std::map<std::string, int>  m_str_int;
};

#endif
