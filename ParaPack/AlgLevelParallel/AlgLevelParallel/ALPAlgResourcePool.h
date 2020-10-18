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
//ÓÃÓÚÔİÊ±´æ´¢Ëã·¨×ÊÔ´³Ø¹ÜÀíµÄËã·¨ÊôĞÔ
class PoolAlgorithmProperty
{
public:
	PoolAlgorithmProperty(){}
	~PoolAlgorithmProperty(){}

private:
	friend class ALPAlgResourcePool;           //½«Ëã·¨×ÊÔ´³ØÀàÉùÃ÷ÎªÓÑÔªÀà
	std::string m_AlgName;                     //Ëã·¨Ãû³Æ
        unsigned int m_MaxInstanceCount;           //¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿1¿¿¿¿¿¿¿n,¿¿¿¿¿1
};


//¾¡Á¿±ÜÃâ¿É¿ËÂ¡Ëã·¨ÊµÀıÃ¿¸öÊÂÀı¶¼·ÖÅä»ØÊÕÒ»´Îµ¼ÖÂµÄĞÔÄÜÎÊÌâ
class ALPAlgResourcePool : public Task
{
public:
	ALPAlgResourcePool(const std::string& name);
	~ALPAlgResourcePool();

public:
	//Æô¶¯Ëã·¨³Ø
	bool start();
	//Í£Ö¹Ëã·¨³Ø
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
	//ÉèÖÃ´æ´¢Ëã·¨ÊôĞÔµÄJSONÎÄ¼şÃû³Æ
	void setAlgsPropertyFileName(const std::string filename) {
		m_AlgsPropertyFileName = filename;
	}

	//»ñµÃ¶¥¼¶Ëã·¨ÁĞ±í
	std::list<ALPAlgBase*> GetTopAlgList();

        //create algorithm instance 
        ALPAlgBase* CreateAlgorithmInsance(const std::string& algName);
	//·ÖÅäËã·¨ÊµÀı
	ALPReturnCode AllocateAlgorithmInstance(const std::string& name, ALPAlgBase*& tmpAlg);
	//»ØÊÕËã·¨ÊµÀı
	ALPReturnCode RecycleAlgorithmInstance(const std::string& name, ALPAlgBase*& tmpAlg);


	//¶ÁÈ¡JSONÎÄ¼ş£¬´æ´¢Ëã·¨³ØËù¹ÜÀíËã·¨µÄÊôĞÔ
	ALPReturnCode ParseAlgsPropertyJsonFile();
	//½âÎöÓÃ»§½Å±¾µÄÅäÖÃ×Ö·û´®£¬´æ´¢Ëã·¨×ÊÔ´³ØËù¹ÜÀíËã·¨µÄÊôĞÔ
	ALPReturnCode ParseUserScriptString();
	

private:
	//´´½¨¶ÔÓ¦ÊıÁ¿µÄËã·¨ÊµÀı
	ALPReturnCode BuildAlgInstancesResRequirements();

private:
	const std::string m_EndEventAlg = "ALPDAGTunnelEventEndAlg";             //±ê¼ÇDAGÍ¨µÀµ±Ç°ÊÂÀıÔËĞĞ½áÊøµÄËã·¨

        typedef tbb::concurrent_bounded_queue<ALPAlgBase*> ALPConcurrentAlgQueue;
	std::map<std::string, ALPConcurrentAlgQueue*> m_AlgNameToAlgInstanceQueue;  //Ëã·¨Ãû³ÆÓëËã·¨ÊµÀı¶ÓÁĞµÄÓ³Éä
	std::map<std::string, unsigned long> m_AlgNameToMaxInstancesCount;         //Ã¿¸öËã·¨¶ÔÓ¦µÄ×î´óÊµÀıÊıÄ¿
	std::map<std::string, unsigned long> m_AlgNameToCreatedInstancesCount;     //Ã¿¸öËã·¨ÒÑ¾­´´½¨µÄÊµÀıÊıÄ¿

	std::string m_m_PoolAlgsPropertySet_String;    //ÓÃ»§½Å±¾×Ö·û´®
	std::string m_AlgsPropertyFileName;            //´æ´¢Ëã·¨ÊôĞÔµÄJSONÎÄ¼şÃû³Æ
	std::vector<PoolAlgorithmProperty*> m_PoolAlgsPropertySet;  //ÓÃÓÚÔİÊ±´æ´¢Ëã·¨×ÊÔ´³Ø¹ÜÀíµÄËã·¨ÊôĞÔ
 
        std::list<ALPAlgBase*>  m_PoolAlgInstanceList;   //point to all algorithms instances managed by this pool, used for initialize,finalize,start,stop.

   

	//Ëã·¨·Ö½âÊ÷¹¦ÄÜ±¸ÓÃ
	std::vector<std::string> m_TopAlgNames;     //¶¥¼¶Ëã·¨Ãû³Æ

	std::list<ALPAlgBase*>  m_TopAlgPtrist;    //¶¥¼¶Ëã·¨Á´±í
	std::list<ALPAlgBase*>  m_SubAlgPtrList;    //×ÓËã·¨Á´±í
	std::list<ALPAlgBase*>  m_LeafAlgPtrList;   //Ò¶×ÓËã·¨Á´±í

};

#endif
