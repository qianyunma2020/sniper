#include <fstream>
#include <ios>
#include <iostream>
#include "AlgLevelParallel/ALPDAG.h"
#include <json/json.h>
//#include "jsoncpp/json/json.h"


std::vector<DAGAlgorithmDataDependency*> ALPDAG::m_DAGAlgDataDependencyVector;
std::unordered_map<std::string, LeafAlgNode*> ALPDAG::m_LeafAlgNameToLeafAlgNodeMap;
std::unordered_map<std::string, DataNode*> ALPDAG::m_DataNameToDataNodeMap;
std::vector<std::string> ALPDAG::m_LeafAlgNodesNamesVector;
std::vector<std::string> ALPDAG::m_DataNodesNamesVector;
std::unordered_map<std::string, LeafAlgNode*> ALPDAG::m_TopoSort_LeafAlgNameToLeafAlgNodeMap;
std::unordered_map<std::string, DataNode*> ALPDAG::m_TopoSort_DataNameToDataNodeMap;
std::vector<std::string> ALPDAG::m_TopoSort_AlgNodesStack;
std::vector<std::string> ALPDAG::m_TopoSort_DataNodesStack;


ALPDAG::ALPDAG()
{
}

ALPDAG::~ALPDAG()
{
}

//读取JSON文件，存储算法池所管理算法的属性
ALPReturnCode ALPDAG::ParseJsonFile(std::string tmpFileName) {
	Json::Reader fileReader;
	Json::Value  root;

	std::ifstream inputFile(tmpFileName, std::ios::binary);
	if (!inputFile.is_open()) {
		LogError << "Algs property Json file open error!" << std::endl;
		return ALPReturnCode::FAILURE;
	}

	if (fileReader.parse(inputFile, root)) {
		for (unsigned int count = 0; count < root.size(); count++) {
			//解析文件，读取信息
			DAGAlgorithmDataDependency* tmpAlgDDepPty = new DAGAlgorithmDataDependency();
			tmpAlgDDepPty->m_AlgName = root[count]["AlgName"].asString();
			//依赖的输入数据
			for (unsigned int inputListCount = 0; inputListCount < root[count]["InputList"].size(); inputListCount++) {
				tmpAlgDDepPty->m_InputDataSet.push_back(root[count]["InputList"][inputListCount].asString());
			}
			//输出数据
			for (unsigned int outputListCount = 0; outputListCount < root[count]["OutputList"].size(); outputListCount++) {
				tmpAlgDDepPty->m_OutputDataSet.push_back(root[count]["OutputList"][outputListCount].asString());
			}

			//保存信息
			std::vector<DAGAlgorithmDataDependency*>::iterator tmpIterator;
			for (tmpIterator = m_DAGAlgDataDependencyVector.begin(); tmpIterator != m_DAGAlgDataDependencyVector.end(); tmpIterator++) {
				if ((*tmpIterator)->m_AlgName == tmpAlgDDepPty->m_AlgName)
					break;
			}
			if (tmpIterator == m_DAGAlgDataDependencyVector.end())
				m_DAGAlgDataDependencyVector.push_back(tmpAlgDDepPty);
		}
	}
	inputFile.close();
	return ALPReturnCode::SUCCESS;
}

//初始化算法间的数据依赖关系
ALPReturnCode ALPDAG::InitDataDependencyRelation() {
	//依次遍历每个算法信息
	for (auto tmpAlgNodeInfoPtr : m_DAGAlgDataDependencyVector) {

		//创建算法结点，更新映射关系
		LeafAlgNode* tmpLeafAlgNode = new LeafAlgNode(tmpAlgNodeInfoPtr->m_AlgName);
		m_LeafAlgNameToLeafAlgNodeMap.emplace(tmpAlgNodeInfoPtr->m_AlgName, tmpLeafAlgNode);
		//更新叶子算法结点名称集合
		m_LeafAlgNodesNamesVector.push_back(tmpAlgNodeInfoPtr->m_AlgName);

		//遍历算法的输出数据集
		for (auto outdata : tmpAlgNodeInfoPtr->m_OutputDataSet) {
			//创建数据结点，更新映射关系
			m_DataNameToDataNodeMap.emplace(outdata, (new DataNode(outdata)));
			DataNode* tmpDataNode = m_DataNameToDataNodeMap[outdata];
			//更新算法结点和数据结点的生产者关系
			tmpDataNode->addProducerNode(tmpLeafAlgNode);
			tmpLeafAlgNode->addOuputDataNode(tmpDataNode);
			//如果数据结点集合中尚不存在该结点，则更新数据结点名称集合
			if (m_DataNodesNamesVector.end() == (std::find(m_DataNodesNamesVector.begin(), m_DataNodesNamesVector.end(), tmpDataNode->GetName()))) {
				m_DataNodesNamesVector.push_back(tmpDataNode->GetName());
			}
		}

		//遍历算法的输入数据集
		for (auto indata : tmpAlgNodeInfoPtr->m_InputDataSet) {
			//创建数据结点，更新映射关系
			m_DataNameToDataNodeMap.emplace(indata, (new DataNode(indata)));
			DataNode* tmpDataNode = m_DataNameToDataNodeMap[indata];
			//更新算法结点和数据结点的消费者关系
			tmpDataNode->addConsumerNode(tmpLeafAlgNode);
			tmpLeafAlgNode->addInputDataNode(tmpDataNode);
			//如果数据结点集合中尚不存在该结点，则更新数据结点名称集合
			if (m_DataNodesNamesVector.end() == (std::find(m_DataNodesNamesVector.begin(), m_DataNodesNamesVector.end(), tmpDataNode->GetName()))) {
				m_DataNodesNamesVector.push_back(tmpDataNode->GetName());
			}
		}

	}
	return ALPReturnCode::SUCCESS;
}

