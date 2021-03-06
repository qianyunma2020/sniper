#ifndef SNIPER_ALP_ALG_RESOURCE_POOL_H
#define SNIPER_ALP_ALG_RESOURCE_POOL_H

#include <bitset>
#include <mutex>
#include <list>
#include <vector>
#include <string>
#include <boost/dynamic_bitset.hpp>
#include <tbb/concurrent_queue.h>
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/ALPAlgBase.h"
#include "SniperKernel/SniperLog.h"
#include "SniperKernel/Task.h"
#include "SniperKernel/gbmacrovar.h"

class ALPAlgResourcePool;
//用于暂时存储算法资源池管理的算法属性
class PoolAlgorithmProperty
{
public:
	PoolAlgorithmProperty(){}
	~PoolAlgorithmProperty(){}

private:
	friend class ALPAlgResourcePool;           //将算法资源池类声明为友元类
	std::string m_AlgName;                     //算法名称
        unsigned int m_MaxInstanceCount;           //靠靠靠靠靠靠靠靠靠1靠靠靠縩,靠靠�1
};


//尽量避免可克隆算法实例每个事例都分配回收一次导致的性能问题
class ALPAlgResourcePool : public Task
{
public:
	ALPAlgResourcePool(const std::string& name);
	~ALPAlgResourcePool();

public:
	//启动算法池
	bool start();
	//停止算法池
	bool stop() override final;
	//run this task
	bool run() override final;
	
protected:
	//the concrete task operations
	bool initialize() override final;
	bool finalize() override final;
	bool config() override final;
	bool execute() override final;
	//clear all svcs and algs
	void reset(){}


public:
	//设置存储算法属性的JSON文件名称
	void setAlgsPropertyFileName(const std::string filename) {
		m_AlgsPropertyFileName = filename;
	}

	//获得顶级算法列表
	std::list<ALPAlgBase*> GetTopAlgList();

        //create algorithm instance 
        ALPAlgBase* CreateAlgorithmInsance(const std::string& algName);
	//分配算法实例
	ALPReturnCode AllocateAlgorithmInstance(const std::string& name, ALPAlgBase*& tmpAlg);
	//回收算法实例
	ALPReturnCode RecycleAlgorithmInstance(const std::string& name, ALPAlgBase*& tmpAlg);


	//读取JSON文件，存储算法池所管理算法的属性
	ALPReturnCode ParseAlgsPropertyJsonFile();
	//解析用户脚本的配置字符串，存储算法资源池所管理算法的属性
	ALPReturnCode ParseUserScriptString();
	

private:
	//创建对应数量的算法实例
	ALPReturnCode BuildAlgInstancesResRequirements();

private:
	const std::string m_EndEventAlg = "ALPDAGTunnelEventEndAlg";             //标记DAG通道当前事例运行结束的算法

        typedef tbb::concurrent_bounded_queue<ALPAlgBase*> ALPConcurrentAlgQueue;
	std::map<std::string, ALPConcurrentAlgQueue*> m_AlgNameToAlgInstanceQueue;  //算法名称与算法实例队列的映射
	std::map<std::string, unsigned long> m_AlgNameToMaxInstancesCount;         //每个算法对应的最大实例数目
	std::map<std::string, unsigned long> m_AlgNameToCreatedInstancesCount;     //每个算法已经创建的实例数目

	std::string m_m_PoolAlgsPropertySet_String;    //用户脚本字符串
	std::string m_AlgsPropertyFileName;            //存储算法属性的JSON文件名称
	std::vector<PoolAlgorithmProperty*> m_PoolAlgsPropertySet;  //用于暂时存储算法资源池管理的算法属性
 
        std::list<ALPAlgBase*>  m_PoolAlgInstanceList;   //point to all algorithms instances managed by this pool, used for initialize,finalize,start,stop.

   

	//算法分解树功能备用
	std::vector<std::string> m_TopAlgNames;     //顶级算法名称

	std::list<ALPAlgBase*>  m_TopAlgPtrist;    //顶级算法链表
	std::list<ALPAlgBase*>  m_SubAlgPtrList;    //子算法链表
	std::list<ALPAlgBase*>  m_LeafAlgPtrList;   //叶子算法链表

};

#endif
