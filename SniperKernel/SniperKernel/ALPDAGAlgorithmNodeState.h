#ifndef SNIPER_ALP_DAG_ALG_NODES_STATE_H
#define SNIPER_ALP_DAG_ALG_NODES_STATE_H

#include <vector>
#include <list>
#include <ostream>
#include <algorithm>
#include "SniperKernel/SniperLog.h"
#include "SniperKernel/ALPReturnCode.h"
 
//�����㷨����״̬,����DAGTunnel����
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
	//�����㷨״̬
	ALPReturnCode SetAlgNodeState(AlgState newState);
        //reset algorithm node status
        void ResetAlgNodeState();

	//ȡֵ
	AlgState GetAlgNodeState() const {
		return m_AlgorithmState;
	}
	//���������㷨״̬
	void reset() {
		m_AlgorithmState = INITIAL;
	}
	//�㷨�Ƿ����ض�״̬
	bool IsEqualTo(AlgState state) const {
		return (state == m_AlgorithmState);
	}

private:
	AlgState m_AlgorithmState;     //DAG�е����㷨����״̬

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



//һ�������������㷨����״̬׷��,����DAGTunnel����
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
	//�����㷨״̬
	ALPReturnCode SetAlgState(unsigned long tmpAlgIndex, AlgState newState);
	//���������㷨״̬
	void reset() { std::fill(m_AlgsStates.begin(), m_AlgsStates.end(), INITIAL); }
	//�������Ƿ����ض�״̬���㷨
	bool contains(AlgState state) const {
		return std::find(m_AlgsStates.begin(), m_AlgsStates.end(), state) != m_AlgsStates.end();
	}
	//�Ƿ����ٰ����ض��б��е�һ��״̬
	bool containsAny(std::list<AlgState> tmplist) const {
		return std::find_first_of(m_AlgsStates.begin(), m_AlgsStates.end(), tmplist.begin(), tmplist.end()) != m_AlgsStates.end();
                
	}
	//�Ƿ�ֻ�����ض��б��е�״̬
	bool containsOnly(std::list<AlgState> tmplist) const {
		return std::all_of(m_AlgsStates.begin(), m_AlgsStates.end(),
			[tmplist](AlgState state) {return std::find(tmplist.begin(), tmplist.end(), state) != tmplist.end(); });
	}

	//[]������ȡֵ
	const AlgState& operator[](unsigned int tmpIndex) const {
		return m_AlgsStates[tmpIndex];
	}
	//����Ԫ�ظ���
	unsigned long size() const { return m_AlgsStates.size(); }
	//���ش����ض�״̬��Ԫ�ظ���
	unsigned long StateSubSetSize(AlgState state) const {
		return std::count_if(m_AlgsStates.begin(), m_AlgsStates.end(), [&](AlgState temp) {return state == temp; });
	}

private:
	std::vector<AlgState> m_AlgsStates;     //��������������DAG�㷨����״̬

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
