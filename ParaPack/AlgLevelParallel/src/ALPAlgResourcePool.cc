#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <json/json.h>
#include "unistd.h"
#include "SniperKernel/ALPAlgBase.h"
#include "SniperKernel/DLEFactory.h"
#include "AlgLevelParallel/ALPAlgResourcePool.h"


ALPAlgResourcePool::ALPAlgResourcePool(const std::string& name) : Task(name)
{

}
ALPAlgResourcePool::~ALPAlgResourcePool() {
	for (auto tmpAlgNameInstancePairPtr : m_AlgNameToAlgInstanceQueue) {
		auto* queue = tmpAlgNameInstancePairPtr.second;
		delete queue;
	}
	m_AlgNameToAlgInstanceQueue.clear();
        m_PoolAlgInstanceList.clear();
}

//读取JSON文件，存储算法池所管理算法的属性
ALPReturnCode ALPAlgResourcePool::ParseAlgsPropertyJsonFile() {
	Json::Reader fileReader;
	Json::Value  root;

	std::ifstream inputFile(m_AlgsPropertyFileName, std::ios::binary);
	if (!inputFile.is_open()) {
		LogError << "Algs property Json file: "<<m_AlgsPropertyFileName<<" opens error!" << std::endl;
		return ALPReturnCode::FAILURE;
	}

        //get total number of all cpu
        unsigned int totalCPUNum = sysconf(_SC_NPROCESSORS_CONF);

	if (fileReader.parse(inputFile,root)) {
		for (unsigned int count = 0; count < root.size(); count++) {
			//解析文件，读取信息
			PoolAlgorithmProperty* tmpAlgPty = new PoolAlgorithmProperty();
			tmpAlgPty->m_AlgName = root[count]["AlgName"].asString();
			//如果算法实例数目小于1，则校正为1
			tmpAlgPty->m_MaxInstanceCount = totalCPUNum;
			if (tmpAlgPty->m_MaxInstanceCount < 1) { 
				tmpAlgPty->m_MaxInstanceCount = 1; 
				LogInfo << "Algorithm " << tmpAlgPty->m_AlgName << " instance count is reset 1." << std::endl;
			}else if (tmpAlgPty->m_MaxInstanceCount > std::thread::hardware_concurrency()) {
				//如果用户配置的最大实例数目超过硬件所支持的最大线程数目，则重新设为最大线程数目
				tmpAlgPty->m_MaxInstanceCount = std::thread::hardware_concurrency();
				LogInfo << "Algorithm " << tmpAlgPty->m_AlgName << " instance is reset to " << tmpAlgPty->m_MaxInstanceCount << "." << std::endl;
			}


			//保存信息
			std::vector<PoolAlgorithmProperty*>::iterator tmpIterator;
			for (tmpIterator = m_PoolAlgsPropertySet.begin(); tmpIterator != m_PoolAlgsPropertySet.end(); tmpIterator++) {
				if ((*tmpIterator)->m_AlgName == tmpAlgPty->m_AlgName)
					break;
			}
			if(tmpIterator== m_PoolAlgsPropertySet.end())
				m_PoolAlgsPropertySet.push_back(tmpAlgPty);
		}
	}
	inputFile.close();
	return ALPReturnCode::SUCCESS;
}

