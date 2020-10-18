#include <exception>
#include "SniperKernel/ALPAlgBase.h"
#include "SniperKernel/SniperLog.h"


ALPAlgBase::ALPAlgBase(const std::string& name) : AlgBase(name), ALPIStateMachine() {
	//declProp("InputDataSet", m_InputDataSet);
	//declProp("OutputDataSet", m_OutputDataSet);
	//m_CoexistAlgInstance_MaxCount = 1;
	m_AlgType = "common";
}

//���������༰�������������㷨�����ߵ�initialize()����
ALPReturnCode ALPAlgBase::InitializeRelatedAlgTool() {

	//����Ѿ���ʼ������ֱ�ӷ��ء�
	if (IsInitialized())  return ALPReturnCode::SUCCESS;
	//����㷨�����ã���ֱ�Ӻ��Է���
	if (IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//�����������initialize()����,���ɹ�ִ��������㷨״̬
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

//���������༰�䴴�������㷨�����ߵ�reInitialize()����
ALPReturnCode ALPAlgBase::ReInitializeRelatedAlgTool() { 
	//����㷨�����ã���ֱ�Ӻ��Է���
	if (IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//�����������reInitialize()����,���ɹ�ִ��������㷨״̬
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

//���������༰�������������㷨�����ߵ�start()����
ALPReturnCode ALPAlgBase::StartRelatedAlgTool() {
	//�㷨�Ѿ����л��ڽ���״̬����ֱ�ӷ���
	if (IsRunning() || IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//�����������start()����,���ɹ�ִ��������㷨״̬
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

//���������༰�������������㷨�����ߵ�reStart()����
ALPReturnCode ALPAlgBase::ReStartRelatedAlgTool() { 
	//����㷨�����ã���ֱ�Ӻ��Է���
	if (IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//�����������restart()����,���ɹ�ִ��������㷨״̬
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

//���þ����㷨��execute()����
ALPReturnCode ALPAlgBase::ExecuteRelatedAlgTool() { 
	//����㷨�����ã���ֱ�Ӻ��Է���
	if (IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//�����������execute()����,���ɹ�ִ���򷵻سɹ���
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

//���þ����㷨��execute(void* eventPtr)����
ALPReturnCode ALPAlgBase::ExecuteRelatedAlgTool(void* eventPtr) {
	//����㷨�����ã���ֱ�Ӻ��Է���
	if (IsDisabled()) return ALPReturnCode::SUCCESS;

	try
	{
		//�����������execute()����,���ɹ�ִ���򷵻سɹ���
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

//���������༰�������������㷨�����ߵ�stop()����
ALPReturnCode ALPAlgBase::StopRelatedAlgTool() { 
	//�㷨�Ѿ����н������ڽ���״̬����ֱ�ӷ���
	if (IsInitialized() || IsDisabled()) 
		return ALPReturnCode::SUCCESS;

	try
	{
		//�����������stop()����,���ɹ�ִ��������㷨״̬
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

//���������༰�������������㷨�����ߵ�finalize()����
ALPReturnCode ALPAlgBase::FinalizeRelatedAlgTool() { 
	//�㷨�Ѿ�finalize���ڽ���״̬����ֱ�ӷ���
	if (IsFinalized() || IsDisabled())
		return ALPReturnCode::SUCCESS;

	try
	{
		//�����������finalize()����,���ɹ�ִ��������㷨״̬
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
	//����Ĺ��ܿ��ܴ��ڽϴ����⣬��ʱ����ʹ��
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




