#include "AlgLevelParallel/ALPDAGTunnelDog.h"
#include "SniperKernel/SniperLog.h"
#include "AlgLevelParallel/ALPDAGTunnel.h"


ALPDAGTunnelDog::ALPDAGTunnelDog(ALPDAGTunnel* tmpDAGTunnel, ALPEventPool* tmpEventPool)
{
    //ÖÃ³õÖµ
    m_CurrentEventGroup = NULL;
    m_DAGTunnelPtr = tmpDAGTunnel;
    m_EventPool = tmpEventPool;

}

ALPDAGTunnelDog::~ALPDAGTunnelDog()
{
    //ÖØÉèÊÂÀıÉÏÏÂÎÄ»·¾³
    m_CurrentEventContext.ReSetEventContext();
    //ÖØÉèÎÄ¼şÉÏÏÂÎÄ»·¾³
    m_CurrentEventContext.ResetFileContext();
    
}

bool ALPDAGTunnelDog::initialize() {   // start event loop thread 
    //ÖÃÏß³Ì×´Ì¬³õÖ
    m_EventLoopThreadState = INACTIVE;
    LogInfo<<"Activating event loop thread......................"<<std::endl;
    //Æô¶¯ÊÂÀıÑ­»·Ïß³Ì
    m_EventLoopThread = std::thread([this]() {this->ActivateTunnelEventLoopThread(); });
    //µÈ´ıÏß³Ì¼¤»î
    while (ACTIVE != m_EventLoopThreadState)
    {
        if (ERROR == m_EventLoopThreadState) {
            LogError << "Function ALPDAGTunnelDog::ALPDAGTunnelDog() implys flag m_EventLoopThreadState is ERROR!" << std::endl;
        }
        LogInfo << "Waiting for thread to active............." << std::endl;
        sleep(1);
    }
    return true;
}

bool ALPDAGTunnelDog::finalize() {     //nothing 
    return true;
}

//¼¤»îÍ¨µÀÊÂÀıÑ­»·Ïß³Ì£¬¸ºÔğ²»¶ÏµÄ´ÓÊÂÀı³ØÖĞ»ñÈ¡ÊÂÀı×é
void ALPDAGTunnelDog::ActivateTunnelEventLoopThread() {
    //¸üĞÂÊÂÀıÑ­»·Ïß³Ì×´Ì¬
    m_EventLoopThreadState = ACTIVE;
    //ÊÂÀıÑ­»·Ïß³Ì¿ªÊ¼ÏòDAGTunnel ÖĞÑ¹ÈëÊÂÀı
    LogInfo << "The Event Loop thread starts working......" << std::endl;
    
    while (ACTIVE == m_EventLoopThreadState)
    {
        if (NULL == m_CurrentEventGroup) {
            //ÈôÊÂÀı×éÉĞÎ´·ÖÅä,ÔòÉêÇëĞÂµÄÊÂÀı×é
            //ÉêÇëÊ§°ÜÊ±£¬ËµÃ÷ÊÂÀı´¦Àí¹¤×÷ÒÑ¾­½áÊø£¬³ÌĞò×¼±¸ÍË³ö
            if (ALPReturnCode::FAILURE == RequestReadyStateEventGroup()) {
                //Í£Ö¹Í¨µÀÊÂÀıÑ­»·Ïß³Ì
                DeactivateTunnelEventLoopThread();
            }
            else {
                //Ñ¹ÈëĞÂµÄÊÂÀı²¢µÈ´ıTunnel´¦ÀíÍê³É
                PushNewEventInTunnel();

                //µ¯³öµ±Ç°ÊÂÀı
                PopCurrentEventFromTunnel();
            }

        }else if (m_CurrentEventGroup->IsProcessed()) {
            //ÈôÊÂÀı×éÒÑ¾­´¦ÀíÍê³É,Ôòµ¯³ö¸ÃÊÂÀı×é£¬²¢·ÖÅäĞÂµÄÊÂÀı×é
            PopCurrentEventGroup();
            //ÉêÇëÊ§°ÜÊ±£¬ËµÃ÷ÊÂÀı´¦Àí¹¤×÷ÒÑ¾­½áÊø£¬³ÌĞò×¼±¸ÍË³ö
            if (ALPReturnCode::FAILURE == RequestReadyStateEventGroup()) {
                //Í£Ö¹Í¨µÀÊÂÀıÑ­»·Ïß³Ì
                DeactivateTunnelEventLoopThread();
            }
            else {
                //Ñ¹ÈëĞÂµÄÊÂÀı²¢µÈ´ıTunnel´¦ÀíÍê³É
                PushNewEventInTunnel();

                //µ¯³öµ±Ç°ÊÂÀı
                PopCurrentEventFromTunnel();
            }  
            
        }else
        {
            //ÈôÊÂÀı×éÒÑ¾­·ÖÅäÇÒÉĞÎ´´¦ÀíÍê³É,ÔòÑ¹ÈëĞÂµÄÊÂÀı²¢µÈ´ıTunnel´¦ÀíÍê³É
            PushNewEventInTunnel();

            //µ¯³öµ±Ç°ÊÂÀı
            PopCurrentEventFromTunnel();
        }
        
    }
}