//建立算法间的数据依赖关系，用于拓扑排序
ALPReturnCode ALPDAG::InitTopologicalSortAlgDataRelation() {
	//依次遍历每个算法信息
	for (auto tmpAlgNodeInfoPtr : m_DAGAlgDataDependencyVector) {

		//创建算法结点，更新映射关系
		LeafAlgNode* tmpLeafAlgNode = new LeafAlgNode(tmpAlgNodeInfoPtr->m_AlgName);
		m_TopoSort_LeafAlgNameToLeafAlgNodeMap.emplace(tmpAlgNodeInfoPtr->m_AlgName, tmpLeafAlgNode);
		

		//遍历算法的输出数据集
		for (auto outdata : tmpAlgNodeInfoPtr->m_OutputDataSet) {
			//创建数据结点，更新映射关系
			m_TopoSort_DataNameToDataNodeMap.emplace(outdata, (new DataNode(outdata)));
			DataNode* tmpDataNode = m_TopoSort_DataNameToDataNodeMap[outdata];
			//更新算法结点和数据结点的生产者关系
			tmpDataNode->addProducerNode(tmpLeafAlgNode);
			tmpLeafAlgNode->addOuputDataNode(tmpDataNode);
		}

		//遍历算法的输入数据集
		for (auto indata : tmpAlgNodeInfoPtr->m_InputDataSet) {
			//创建数据结点，更新映射关系
			m_TopoSort_DataNameToDataNodeMap.emplace(indata, (new DataNode(indata)));
			DataNode* tmpDataNode = m_TopoSort_DataNameToDataNodeMap[indata];
			//更新算法结点和数据结点的消费者关系
			tmpDataNode->addConsumerNode(tmpLeafAlgNode);
			tmpLeafAlgNode->addInputDataNode(tmpDataNode);
		}

	}
	return ALPReturnCode::SUCCESS;
}

//拓扑排序，如果无环返回true,否则为false
bool ALPDAG::TopologicalSort() {
	//入度为零的算法结点入栈
	for (auto tmpAlgNode : m_TopoSort_LeafAlgNameToLeafAlgNodeMap) {
		if (0 == tmpAlgNode.second->GetInDegree()) {
			m_TopoSort_AlgNodesStack.push_back(tmpAlgNode.first);
		}
	}
	//入度为零的数据结点入栈
	for (auto tmpDataNode : m_TopoSort_DataNameToDataNodeMap) {
		if (0 == tmpDataNode.second->GetInDegree()) {
			m_TopoSort_DataNodesStack.push_back(tmpDataNode.first);
		}
	}

	while ((m_TopoSort_AlgNodesStack.size()) || (m_TopoSort_DataNodesStack.size())) {
		//算法结点出栈
		if (m_TopoSort_AlgNodesStack.size()) {
			std::string tmpAlgName = m_TopoSort_AlgNodesStack.back();
                        m_TopoSort_AlgNodesStack.pop_back();
			//移除输出数据对应结点的生产者关系
			for (auto tmpDataNodePtr : m_TopoSort_LeafAlgNameToLeafAlgNodeMap[tmpAlgName]->GetOutputDataNodes()) {
				tmpDataNodePtr->removeProducerNode(m_TopoSort_LeafAlgNameToLeafAlgNodeMap[tmpAlgName]);
				//若数据结点入度为零则入栈
				if (0 == tmpDataNodePtr->GetInDegree()) {
					m_TopoSort_DataNodesStack.push_back(tmpDataNodePtr->GetName());
				}
			}
			//删除算法结点元素
			m_TopoSort_LeafAlgNameToLeafAlgNodeMap.erase(tmpAlgName);
		}//if

		//数据结点出栈
		if (m_TopoSort_DataNodesStack.size()) {
			std::string tmpDataName = m_TopoSort_DataNodesStack.back();
                        m_TopoSort_DataNodesStack.pop_back();
			//移除依赖该数据的算法结点的消费者关系
			for (auto tmpAlgNodePtr : m_TopoSort_DataNameToDataNodeMap[tmpDataName]->GetConsumers()) {
				tmpAlgNodePtr->removeInputDataNode(m_TopoSort_DataNameToDataNodeMap[tmpDataName]);
				//若算法结点入度为零，则入栈
				if (0 == tmpAlgNodePtr->GetInDegree()) {
					m_TopoSort_AlgNodesStack.push_back(tmpAlgNodePtr->GetName());
				}
			}
			//删除数据结点元素
			m_TopoSort_DataNameToDataNodeMap.erase(tmpDataName);
		}//if
	}//while
	
	if (m_TopoSort_LeafAlgNameToLeafAlgNodeMap.size() || m_TopoSort_DataNameToDataNodeMap.size()) {
		LogError << "There is circle in DAG , system Error!" << std::endl;
		return false;
	}
	else
		return true;
}

