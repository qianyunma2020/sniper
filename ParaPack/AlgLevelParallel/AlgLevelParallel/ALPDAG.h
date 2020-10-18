#ifndef SNIPER_ALP_DAG_H
#define SNIPER_ALP_DAG_H

#include <algorithm>
#include <atomic>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "SniperKernel/ALPAlgBase.h"
#include "SniperKernel/ALPReturnCode.h"

class ALPDAG;


//�㷨�ع����㷨�����Դ洢
class DAGAlgorithmDataDependency
{
public:
	DAGAlgorithmDataDependency() {}
	~DAGAlgorithmDataDependency() {}

private:
	friend class ALPDAG;                //��DAG������Ϊ��Ԫ��
	std::string m_AlgName;                     //�㷨����
	std::vector<std::string> m_InputDataSet;   //�㷨�������������ݼ�
	std::vector<std::string> m_OutputDataSet;  //�㷨������������ݼ�
};



//�㷨�ڵ�
class AlgNode 
{
public:
		AlgNode(const std::string& name) {
			m_AlgNodeName = name;
		}
		~AlgNode() {}
		//��ȡ����ֵ
		const std::string& GetName() const { return m_AlgNodeName; }
		
protected:
		std::string m_AlgNodeName;                           //�㷨�ڵ�����,Ҳ�����㷨������
};


//Ҷ���㷨�ڵ�
class DataNode;
class LeafAlgNode :public AlgNode {
public:
	LeafAlgNode(const std::string& name) : AlgNode(name)
        {

	}

	~LeafAlgNode() {
		m_AlgInputData.clear();
		m_AlgOutputData.clear();
	}

	//��ȡ�㷨���������ݽڵ��б�
	const std::vector<DataNode*>& GetInputDataNodes() const { return m_AlgInputData; }
	//��ȡ�㷨��������ݽڵ��б�
	const std::vector<DataNode*>& GetOutputDataNodes() const { return m_AlgOutputData; }
	
	//���������Ϣ
	void PrintState() {}

private:
	//�����㷨����ȣ����������ٸ���������
	unsigned int GetInDegree() const { return m_AlgInputData.size(); }
	//�����㷨�ĳ��ȣ���������ٸ�����
	unsigned int GetOutDegree() const { return m_AlgOutputData.size(); }

	//����������������ݽڵ�
	void addInputDataNode(DataNode* node) {
		m_AlgInputData.push_back(node);
	}
	
	//�Ƴ��㷨�������������ݽ��
	void removeInputDataNode(DataNode* node) {
		for (std::vector<DataNode*>::iterator tmpIterator = m_AlgInputData.begin(); tmpIterator != m_AlgInputData.end(); tmpIterator++) {
			if (*tmpIterator == node) {
				m_AlgInputData.erase(tmpIterator);
				break;
			}
		}
	}

	//���������������ݽڵ�
	void addOuputDataNode(DataNode* node) {
		m_AlgOutputData.push_back(node);
	}

private:
	friend class ALPDAG;
	std::vector<DataNode*> m_AlgInputData;    //�㷨�������������ݽ��
	std::vector<DataNode*> m_AlgOutputData;   //�㷨��������ݽ��
};



//���ݽڵ�
class DataNode {
public:
	DataNode(const std::string& name) : m_DataNodeName(name) {}
	~DataNode() {
		m_DataProducers.clear();
		m_DataConsumers.clear();
	}
	const std::string& GetName() { return m_DataNodeName; }

	//��ȡ�������б�
	const std::vector<LeafAlgNode*>& GetProducers() const { return m_DataProducers; }
	//��ȡ�������б�
	const std::vector<LeafAlgNode*>& GetConsumers() const { return m_DataConsumers; }
	

private:
	//��ȡ���ݽ��ĳ���
	unsigned int GetOutDegree() const { return m_DataConsumers.size(); }
	//��ȡ���ݽ������
	unsigned int GetInDegree() const { return m_DataProducers.size(); }

