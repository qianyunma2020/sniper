#ifndef SNIPER_ALP_IState_H
#define SNIPER_ALP_IState_H

#include "SniperKernel/DLElement.h"
#include "SniperKernel/ALPReturnCode.h"

//sniper算法本身的状态机，注意和DAG中算法结点的状态区分

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
	//获取当前状态
	ALPStateValue GetState() const { return m_State; }
	//判断是否已经初始化
	bool IsInitialized() { 
		return (INITIALIZED == m_State);
	}
	//判断是否已经最终化
	bool IsFinalized() {
		return (CONFIGURED == m_State);
	}
	//判断算法是否处于禁用状态
	bool IsDisabled() {
		return (DISABLED == m_State);
	}
	//判断算法是否处于运行状态
	bool IsRunning() {
		return (RUNNING == m_State);
	}

	//CONFIGURE操作
	ALPReturnCode ConfigureState() {
		if (ERROR == TransitionState(CONFIGURE))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//INITIALIZE操作
	ALPReturnCode InitializeState() {
		if (ERROR == TransitionState(INITIALIZE))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//START操作
	ALPReturnCode StartState() {
		if (ERROR == TransitionState(START))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//STOP操作
	ALPReturnCode StopState() {
		if (ERROR == TransitionState(STOP))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//FINALIZE操作
	ALPReturnCode FinalizeState() {
		if (ERROR == TransitionState(FINALIZE))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//ERROR操作
	ALPReturnCode ErrorState() {
		TransitionState(ERRING);
		return ALPReturnCode::SUCCESS;
	}
	//ABORT操作
	ALPReturnCode AbortState() {
		TransitionState(ABORT); 
		return ALPReturnCode::SUCCESS;
	}
	//重新初始化操作
	ALPReturnCode ReInitializeState() {
		if ((ERROR == TransitionState(ABORT)) || (ERROR == TransitionState(INITIALIZE)))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}
	//重启操作
	ALPReturnCode ReStartState() {
		if ((ERROR == TransitionState(ABORT)) || (ERROR == TransitionState(INITIALIZE)) || (ERROR == TransitionState(START)))
			return ALPReturnCode::FAILURE;
		return ALPReturnCode::SUCCESS;
	}

       // bool initialize() { return true; }
       // bool finalize() { return true; }
       // void show(int indent) {}


protected:
	//对状态值进行变更
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
	ALPStateValue m_State;   //状态值
};


std::ostream& operator<<(std::ostream& os, const ALPIStateMachine& state);


#endif