//创建有向无环图
ALPReturnCode ALPDAG::BuildDAG(std::string tmpFileName) {
	//解析Json文件
	ParseJsonFile(tmpFileName);

	//拓扑排序检查DAG中是否有循环依赖,若发现环则报错终止系统
	InitTopologicalSortAlgDataRelation();
	if (!TopologicalSort())
		return ALPReturnCode::FAILURE;

	//初始化算法间的数据依赖关系
	InitDataDependencyRelation();

	//为所有没有输入数据的算法添加事例开始数据的数据依赖
	const std::string bgevent = "EventBeginData";
	DataNode* EventBeginDataNode = new DataNode(bgevent);
	m_DataNameToDataNodeMap.emplace(bgevent, EventBeginDataNode);
	m_DataNodesNamesVector.push_back(bgevent);
	for (auto algnodePtr : m_LeafAlgNameToLeafAlgNodeMap) {
		//如果算法结点入度为零，则更新"EventBeginData"结点与算法结点的消费者关系
		if (0 == (algnodePtr.second->GetInDegree())) {
			EventBeginDataNode->addConsumerNode(algnodePtr.second);
			algnodePtr.second->addInputDataNode(EventBeginDataNode);
		}
	}


	const std::string endeventdata = "EventEndData";
	unsigned int ZeroOutDegreeAlgCount = 0;
	//检查是否存在没有输出数据的算法
	for (auto algnodePtr : m_LeafAlgNameToLeafAlgNodeMap) {
		//如果算法结点出度为零，则更新算法结点与"EventEndData"结点的生产者关系
		if (0 == (algnodePtr.second->GetOutDegree())) {
                        //create sveral event end data node
                        std::string tmpDataNodeName = endeventdata + std::to_string(ZeroOutDegreeAlgCount);
                        DataNode* EventEndDataNode = new DataNode(tmpDataNodeName);                  
			EventEndDataNode->addProducerNode(algnodePtr.second);
			algnodePtr.second->addOuputDataNode(EventEndDataNode);
                        //insert new data node into DAG
                        m_DataNameToDataNodeMap.emplace(tmpDataNodeName, EventEndDataNode);
                        m_DataNodesNamesVector.push_back(tmpDataNodeName);
			ZeroOutDegreeAlgCount++;
                 
                        LogDebug<<"Class ALPDAG function BuildDAG implys data node "<<tmpDataNodeName<<" has been added!"<<std::endl;
		}
	}
/*	//若真的存在没有输出数据的算法，则在DAG中添加"EventEndData"数据结点
	if (0 != ZeroOutDegreeAlgCount) {
		m_DataNameToDataNodeMap.emplace(endeventdata, EventEndDataNode);
		m_DataNodesNamesVector.push_back(endeventdata);
	}  */
	

	//添加事例结束算法
	const std::string endeventalg = "ALPDAGTunnelEventEndAlg";
	LeafAlgNode* EventEndLeafAlgNode = new LeafAlgNode(endeventalg);
	for (auto datanodePtr : m_DataNameToDataNodeMap) {
		//如果数据结点的出度为零，则更新数据结点与"EventEndAlg"算法结点的消费者关系
		if (0 == datanodePtr.second->GetOutDegree()) {
			EventEndLeafAlgNode->addInputDataNode(datanodePtr.second);
			datanodePtr.second->addConsumerNode(EventEndLeafAlgNode);
		}
	}
        m_LeafAlgNameToLeafAlgNodeMap.emplace(endeventalg, EventEndLeafAlgNode);
        m_LeafAlgNodesNamesVector.push_back(endeventalg);

	return ALPReturnCode::SUCCESS;
}

//输出所有信息
void ALPDAG::PrintDAGInfo() {

}
