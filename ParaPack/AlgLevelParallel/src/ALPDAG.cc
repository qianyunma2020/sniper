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

//��ȡJSON�ļ����洢�㷨���������㷨������
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
			//�����ļ�����ȡ��Ϣ
			DAGAlgorithmDataDependency* tmpAlgDDepPty = new DAGAlgorithmDataDependency();
			tmpAlgDDepPty->m_AlgName = root[count]["AlgName"].asString();
			//��������������
			for (unsigned int inputListCount = 0; inputListCount < root[count]["InputList"].size(); inputListCount++) {
				tmpAlgDDepPty->m_InputDataSet.push_back(root[count]["InputList"][inputListCount].asString());
			}
			//�������
			for (unsigned int outputListCount = 0; outputListCount < root[count]["OutputList"].size(); outputListCount++) {
				tmpAlgDDepPty->m_OutputDataSet.push_back(root[count]["OutputList"][outputListCount].asString());
			}

			//������Ϣ
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

//��ʼ���㷨�������������ϵ
ALPReturnCode ALPDAG::InitDataDependencyRelation() {
	//���α���ÿ���㷨��Ϣ
	for (auto tmpAlgNodeInfoPtr : m_DAGAlgDataDependencyVector) {

		//�����㷨��㣬����ӳ���ϵ
		LeafAlgNode* tmpLeafAlgNode = new LeafAlgNode(tmpAlgNodeInfoPtr->m_AlgName);
		m_LeafAlgNameToLeafAlgNodeMap.emplace(tmpAlgNodeInfoPtr->m_AlgName, tmpLeafAlgNode);
		//����Ҷ���㷨������Ƽ���
		m_LeafAlgNodesNamesVector.push_back(tmpAlgNodeInfoPtr->m_AlgName);

		//�����㷨��������ݼ�
		for (auto outdata : tmpAlgNodeInfoPtr->m_OutputDataSet) {
			//�������ݽ�㣬����ӳ���ϵ
			m_DataNameToDataNodeMap.emplace(outdata, (new DataNode(outdata)));
			DataNode* tmpDataNode = m_DataNameToDataNodeMap[outdata];
			//�����㷨�������ݽ��������߹�ϵ
			tmpDataNode->addProducerNode(tmpLeafAlgNode);
			tmpLeafAlgNode->addOuputDataNode(tmpDataNode);
			//������ݽ�㼯�����в����ڸý�㣬��������ݽ�����Ƽ���
			if (m_DataNodesNamesVector.end() == (std::find(m_DataNodesNamesVector.begin(), m_DataNodesNamesVector.end(), tmpDataNode->GetName()))) {
				m_DataNodesNamesVector.push_back(tmpDataNode->GetName());
			}
		}

		//�����㷨���������ݼ�
		for (auto indata : tmpAlgNodeInfoPtr->m_InputDataSet) {
			//�������ݽ�㣬����ӳ���ϵ
			m_DataNameToDataNodeMap.emplace(indata, (new DataNode(indata)));
			DataNode* tmpDataNode = m_DataNameToDataNodeMap[indata];
			//�����㷨�������ݽ��������߹�ϵ
			tmpDataNode->addConsumerNode(tmpLeafAlgNode);
			tmpLeafAlgNode->addInputDataNode(tmpDataNode);
			//������ݽ�㼯�����в����ڸý�㣬��������ݽ�����Ƽ���
			if (m_DataNodesNamesVector.end() == (std::find(m_DataNodesNamesVector.begin(), m_DataNodesNamesVector.end(), tmpDataNode->GetName()))) {
				m_DataNodesNamesVector.push_back(tmpDataNode->GetName());
			}
		}

	}
	return ALPReturnCode::SUCCESS;
}

//�����㷨�������������ϵ��������������
ALPReturnCode ALPDAG::InitTopologicalSortAlgDataRelation() {
	//���α���ÿ���㷨��Ϣ
	for (auto tmpAlgNodeInfoPtr : m_DAGAlgDataDependencyVector) {

		//�����㷨��㣬����ӳ���ϵ
		LeafAlgNode* tmpLeafAlgNode = new LeafAlgNode(tmpAlgNodeInfoPtr->m_AlgName);
		m_TopoSort_LeafAlgNameToLeafAlgNodeMap.emplace(tmpAlgNodeInfoPtr->m_AlgName, tmpLeafAlgNode);
		

		//�����㷨��������ݼ�
		for (auto outdata : tmpAlgNodeInfoPtr->m_OutputDataSet) {
			//�������ݽ�㣬����ӳ���ϵ
			m_TopoSort_DataNameToDataNodeMap.emplace(outdata, (new DataNode(outdata)));
			DataNode* tmpDataNode = m_TopoSort_DataNameToDataNodeMap[outdata];
			//�����㷨�������ݽ��������߹�ϵ
			tmpDataNode->addProducerNode(tmpLeafAlgNode);
			tmpLeafAlgNode->addOuputDataNode(tmpDataNode);
		}

		//�����㷨���������ݼ�
		for (auto indata : tmpAlgNodeInfoPtr->m_InputDataSet) {
			//�������ݽ�㣬����ӳ���ϵ
			m_TopoSort_DataNameToDataNodeMap.emplace(indata, (new DataNode(indata)));
			DataNode* tmpDataNode = m_TopoSort_DataNameToDataNodeMap[indata];
			//�����㷨�������ݽ��������߹�ϵ
			tmpDataNode->addConsumerNode(tmpLeafAlgNode);
			tmpLeafAlgNode->addInputDataNode(tmpDataNode);
		}

	}
	return ALPReturnCode::SUCCESS;
}

