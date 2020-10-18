#ifndef SNIPER_ALP_EVENTGROUP_H
#define SNIPER_ALP_EVENTGROUP_H

#include <atomic>
#include <string>
#include "SniperKernel/gbmacrovar.h"
#include"SniperKernel/ALPEventUnit.h"

//��������
class ALPEventGroup
{
public:
    ALPEventGroup();
    ~ALPEventGroup();

public:
    ALPEventUnit* NextEvent(unsigned long nsteps = 1, bool read = true);     //�����̻߳�ȡ��һ������
    ALPEventUnit* PrevEvent(unsigned long nsteps = 1, bool read = true);     //�����̻߳�ȡ��Ӧ��������һ������
    ALPEventUnit* HeadEvent(bool read = true);                               //�����̻߳�ȡ��Ӧ�������һ������
    ALPEventUnit* TailEvent(bool read = true);                               //�����̻߳�ȡ��Ӧ���������һ������

    bool EventToInput(void* tmpEventPtr);                           //�����߳�������������
    ALPEventUnit* EventToProcess();                                 //�����̻߳�ȡ����
    ALPEventUnit* EventToOutput(unsigned long nsteps = 1);          //����̻߳�ȡ���� 

public:
    void SetGroupState(long tmpState);                         //�����������״̬
    long GetGroupState();                                      //�����������״̬
    void SetCurrentFileName(const std::string tmpFileName);    //���õ�ǰFileContext��Ӧ���ļ�����
    std::string GetCurrentFileName();                          //���ص�ǰFileContext��Ӧ���ļ�����  

    unsigned long  GetGroupSize();                             //ÿ��������Ŀ
    bool IsEmpty();                                            //�ж��������Ƿ�Ϊ��
    bool IsReady();                                            //�ж��������Ƿ����ݾ���
    bool IsProcessed();                                        //�������Ƿ��Ѿ��������
    bool IsFileWriting();                                      //�������Ƿ������ڴ���д״̬
    bool IsRelease();                                          //�������Ƿ��Ѿ�������ļ� 
    bool IsError();                                            //�������Ƿ��ڴ���״̬

    
private:
    void SetRunningEventIndex(unsigned long tmpIndex);     //�����������ڵĵ�ǰ����λ��

private:
    ALPEventUnit* m_EventPtr[EventIOQueueStream_GroupSize];         //ָ��������ָ������
    std::atomic<long>    m_EventGroupState;                         //��ʶ�������鵱ǰ״̬
    unsigned long m_Inputting_EventIndex;                           //��¼�ļ������̵߳���������λ��
    unsigned long m_Running_EventIndex;                             //��¼���������̵߳���������λ��
    unsigned long m_Outputting_EventIndex;                          //��¼�ļ�����̵߳���������λ��
    std::string m_CurentFileName;                                   //�����鵱ǰFileContext��Ӧ���ļ�����,�����ӳټ���

private:
    // following interfaces are not supported
    ALPEventGroup(const ALPEventGroup&);
    ALPEventGroup& operator=(const ALPEventGroup&);
};

#endif // !SNIPER_ALP_EVENTGROUP_H