//创建对应数量的算法实例
ALPReturnCode ALPAlgResourcePool::BuildAlgInstancesResRequirements() {
        LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 1." <<std::endl;  


	//创建算法对应数目的实例
	for (auto tmpPoolAlgProperty : m_PoolAlgsPropertySet) {
                
                LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 2." <<std::endl;

		const std::string tmpAlgName= tmpPoolAlgProperty->m_AlgName;
		tbb::concurrent_bounded_queue<ALPAlgBase*>* tmpQueue = new tbb::concurrent_bounded_queue<ALPAlgBase*>();
		m_AlgNameToAlgInstanceQueue[tmpAlgName] = tmpQueue;
                LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 3." <<std::endl;

		//创建算法实例并压入队列
                ALPAlgBase* tmpAlg = CreateAlgorithmInsance(tmpAlgName);
		if (!tmpAlg) {
			LogError << "Dynamic cast from AlgBase to ALPAlgBase failed. " << std::endl;
			return ALPReturnCode::FAILURE;
		}
		tmpQueue->push(tmpAlg);
                m_PoolAlgInstanceList.push_back(tmpAlg); 
                LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 4." <<std::endl;

		if (tmpAlg->IsReEntrant()) {
			//如果算法可重入
			m_AlgNameToMaxInstancesCount[tmpAlgName] = 1;
			m_AlgNameToCreatedInstancesCount[tmpAlgName] = 1;
		}
		else if (tmpAlg->IsClonable()) {
			//如果算法可克隆，则创建相应数量的算法实例
			for (unsigned int count = 1; count < tmpPoolAlgProperty->m_MaxInstanceCount; count++) {
                                ALPAlgBase* tmpAlgClone = CreateAlgorithmInsance(tmpAlgName);
				if (!tmpAlgClone) {
					LogError << "Dynamic cast from AlgBase to ALPAlgBase failed. " << std::endl;
					return ALPReturnCode::FAILURE;
				}
				tmpQueue->push(tmpAlgClone);
                                m_PoolAlgInstanceList.push_back(tmpAlgClone);
			}
			m_AlgNameToMaxInstancesCount[tmpAlgName] = tmpPoolAlgProperty->m_MaxInstanceCount;
			m_AlgNameToCreatedInstancesCount[tmpAlgName] = tmpPoolAlgProperty->m_MaxInstanceCount;
		}
		else {
			//普通算法
			m_AlgNameToMaxInstancesCount[tmpAlgName] = 1;
			m_AlgNameToCreatedInstancesCount[tmpAlgName] = 1;
		}
                LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 5." <<std::endl;

	}

        LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 7." <<std::endl;
	//为标记DAG通道当前事例运行结束的算法ALPDAGTunnelEventEndALg创建队列，并压入特定数目的算法实例
	tbb::concurrent_bounded_queue<ALPAlgBase*>* endEventAlgQueue = new tbb::concurrent_bounded_queue<ALPAlgBase*>();
	m_AlgNameToAlgInstanceQueue[m_EndEventAlg] = endEventAlgQueue;
	//默认创建100个算法实例
	for (unsigned int count = 1; count < EventPool_DAGTunnel_MaxCount; count++) {
                LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 7.1."<<std::endl;
                LogDebug<<"count="<<count<<", EventPool_DAGTunnel_MaxCount="<<EventPool_DAGTunnel_MaxCount<<". "<<std::endl;
                ALPAlgBase* endEventAlgClone = CreateAlgorithmInsance(m_EndEventAlg);
		if (!endEventAlgClone) {
			LogError << "Dynamic cast ALPDAGTunnelEventEndALg from AlgBase to ALPAlgBase failed. " << std::endl;
			return ALPReturnCode::FAILURE;
		}
                LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 7.2." <<std::endl;
		endEventAlgQueue->push(endEventAlgClone);
                LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 7.3." <<std::endl;
                m_PoolAlgInstanceList.push_back(endEventAlgClone);
                LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 7.4." <<std::endl;

	}
	m_AlgNameToMaxInstancesCount[m_EndEventAlg] = EventPool_DAGTunnel_MaxCount;
	m_AlgNameToCreatedInstancesCount[m_EndEventAlg] = EventPool_DAGTunnel_MaxCount;
        LogDebug<< "Function ALPAlgResourcePool::BuildAlgInstancesResRequirements() step 8." <<std::endl;


	return ALPReturnCode::SUCCESS;
}

bool ALPAlgResourcePool::initialize() {
        LogInfo<< "Function ALPAlgResourcePool::initialize() step 1." <<std::endl;
	//解析Json文件并创建算法实例
	ParseAlgsPropertyJsonFile();
        LogDebug<< "Function ALPAlgResourcePool::initialize() step 2." <<std::endl;
	if (m_PoolAlgsPropertySet.empty())
	{  
		LogError << "No Algorithm in event, the task will abort!" << std::endl;
		return false;
	}
        LogDebug<< "Function ALPAlgResourcePool::initialize() step 3." <<std::endl;
	BuildAlgInstancesResRequirements();
        LogDebug<< "Function ALPAlgResourcePool::initialize() step 4." <<std::endl;
	//调用派生类及其所创建的子算法及工具的initialize()方法
	for (auto tmpAlgInstance : m_PoolAlgInstanceList) {
		ALPReturnCode temp = tmpAlgInstance->InitializeRelatedAlgTool();
		if (ALPReturnCode::FAILURE == temp) {
			LogError << "Error is happening when try to start algortihm." << tmpAlgInstance->objName() << std::endl;
			return false;
		}
	}
        LogDebug<< "Function ALPAlgResourcePool::initialize() step 5." <<std::endl;
	return true;
}

