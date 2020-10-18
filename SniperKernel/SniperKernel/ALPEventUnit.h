#ifndef SNIPER_ALP_EVENT_UNIT_H
#define SNIPER_ALP_EVENT_UNIT_H

#include <cstdint>
#include <vector>
#include <cstddef>



//�������ݵ�Ԫ
class ALPEventUnit final
{
public:
	ALPEventUnit(void* tmpEventDataPtr);
	ALPEventUnit(uint64_t tmpEventID, unsigned long int tmpRunID, void* tmpEventDataPtr);
	~ALPEventUnit();

	uint64_t GetEventID() { return m_EventID; }
	unsigned long int GetRunID() { return m_RunID; }
	void* GetEventDataPtr() { return m_EventDataPtr; }
	//��������
	void SetEventDataPtr(void* tmpDataPtr);

private:
	uint64_t m_EventID;           //����ID
	unsigned long int m_RunID;    //���к�
	void* m_EventDataPtr;         //ָ���������ݵ�ָ��


private:
	// following interfaces are not supported
	ALPEventUnit(const ALPEventUnit&);
	ALPEventUnit& operator=(const ALPEventUnit&);
};


#endif