	//Ϊ���ݽڵ����������
	void addProducerNode(LeafAlgNode* algnode) {
		if (std::find(m_DataProducers.begin(), m_DataProducers.end(), algnode) == m_DataProducers.end())
			m_DataProducers.push_back(algnode);
	}
	//Ϊ���ݽڵ��Ƴ�������
	void removeProducerNode(LeafAlgNode* algnode) {
		for (std::vector<LeafAlgNode*>::iterator tmpIterator = m_DataProducers.begin(); tmpIterator != m_DataProducers.end(); tmpIterator++) {
			if (*tmpIterator == algnode) {
				m_DataProducers.erase(tmpIterator);
				break;
			}
		}
	}
	//Ϊ���ݽڵ����������
	void addConsumerNode(LeafAlgNode* algnode) {
		if (std::find(m_DataConsumers.begin(), m_DataConsumers.end(), algnode) == m_DataConsumers.end())
			m_DataConsumers.push_back(algnode);
	}


private:
	friend class ALPDAG;
	std::string m_DataNodeName;                  //���ݽڵ�����
	std::vector<LeafAlgNode*> m_DataProducers;   //�������б�
	std::vector<LeafAlgNode*> m_DataConsumers;   //�������б�
};



//DAG�㷨ͨ��������ʵ���㷨����
class ALPDAG
{
public:
	ALPDAG();
	~ALPDAG();
	
	//ͨ�����ƻ�ȡ���ݽ��
	static DataNode* GetDataNode(const std::string& dataname) {
		return m_DataNameToDataNodeMap.at(dataname);
	}
	//ͨ�����ƻ�ȡ�㷨���
	static LeafAlgNode* GetLeafAlgNode(const std::string& algname) {
		return m_LeafAlgNameToLeafAlgNodeMap.at(algname);
	}
	//����DAG��Ҷ���㷨�����������
	static std::vector<std::string>& GetLeafAlgNodesNames() { 
		return m_LeafAlgNodesNamesVector; 
	}
	//����DAG�����ݽ����������
	static std::vector<std::string>& GetDataNodesNames() {
		return m_DataNodesNamesVector; 
	}

	//��ȡDAG�еĽ����Ŀ
	static unsigned long GetNodesCount() { 
		return m_LeafAlgNodesNamesVector.size() + m_DataNodesNamesVector.size();
	}
	//��ȡDAG�����ݽ����Ŀ
	static unsigned long GetDataNodesCount() {
		return m_DataNodesNamesVector.size();
	}
	//��ȡDAG���㷨�����Ŀ
	static unsigned long GetLeafAlgNodesCount() { 
		return m_LeafAlgNodesNamesVector.size();
	}
	
	//���������޻�ͼ
	static ALPReturnCode BuildDAG(std::string tmpFileName);

	//���������Ϣ
	static void PrintDAGInfo();

private:
	//��ȡJSON�ļ����洢�㷨���������㷨������
	static ALPReturnCode ParseJsonFile(std::string tmpFileName);
	//��ʼ���㷨�������������ϵ
	static ALPReturnCode InitDataDependencyRelation();
	
private:
	//������ʱ�洢�㷨��Դ�ع�����㷨����
	static std::vector<DAGAlgorithmDataDependency*> m_DAGAlgDataDependencyVector;  

	//�㷨���Ƶ��㷨����ӳ��
	static std::unordered_map<std::string, LeafAlgNode*> m_LeafAlgNameToLeafAlgNodeMap;
	//�������Ƶ����ݽ���ӳ��
	static std::unordered_map<std::string, DataNode*> m_DataNameToDataNodeMap;

	//Ҷ���㷨������Ƽ���
	static std::vector<std::string> m_LeafAlgNodesNamesVector;
	//���ݽ�����Ƽ���
	static std::vector<std::string> m_DataNodesNamesVector;


/******************************************���·���������������������************************************************/
private:
	//�����㷨�������������ϵ��������������
	static ALPReturnCode InitTopologicalSortAlgDataRelation();
	//������������޻�����true,����Ϊfalse
	static bool TopologicalSort();

private:
	//�㷨���Ƶ��㷨����ӳ��
	static std::unordered_map<std::string, LeafAlgNode*> m_TopoSort_LeafAlgNameToLeafAlgNodeMap;
	//�������Ƶ����ݽ���ӳ��
	static std::unordered_map<std::string, DataNode*> m_TopoSort_DataNameToDataNodeMap;
	//�㷨���ջ��������������
	static std::vector<std::string> m_TopoSort_AlgNodesStack;
	//���ݽ��ջ��������������
	static std::vector<std::string> m_TopoSort_DataNodesStack;

};


#endif
