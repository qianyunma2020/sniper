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


//算法池管理算法的属性存储
class DAGAlgorithmDataDependency
{
public:
	DAGAlgorithmDataDependency() {}
	~DAGAlgorithmDataDependency() {}

private:
	friend class ALPDAG;                //将DAG类声明为友元类
	std::string m_AlgName;                     //算法名称
	std::vector<std::string> m_InputDataSet;   //算法依赖的输入数据集
	std::vector<std::string> m_OutputDataSet;  //算法产生的输出数据集
};



//算法节点
class AlgNode 
{
public:
		AlgNode(const std::string& name) {
			m_AlgNodeName = name;
		}
		~AlgNode() {}
		//获取属性值
		const std::string& GetName() const { return m_AlgNodeName; }
		
protected:
		std::string m_AlgNodeName;                           //算法节点名称,也就是算法的名称
};


//叶子算法节点
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

	//获取算法的依赖数据节点列表
	const std::vector<DataNode*>& GetInputDataNodes() const { return m_AlgInputData; }
	//获取算法的输出数据节点列表
	const std::vector<DataNode*>& GetOutputDataNodes() const { return m_AlgOutputData; }
	
	//输出所有信息
	void PrintState() {}

private:
	//返回算法的入度，即依赖多少个输入数据
	unsigned int GetInDegree() const { return m_AlgInputData.size(); }
	//返回算法的出度，即输出多少个数据
	unsigned int GetOutDegree() const { return m_AlgOutputData.size(); }

	//添加依赖的输入数据节点
	void addInputDataNode(DataNode* node) {
		m_AlgInputData.push_back(node);
	}
	
	//移除算法依赖的输入数据结点
	void removeInputDataNode(DataNode* node) {
		for (std::vector<DataNode*>::iterator tmpIterator = m_AlgInputData.begin(); tmpIterator != m_AlgInputData.end(); tmpIterator++) {
			if (*tmpIterator == node) {
				m_AlgInputData.erase(tmpIterator);
				break;
			}
		}
	}

	//添加依赖的输出数据节点
	void addOuputDataNode(DataNode* node) {
		m_AlgOutputData.push_back(node);
	}

private:
	friend class ALPDAG;
	std::vector<DataNode*> m_AlgInputData;    //算法依赖的输入数据结点
	std::vector<DataNode*> m_AlgOutputData;   //算法的输出数据结点
};



//数据节点
class DataNode {
public:
	DataNode(const std::string& name) : m_DataNodeName(name) {}
	~DataNode() {
		m_DataProducers.clear();
		m_DataConsumers.clear();
	}
	const std::string& GetName() { return m_DataNodeName; }

	//获取生产者列表
	const std::vector<LeafAlgNode*>& GetProducers() const { return m_DataProducers; }
	//获取消费者列表
	const std::vector<LeafAlgNode*>& GetConsumers() const { return m_DataConsumers; }
	

private:
	//获取数据结点的出度
	unsigned int GetOutDegree() const { return m_DataConsumers.size(); }
	//获取数据结点的入度
	unsigned int GetInDegree() const { return m_DataProducers.size(); }

	//为数据节点添加生产者
	void addProducerNode(LeafAlgNode* algnode) {
		if (std::find(m_DataProducers.begin(), m_DataProducers.end(), algnode) == m_DataProducers.end())
			m_DataProducers.push_back(algnode);
	}
	//为数据节点移除生产者
	void removeProducerNode(LeafAlgNode* algnode) {
		for (std::vector<LeafAlgNode*>::iterator tmpIterator = m_DataProducers.begin(); tmpIterator != m_DataProducers.end(); tmpIterator++) {
			if (*tmpIterator == algnode) {
				m_DataProducers.erase(tmpIterator);
				break;
			}
		}
	}
	//为数据节点添加消费者
	void addConsumerNode(LeafAlgNode* algnode) {
		if (std::find(m_DataConsumers.begin(), m_DataConsumers.end(), algnode) == m_DataConsumers.end())
			m_DataConsumers.push_back(algnode);
	}


private:
	friend class ALPDAG;
	std::string m_DataNodeName;                  //数据节点名称
	std::vector<LeafAlgNode*> m_DataProducers;   //生产者列表
	std::vector<LeafAlgNode*> m_DataConsumers;   //消费者列表
};



//DAG算法通道，用来实现算法并行
class ALPDAG
{
public:
	ALPDAG();
	~ALPDAG();
	
	//通过名称获取数据结点
	static DataNode* GetDataNode(const std::string& dataname) {
		return m_DataNameToDataNodeMap.at(dataname);
	}
	//通过名称获取算法结点
	static LeafAlgNode* GetLeafAlgNode(const std::string& algname) {
		return m_LeafAlgNameToLeafAlgNodeMap.at(algname);
	}
	//返回DAG中叶子算法结点名称数组
	static std::vector<std::string>& GetLeafAlgNodesNames() { 
		return m_LeafAlgNodesNamesVector; 
	}
	//返回DAG中数据结点名称数组
	static std::vector<std::string>& GetDataNodesNames() {
		return m_DataNodesNamesVector; 
	}

	//获取DAG中的结点数目
	static unsigned long GetNodesCount() { 
		return m_LeafAlgNodesNamesVector.size() + m_DataNodesNamesVector.size();
	}
	//获取DAG中数据结点数目
	static unsigned long GetDataNodesCount() {
		return m_DataNodesNamesVector.size();
	}
	//获取DAG中算法结点数目
	static unsigned long GetLeafAlgNodesCount() { 
		return m_LeafAlgNodesNamesVector.size();
	}
	
	//创建有向无环图
	static ALPReturnCode BuildDAG(std::string tmpFileName);

	//输出所有信息
	static void PrintDAGInfo();

private:
	//读取JSON文件，存储算法池所管理算法的属性
	static ALPReturnCode ParseJsonFile(std::string tmpFileName);
	//初始化算法间的数据依赖关系
	static ALPReturnCode InitDataDependencyRelation();
	
private:
	//用于暂时存储算法资源池管理的算法属性
	static std::vector<DAGAlgorithmDataDependency*> m_DAGAlgDataDependencyVector;  

	//算法名称到算法结点的映射
	static std::unordered_map<std::string, LeafAlgNode*> m_LeafAlgNameToLeafAlgNodeMap;
	//数据名称到数据结点的映射
	static std::unordered_map<std::string, DataNode*> m_DataNameToDataNodeMap;

	//叶子算法结点名称集合
	static std::vector<std::string> m_LeafAlgNodesNamesVector;
	//数据结点名称集合
	static std::vector<std::string> m_DataNodesNamesVector;


/******************************************以下方法与数据用于拓扑排序************************************************/
private:
	//建立算法间的数据依赖关系，用于拓扑排序
	static ALPReturnCode InitTopologicalSortAlgDataRelation();
	//拓扑排序，如果无环返回true,否则为false
	static bool TopologicalSort();

private:
	//算法名称到算法结点的映射
	static std::unordered_map<std::string, LeafAlgNode*> m_TopoSort_LeafAlgNameToLeafAlgNodeMap;
	//数据名称到数据结点的映射
	static std::unordered_map<std::string, DataNode*> m_TopoSort_DataNameToDataNodeMap;
	//算法结点栈，用于拓扑排序
	static std::vector<std::string> m_TopoSort_AlgNodesStack;
	//数据结点栈，用于拓扑排序
	static std::vector<std::string> m_TopoSort_DataNodesStack;

};


#endif
