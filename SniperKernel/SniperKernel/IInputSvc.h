#ifndef I_INPUT_SVC_H
#define I_INPUT_SVC_H 0

#include <string>
#include <map>
#include "IInputStream.h"

class IInputSvc {

    public:
        typedef std::map<std::string, IInputStream*> StreamMap;

        IInputSvc() {}
        virtual ~IInputSvc() {}

        std::map<std::string, IInputStream*>& inputStream() { return m_inputStream; }

    protected:
        StreamMap   m_inputStream;  // Path to input stream map
};


#endif
