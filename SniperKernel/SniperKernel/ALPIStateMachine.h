#ifndef SNIPER_ALP_IState_H
#define SNIPER_ALP_IState_H

#include "SniperKernel/DLElement.h"
#include "SniperKernel/ALPReturnCode.h"

//sniper�㷨�����״̬����ע���DAG���㷨����״̬����

enum ALPStateValue { DISABLED, CONFIGURED, INITIALIZED, RUNNING, ERROR };
enum ALPStateConversion {
	CONFIGURE,   //DISABLED->CONFIGURED 
	INITIALIZE,  //CONFIGURED->INITIALIZED
	START,       //INITIALIZED->RUNNING
	STOP,        //RUNNING->INITIALIZED
	FINALIZE,    //INITIALIZED->CONFIGURED
	ERRING,       //  ->ERROR
	ABORT,       //  ->CONFIGURED
	DISABLE      //CONFIGURED->DISABLED
};

class ALPIStateMachine // : virtual public DLElement
{
public:
	//ALPIStateMachine(const std::string& name) : DLElement(name)
	ALPIStateMachine() 
	{ 
	     m_State = CONFIGURED;
	}

	~ALPIStateMachine() { m_State = CONFIGURED; }
	//��ȡ��ǰ״̬
	ALPStateValue GetState() const { return m_State; }
	//�ж��Ƿ��Ѿ���ʼ��
	bool IsInitialized() { 
		return (INITIALIZED == m_State);
	}
	//�ж��Ƿ��Ѿ����ջ�
	bool IsFinalized() {
		return (CONFIGURED == m_State);
	}
	//�ж��㷨�Ƿ��ڽ���״̬
	bool IsDisabled() {
		return (DISABLED == m_State);
	}
	//�ж��㷨�Ƿ�������״̬
	bool IsRunning() {
		return (RUNNING == m_State);
	}

	//CONFIGURE����
	ALPReturnCode ConfigureState() {
		if (ERROR == TransitionState(CONFIGURE))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//INITIALIZE����
	ALPReturnCode InitializeState() {
		if (ERROR == TransitionState(INITIALIZE))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//START����
	ALPReturnCode StartState() {
		if (ERROR == TransitionState(START))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//STOP����
	ALPReturnCode StopState() {
		if (ERROR == TransitionState(STOP))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//FINALIZE����
	ALPReturnCode FinalizeState() {
		if (ERROR == TransitionState(FINALIZE))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//ERROR����
	ALPReturnCode ErrorState() {
		TransitionState(ERRING);
		return ALPReturnCode::SUCCESS;
	}
	//ABORT����
	ALPReturnCode AbortState() {
		TransitionState(ABORT); 
		return ALPReturnCode::SUCCESS;
	}
	//���³�ʼ������
	ALPReturnCode ReInitializeState() {
		if ((ERROR == TransitionState(ABORT)) || (ERROR == TransitionState(INITIALIZE)))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//��������
	ALPReturnCode ReStartState() {
		if ((ERROR == TransitionState(ABORT)) || (ERROR == TransitionState(INITIALIZE)) || (ERROR == TransitionState(START)))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}

       // bool initialize() { return true; }
       // bool finalize() { return true; }
       // void show(int indent) {}


protected:
	//��״ֵ̬���б��
	ALPStateValue TransitionState(const ALPStateConversion tmpConversion)
	{
		switch (tmpConversion)
		{
		case CONFIGURE:
			if (DISABLED == m_State) 
				m_State = CONFIGURED;
			else
				m_State = ERROR;
			break;
		case INITIALIZE:
			if (CONFIGURED == m_State) 
				m_State = INITIALIZED;
			else
				m_State = ERROR;
			break;
		case START:
			if (INITIALIZED == m_State) 
				m_State = RUNNING;
			else
				m_State = ERROR;
			break;
		case STOP:
			if (RUNNING == m_State) 
				m_State = INITIALIZED;
			else
				m_State = ERROR;
			break;
		case FINALIZE:
			if (INITIALIZED == m_State) 
				m_State = CONFIGURED;
			else
				m_State = ERROR;
			break;
		case DISABLE:
			if (CONFIGURED == m_State) 
				m_State = DISABLED;
			else
				m_State = ERROR;
			break;
		case ABORT:
			m_State = CONFIGURED;     
			break;
		default:
			m_State = ERROR;    
			break;
		}
		return m_State;
	}
	
private:
	ALPStateValue m_State;   //״ֵ̬
};


std::ostream& operator<<(std::ostream& os, const ALPIStateMachine& state);


#endif