//������������޻�����true,����Ϊfalse
bool ALPDAG::TopologicalSort() {
	//���Ϊ����㷨�����ջ
	for (auto tmpAlgNode : m_TopoSort_LeafAlgNameToLeafAlgNodeMap) {
		if (0 == tmpAlgNode.second->GetInDegree()) {
			m_TopoSort_AlgNodesStack.push_back(tmpAlgNode.first);
		}
	}
	//���Ϊ������ݽ����ջ
	for (auto tmpDataNode : m_TopoSort_DataNameToDataNodeMap) {
		if (0 == tmpDataNode.second->GetInDegree()) {
			m_TopoSort_DataNodesStack.push_back(tmpDataNode.first);
		}
	}

	while ((m_TopoSort_AlgNodesStack.size()) || (m_TopoSort_DataNodesStack.size())) {
		//�㷨����ջ
		if (m_TopoSort_AlgNodesStack.size()) {
			std::string tmpAlgName = m_TopoSort_AlgNodesStack.back();
                        m_TopoSort_AlgNodesStack.pop_back();
			//�Ƴ�������ݶ�Ӧ���������߹�ϵ
			for (auto tmpDataNodePtr : m_TopoSort_LeafAlgNameToLeafAlgNodeMap[tmpAlgName]->GetOutputDataNodes()) {
				tmpDataNodePtr->removeProducerNode(m_TopoSort_LeafAlgNameToLeafAlgNodeMap[tmpAlgName]);
				//�����ݽ�����Ϊ������ջ
				if (0 == tmpDataNodePtr->GetInDegree()) {
					m_TopoSort_DataNodesStack.push_back(tmpDataNodePtr->GetName());
				}
			}
			//ɾ���㷨���Ԫ��
			m_TopoSort_LeafAlgNameToLeafAlgNodeMap.erase(tmpAlgName);
		}//if

		//���ݽ���ջ
		if (m_TopoSort_DataNodesStack.size()) {
			std::string tmpDataName = m_TopoSort_DataNodesStack.back();
                        m_TopoSort_DataNodesStack.pop_back();
			//�Ƴ����������ݵ��㷨���������߹�ϵ
			for (auto tmpAlgNodePtr : m_TopoSort_DataNameToDataNodeMap[tmpDataName]->GetConsumers()) {
				tmpAlgNodePtr->removeInputDataNode(m_TopoSort_DataNameToDataNodeMap[tmpDataName]);
				//���㷨������Ϊ�㣬����ջ
				if (0 == tmpAlgNodePtr->GetInDegree()) {
					m_TopoSort_AlgNodesStack.push_back(tmpAlgNodePtr->GetName());
				}
			}
			//ɾ�����ݽ��Ԫ��
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

//���������޻�ͼ
ALPReturnCode ALPDAG::BuildDAG(std::string tmpFileName) {
	//����Json�ļ�
	ParseJsonFile(tmpFileName);

	//����������DAG���Ƿ���ѭ������,�����ֻ��򱨴���ֹϵͳ
	InitTopologicalSortAlgDataRelation();
	if (!TopologicalSort())
		return ALPReturnCode::FAILURE;

	//��ʼ���㷨�������������ϵ
	InitDataDependencyRelation();

	//Ϊ����û���������ݵ��㷨���������ʼ���ݵ���������
	const std::string bgevent = "EventBeginData";
	DataNode* EventBeginDataNode = new DataNode(bgevent);
	m_DataNameToDataNodeMap.emplace(bgevent, EventBeginDataNode);
	m_DataNodesNamesVector.push_back(bgevent);
	for (auto algnodePtr : m_LeafAlgNameToLeafAlgNodeMap) {
		//����㷨������Ϊ�㣬�����"EventBeginData"������㷨���������߹�ϵ
		if (0 == (algnodePtr.second->GetInDegree())) {
			EventBeginDataNode->addConsumerNode(algnodePtr.second);
			algnodePtr.second->addInputDataNode(EventBeginDataNode);
		}
	}


	const std::string endeventdata = "EventEndData";
	unsigned int ZeroOutDegreeAlgCount = 0;
	//����Ƿ����û��������ݵ��㷨
	for (auto algnodePtr : m_LeafAlgNameToLeafAlgNodeMap) {
		//����㷨������Ϊ�㣬������㷨�����"EventEndData"���������߹�ϵ
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
/*	//����Ĵ���û��������ݵ��㷨������DAG�����"EventEndData"���ݽ��
	if (0 != ZeroOutDegreeAlgCount) {
		m_DataNameToDataNodeMap.emplace(endeventdata, EventEndDataNode);
		m_DataNodesNamesVector.push_back(endeventdata);
	}  */
	

	//������������㷨
	const std::string endeventalg = "ALPDAGTunnelEventEndAlg";
	LeafAlgNode* EventEndLeafAlgNode = new LeafAlgNode(endeventalg);
	for (auto datanodePtr : m_DataNameToDataNodeMap) {
		//������ݽ��ĳ���Ϊ�㣬��������ݽ����"EventEndAlg"�㷨���������߹�ϵ
		if (0 == datanodePtr.second->GetOutDegree()) {
			EventEndLeafAlgNode->addInputDataNode(datanodePtr.second);
			datanodePtr.second->addConsumerNode(EventEndLeafAlgNode);
		}
	}
        m_LeafAlgNameToLeafAlgNodeMap.emplace(endeventalg, EventEndLeafAlgNode);
        m_LeafAlgNodesNamesVector.push_back(endeventalg);

	return ALPReturnCode::SUCCESS;
}

//���������Ϣ
void ALPDAG::PrintDAGInfo() {

}
