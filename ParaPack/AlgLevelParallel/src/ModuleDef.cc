#include "AlgLevelParallel/ALPManagerTask.h"
#include <boost/python.hpp>

namespace bp = boost::python;

BOOST_PYTHON_MODULE(libAlgLevelParallel)
{
    using namespace boost::python;

    bp::class_<ALPManagerTask, boost::noncopyable>("ALPManagerTask", bp::init<const std::string&>())
       .def("run", &ALPManagerTask::run)
       .def("setLogLevel", &ALPManagerTask::setLogLevel)
       .def("createSvc", &ALPManagerTask::createSvc, return_value_policy<reference_existing_object>())
       ;
}


