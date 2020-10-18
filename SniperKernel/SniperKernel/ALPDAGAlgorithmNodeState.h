#ifndef SNIPER_ALP_DAG_ALG_NODES_STATE_H
#define SNIPER_ALP_DAG_ALG_NODES_STATE_H

#include <vector>
#include <list>
#include <ostream>
#include <algorithm>
#include "SniperKernel/SniperLog.h"
#include "SniperKernel/ALPReturnCode.h"
 
//单个算法结点的状态,仅供DAGTunnel调用
class ALPDAGAlgorithmNodeState final {
public:
	enum AlgState : unsigned char {
		INITIAL = 0,
		//CONTROLREADY = 1,
		DATAREADY = 2,
		RUNNING = 3,
		EVTACCEPTED = 4,
		EVTREJECTED = 5,
		ERROR = 6
	};

	ALPDAGAlgorithmNodeState() : m_AlgorithmState(INITIAL) {};
	~ALPDAGAlgorithmNodeState(){}
	//设置算法状态
	ALPReturnCode SetAlgNodeState(AlgState newState);
        //reset algorithm node status
        void ResetAlgNodeState();

	//取值
	AlgState GetAlgNodeState() const {
		return m_AlgorithmState;
	}
	//重置所有算法状态
	void reset() {
		m_AlgorithmState = INITIAL;
	}
	//算法是否处于特定状态
	bool IsEqualTo(AlgState state) const {
		return (state == m_AlgorithmState);
	}

private:
	AlgState m_AlgorithmState;     //DAG中单个算法结点的状态

};

inline std::ostream& operator<<(std::ostream& os, ALPDAGAlgorithmNodeState::AlgState temp) {
	switch (temp) {
	case ALPDAGAlgorithmNodeState::AlgState::INITIAL: os << "INITIAL";  break;
		//case ALPDAGAlgorithmNodeState::AlgState::CONTROLREADY: os << "CONTROLREADY";  break;
	case ALPDAGAlgorithmNodeState::AlgState::DATAREADY:  os << "DATAREADY";  break;
	case ALPDAGAlgorithmNodeState::AlgState::RUNNING:  os << "RUNNING";  break;
	case ALPDAGAlgorithmNodeState::AlgState::EVTACCEPTED: os << "EVTACCEPTED";  break;
	case ALPDAGAlgorithmNodeState::AlgState::EVTREJECTED:  os << "EVTREJECTED";  break;
	case ALPDAGAlgorithmNodeState::AlgState::ERROR:  os << "ERROR";  break;
	default:  os << "UNKNOWN";
	}
	return os;
}



//一个事例内所有算法结点的状态追踪,仅供DAGTunnel调用
class ALPDAGAlgNodesState final {
public:
	enum AlgState : unsigned char {
		INITIAL = 0,
		//CONTROLREADY = 1,
		DATAREADY = 2,
		RUNNING = 3,
		EVTACCEPTED = 4,
		EVTREJECTED = 5,
		ERROR = 6
	};

	ALPDAGAlgNodesState(unsigned long algscount) : m_AlgsStates(algscount, INITIAL) {};
	//设置算法状态
	ALPReturnCode SetAlgState(unsigned long tmpAlgIndex, AlgState newState);
	//重置所有算法状态
	void reset() { std::fill(m_AlgsStates.begin(), m_AlgsStates.end(), INITIAL); }
	//事例内是否有特定状态的算法
	bool contains(AlgState state) const {
		return std::find(m_AlgsStates.begin(), m_AlgsStates.end(), state) != m_AlgsStates.end();
	}
	//是否至少包含特定列表中的一种状态
	bool containsAny(std::list<AlgState> tmplist) const {
		return std::find_first_of(m_AlgsStates.begin(), m_AlgsStates.end(), tmplist.begin(), tmplist.end()) != m_AlgsStates.end();
                
	}
	//是否只包含特定列表中的状态
	bool containsOnly(std::list<AlgState> tmplist) const {
		return std::all_of(m_AlgsStates.begin(), m_AlgsStates.end(),
			[tmplist](AlgState state) {return std::find(tmplist.begin(), tmplist.end(), state) != tmplist.end(); });
	}

	//[]操作符取值
	const AlgState& operator[](unsigned int tmpIndex) const {
		return m_AlgsStates[tmpIndex];
	}
	//返回元素个数
	unsigned long size() const { return m_AlgsStates.size(); }
	//返回处于特定状态的元素个数
	unsigned long StateSubSetSize(AlgState state) const {
		return std::count_if(m_AlgsStates.begin(), m_AlgsStates.end(), [&](AlgState temp) {return state == temp; });
	}

private:
	std::vector<AlgState> m_AlgsStates;     //单个事例内所有DAG算法结点的状态

};

inline std::ostream& operator<<(std::ostream& os, ALPDAGAlgNodesState::AlgState temp) {
	switch (temp) {
	case ALPDAGAlgNodesState::AlgState::INITIAL: os << "INITIAL";  break;
	//case ALPDAGAlgNodesState::AlgState::CONTROLREADY: os << "CONTROLREADY";  break;
	case ALPDAGAlgNodesState::AlgState::DATAREADY:  os << "DATAREADY";  break;
	case ALPDAGAlgNodesState::AlgState::RUNNING:  os << "RUNNING";  break;
	case ALPDAGAlgNodesState::AlgState::EVTACCEPTED: os << "EVTACCEPTED";  break;
	case ALPDAGAlgNodesState::AlgState::EVTREJECTED:  os << "EVTREJECTED";  break;
	case ALPDAGAlgNodesState::AlgState::ERROR:  os << "ERROR";  break;
	default:  os << "UNKNOWN";
	}
	return os;
}





#endif
