#include <exception>
#include "SniperKernel/ALPAlgBase.h"
#include "SniperKernel/SniperLog.h"


ALPAlgBase::ALPAlgBase(const std::string& name) : AlgBase(name), ALPIStateMachine() {
	//declProp("InputDataSet", m_InputDataSet);
	//declProp("OutputDataSet", m_OutputDataSet);
	//m_CoexistAlgInstance_MaxCount = 1;
	m_AlgType = "common";
}

//调用派生类及其所创建的子算法及工具的initialize()方法
ALPReturnCode ALPAlgBase::InitializeRelatedAlgTool() {

	//如果已经初始化，则直接返回。
	if (IsInitialized())  return ALPReturnCode::SUCCESS;
	//如果算法被禁用，则直接忽略返回
	if (IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//调用派生类的initialize()方法,若成功执行则更新算法状态
		if (initialize())
			return InitializeState();  
	}
	catch (const std::exception& e)
	{
		LogError << "Exception appears in function  ALPAlgBase::InitializeRelatedAlgTool()." << std::endl;
		return ALPReturnCode::FAILURE;
	}
	
	return ALPReturnCode::FAILURE;
}

//调用派生类及其创建的子算法及工具的reInitialize()方法
ALPReturnCode ALPAlgBase::ReInitializeRelatedAlgTool() { 
	//如果算法被禁用，则直接忽略返回
	if (IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//调用派生类的reInitialize()方法,若成功执行则更新算法状态
		if (reInitialize())
			return ReInitializeState(); 
	}
	catch (const std::exception& e)
	{
		LogError << "Exception appears in function  ALPAlgBase::ReInitializeRelatedAlgTool()." << std::endl;
		return ALPReturnCode::FAILURE;
	}
	
	return ALPReturnCode::FAILURE;
}

//调用派生类及其所创建的子算法及工具的start()方法
ALPReturnCode ALPAlgBase::StartRelatedAlgTool() {
	//算法已经运行或处于禁用状态，则直接返回
	if (IsRunning() || IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//调用派生类的start()方法,若成功执行则更新算法状态
		if (start())
			return StartState();  
	}
	catch (const std::exception& e)
	{
		LogError << "Exception appears in function  ALPAlgBase::StartRelatedAlgTool()." << std::endl;
		return ALPReturnCode::FAILURE;
	}
	
	return ALPReturnCode::FAILURE;
}

//调用派生类及其所创建的子算法及工具的reStart()方法
ALPReturnCode ALPAlgBase::ReStartRelatedAlgTool() { 
	//如果算法被禁用，则直接忽略返回
	if (IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//调用派生类的restart()方法,若成功执行则更新算法状态
		if (reStart())
			return ReStartState();
	}
	catch (const std::exception& e)
	{
		LogError << "Exception appears in function  ALPAlgBase::ReStartRelatedAlgTool()." << std::endl;
		return ALPReturnCode::FAILURE;
	}

	return ALPReturnCode::FAILURE;
}

//调用具体算法的execute()方法
ALPReturnCode ALPAlgBase::ExecuteRelatedAlgTool() { 
	//如果算法被禁用，则直接忽略返回
	if (IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//调用派生类的execute()方法,若成功执行则返回成功码
		if (execute())
			return ALPReturnCode::SUCCESS;
	}
	catch (const std::exception& e)
	{
		LogError << "Exception appears in function  ALPAlgBase::ExecuteRelatedAlgTool()." << std::endl;
		return ALPReturnCode::FAILURE;
	}

	return ALPReturnCode::FAILURE; 
}

//调用具体算法的execute(void* eventPtr)方法
ALPReturnCode ALPAlgBase::ExecuteRelatedAlgTool(void* eventPtr) {
	//如果算法被禁用，则直接忽略返回
	if (IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//调用派生类的execute()方法,若成功执行则返回成功码
		if (execute(eventPtr))
			return ALPReturnCode::SUCCESS;
	}
	catch (const std::exception & e)
	{
		LogError << "Exception appears in function  ALPAlgBase::ExecuteRelatedAlgTool()." << std::endl;
		return ALPReturnCode::FAILURE;
	}

	return ALPReturnCode::FAILURE;
}

//调用派生类及其所创建的子算法及工具的stop()方法
ALPReturnCode ALPAlgBase::StopRelatedAlgTool() { 
	//算法已经运行结束或处于禁用状态，则直接返回
	if (IsInitialized() || IsDisabled()) 
		return ALPReturnCode::SUCCESS;

	try
	{
		//调用派生类的stop()方法,若成功执行则更新算法状态
		if (stop())
			return StopState();
	}
	catch (const std::exception& e)
	{
		LogError << "Exception appears in function  ALPAlgBase::StopRelatedAlgTool()." << std::endl;
		return ALPReturnCode::FAILURE;
	}

	return ALPReturnCode::FAILURE; 
}

//调用派生类及其所创建的子算法及工具的finalize()方法
ALPReturnCode ALPAlgBase::FinalizeRelatedAlgTool() { 
	//算法已经finalize或处于禁用状态，则直接返回
	if (IsFinalized() || IsDisabled())
		return ALPReturnCode::SUCCESS;

	try
	{
		//调用派生类的finalize()方法,若成功执行则更新算法状态
		if (finalize())
			return FinalizeState();
	}
	catch (const std::exception& e)
	{
		LogError << "Exception appears in function  ALPAlgBase::FinalizeRelatedAlgTool()." << std::endl;
		return ALPReturnCode::FAILURE;
	}

	return ALPReturnCode::FAILURE;
}

bool ALPAlgBase::reInitialize() {
	return true;
	//下面的功能可能存在较大问题，暂时不予使用
	if (finalize() && initialize())
		return true;
	return false;
}

bool ALPAlgBase::start() {
	return true;
}

bool ALPAlgBase::reStart() {
	if (stop() && start())
		return true;
	return false;
}

bool ALPAlgBase::stop() {
	return true;
}