bool ALPAlgResourcePool::finalize() {
	//调用派生类及其所创建的子算法及工具的finalize()方法
	for (auto tmpAlgInstance : m_PoolAlgInstanceList) {
		ALPReturnCode temp = tmpAlgInstance->FinalizeRelatedAlgTool();
		if (ALPReturnCode::FAILURE == temp) {
			LogError << "Error is happening when try to start algortihm." << tmpAlgInstance->objName() << std::endl;
			return false;
		}
	}
	return true;
}

//启动算法池
bool ALPAlgResourcePool::start() {
	//调用派生类及其所创建的子算法及工具的start()方法
	for (auto tmpAlgInstance : m_PoolAlgInstanceList) {
		ALPReturnCode temp = tmpAlgInstance->StartRelatedAlgTool();
		if (ALPReturnCode::FAILURE == temp) {
			LogError << "Error is happening when try to start algortihm." << tmpAlgInstance->objName() << std::endl;
			return false;
		}
	}
	return true;
}
//停止算法池
bool ALPAlgResourcePool::stop() {
	//调用派生类及其所创建的子算法及工具的stop()方法
	for (auto tmpAlgInstance : m_PoolAlgInstanceList) {
		ALPReturnCode temp = tmpAlgInstance->StopRelatedAlgTool();
		if (ALPReturnCode::FAILURE == temp) {
			LogError << "Error is happening when try to start algortihm." << tmpAlgInstance->objName() << std::endl;
			return false;
		}
	}
	return true;
}

//获得顶级算法列表
std::list<ALPAlgBase*> ALPAlgResourcePool::GetTopAlgList() {
        std::list<ALPAlgBase*> tmpTopAlgList;
	return tmpTopAlgList;
}


//create algorithm instance 
ALPAlgBase* ALPAlgResourcePool::CreateAlgorithmInsance(const std::string& algName)  {
    DLElement* obj = DLEFactory::instance().create(algName);
    if ( obj != nullptr ) {
        ALPAlgBase* result = dynamic_cast<ALPAlgBase*>(obj);
        if ( result != nullptr ) {
            result->setParent(this);
            return result;
        }
        else {
            LogFatal << obj->objName() << " cannot cast to AlgBase."<< std::endl;
        }
        delete obj;
    }
    return nullptr; 
}


//分配算法实例
ALPReturnCode ALPAlgResourcePool::AllocateAlgorithmInstance(const std::string& name, ALPAlgBase*& tmpAlg) {
        LogDebug << "Function ALPAlgResourcePool::AllocateAlgorithmInstance() says hello." <<std::endl;
	auto tmpAlgQueuePtr = m_AlgNameToAlgInstanceQueue.find(name);
        LogDebug << "Function ALPAlgResourcePool::AllocateAlgorithmInstance() step1." <<std::endl;
	if (tmpAlgQueuePtr == m_AlgNameToAlgInstanceQueue.end()) {
		LogError << "Algorithm name:"<<name<<" is asked, but not found! Error!" << std::endl;
		return ALPReturnCode::FAILURE;
	}
        LogDebug << "Function ALPAlgResourcePool::AllocateAlgorithmInstance() step2.name="<<name<<std::endl;
	//获取算法实例，若没有实例则阻塞
	tmpAlgQueuePtr->second->pop(tmpAlg);
        LogDebug << "Function ALPAlgResourcePool::AllocateAlgorithmInstance() step3.name="<<name<<std::endl;


	//如果算法为可重入算法，则立刻压入算法，保证可用
	if (tmpAlg->IsReEntrant()) {
		tmpAlgQueuePtr->second->push(tmpAlg);
	}
        LogDebug << "Function ALPAlgResourcePool::AllocateAlgorithmInstance() says byebye." <<std::endl;
	return ALPReturnCode::SUCCESS;
}

//回收算法实例
ALPReturnCode ALPAlgResourcePool::RecycleAlgorithmInstance(const std::string& name, ALPAlgBase*& tmpAlg) {
	//如果算法不可重入，则回收算法
	if (!tmpAlg->IsReEntrant()) {
		m_AlgNameToAlgInstanceQueue[name]->push(tmpAlg);
	}
	return ALPReturnCode::SUCCESS;
}


bool ALPAlgResourcePool::config() {
	return true;
}

//run this task
bool ALPAlgResourcePool::run() {
	return true;
}

bool ALPAlgResourcePool::execute() {
	return true;
}


