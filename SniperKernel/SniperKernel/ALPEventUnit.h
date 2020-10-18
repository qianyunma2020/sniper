#ifndef SNIPER_ALP_EVENT_UNIT_H
#define SNIPER_ALP_EVENT_UNIT_H

#include <cstdint>
#include <vector>
#include <cstddef>



//事例数据单元
class ALPEventUnit final
{
public:
	ALPEventUnit(void* tmpEventDataPtr);
	ALPEventUnit(uint64_t tmpEventID, unsigned long int tmpRunID, void* tmpEventDataPtr);
	~ALPEventUnit();

	uint64_t GetEventID() { return m_EventID; }
	unsigned long int GetRunID() { return m_RunID; }
	void* GetEventDataPtr() { return m_EventDataPtr; }
	//设置数据
	void SetEventDataPtr(void* tmpDataPtr);

private:
	uint64_t m_EventID;           //事例ID
	unsigned long int m_RunID;    //运行号
	void* m_EventDataPtr;         //指向事例数据的指针


private:
	// following interfaces are not supported
	ALPEventUnit(const ALPEventUnit&);
	ALPEventUnit& operator=(const ALPEventUnit&);
};


#endif