//Í£Ö¹Í¨µÀÊÂÀıÑ­»·Ïß³Ì
ALPReturnCode ALPDAGTunnelDog::DeactivateTunnelEventLoopThread() {
    if (ACTIVE == m_EventLoopThreadState) {
        LogInfo << "The Event Loop thread stop working......" << std::endl;
        m_EventLoopThreadState = INACTIVE;
    }
    return ALPReturnCode::SUCCESS;
}

//ÉêÇë´¦ÓÚReady×´Ì¬µÄÊÂÀı×é
ALPReturnCode ALPDAGTunnelDog::RequestReadyStateEventGroup() {
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() says hello."<<std::endl;
    //ÉêÇëĞÂµÄÊÂÀı×é
    m_CurrentEventGroup = m_EventPool->AllocateReadyEventGroup();
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() step1."<<std::endl;
    //Èç¹ûÎªNULL,ÔòËµÃ÷ÊÂÀı´¦Àí¹¤×÷ÒÑ¾­½áÊø£¬³ÌĞò×¼±¸ÍË³ö
    if (NULL == m_CurrentEventGroup)
        return ALPReturnCode::FAILURE;
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() step2."<<std::endl;
    //Èç¹ûÎÄ¼ş·¢Éú±ä¸ü£¬Ôò¸üĞÂÎÄ¼şÉÏÏÂÎÄ»·¾³
    if (m_CurrentEventContext.FileOccurAlteration(m_CurrentEventGroup->GetCurrentFileName())) {
        m_CurrentEventContext.SetFileContext(m_CurrentEventGroup->GetCurrentFileName());
    }
    LogDebug<<"Class ALPDAGTunnelDog function RequestReadyStateEventGroup() step3."<<std::endl;
    return ALPReturnCode::SUCCESS;
}

//µ¯³öÒÑ¾­´¦ÀíÍê³ÉµÄÊÂÀı×é
ALPReturnCode ALPDAGTunnelDog::PopCurrentEventGroup() {
    LogDebug<<"Class ALPDAGTunnelDog function PopCurrentEventGroup says hello."<<std::endl;
    //¸üĞÂÊÂÀı×éµÄ×´Ì¬
    m_CurrentEventGroup->SetGroupState(EventIOQueueStream_GroupState_Processed);
    //µ±Ç°ÊÂÀı×é´¦ÀíÍê³É£¬¸üĞÂProcessed×´Ì¬ÊÂÀı×éÊıÄ¿µÄĞÅºÅÁ¿£¬ÒÔ¹©ÎÄ¼şÊä³öÏß³ÌÊ¹ÓÃ
    m_EventPool->PushProcessedEventGroupSemaphore();

    return ALPReturnCode::SUCCESS;
}

//ÉêÇëËã·¨ÊµÀı£¬ÏòDAG TunnelÖĞÑ¹ÈëĞÂµÄÊÂÀı
ALPReturnCode ALPDAGTunnelDog::PushNewEventInTunnel() {
    //¸üĞÂÊÂÀıÉÏÏÂÎÄ»·¾³
    //LogDebug<<"Class ALPDAGTunnelDog function PushNewEventInTunnel says hello."<<std::endl;
    m_CurrentEventContext.SetCurrentEventContext(m_CurrentEventGroup->EventToProcess());
    //LogDebug<<"Class ALPDAGTunnelDog function PushNewEventInTunnel step1."<<std::endl;
 
    //ÊÂÀıÒÑ¾­Ñ¹Èë£¬¿ªÊ¼´¦Àíµ±Ç°ÊÂÀı
    m_DAGTunnelPtr->BeginEventProcessing(m_DAGTunnelPtr);
    //LogDebug<<"Class ALPDAGTunnelDog function PushNewEventInTunnel step2."<<std::endl; 

    return ALPReturnCode::SUCCESS;
}

//ÊÍ·Å²¿·ÖËã·¨ÊµÀı£¬´ÓTunnelÖĞµ¯³öÒÑ¾­´¦ÀíÍê³ÉµÄÊÂÀı
ALPReturnCode ALPDAGTunnelDog::PopCurrentEventFromTunnel() {
    //LogDebug<<"Class ALPDAGTunnelDog function PopCurrentEventFromTunnel says hello."<<std::endl;
    //µÈ´ıDAG Tunnel Íê³Éµ±Ç°ÊÂÀıµÄ´¦Àí
    m_DAGTunnelPtr->m_TunnelEventEnd_Semaphore.wait();
    //LogDebug<<"Class ALPDAGTunnelDog function PopCurrentEventFromTunnel will pop event now."<<std::endl;
 
    //ÖØÉèÊÂÀıÉÏÏÂÎÄ»·¾³
    m_CurrentEventContext.ReSetEventContext();
    //reset all DAG nodes status
    m_DAGTunnelPtr->ResetAllDAGNodesStatus();

    return ALPReturnCode::SUCCESS;
}


//»ñµÃÖ¸Ïòµ±Ç°ÊÂÀıÊı¾İµÄÖ¸Õë
void* ALPDAGTunnelDog::GetCurrentEventDataPtr() {
    //LogDebug<<"Class ALPDAGTunnelDog function GetCurrentEventDataPtr says hello."<<std::endl;
    return m_CurrentEventContext.GetEventDataPtr();
}



