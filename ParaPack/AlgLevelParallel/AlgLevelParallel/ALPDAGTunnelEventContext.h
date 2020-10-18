#ifndef SNIPER_ALP_DAG_TUNNEL_EVENT_CONTEXT_H
#define SNIPER_ALP_DAG_TUNNEL_EVENT_CONTEXT_H

#include <cstdint>
#include "SniperKernel/ALPReturnCode.h"
#include "SniperKernel/ALPEventUnit.h"

//DAGͨ����ǰ�����������Ļ���
class ALPDAGTunnelEventContext
{
public:
	ALPDAGTunnelEventContext();
	~ALPDAGTunnelEventContext();
	
	//�ж��ļ��Ƿ�������
	bool FileOccurAlteration(const std::string filename);
	//�����ļ������Ļ���
	ALPReturnCode SetFileContext(const std::string filename);
	//�����ļ������Ļ���
	void ResetFileContext();

	//�������������Ļ���
	ALPReturnCode SetCurrentEventContext(ALPEventUnit* tmpEventUnitPtr);
	//�������������Ļ���
	ALPReturnCode ReSetEventContext();

	//��ȡ��ǰ�������ݵ�ָ��
	void* GetEventDataPtr();

private:
	ALPEventUnit* m_EventUnitPtr;   //ָ��������Ԫ��ָ��
	std::string m_InputFileName;    //�����ļ�����

};


#endif
