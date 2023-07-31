#include "NCoreEventHandle.h"
#include "NEventTrigger.h"
#include "NCoreBusControl.h"
#include <QVariant>

NCoreEventHandle::NCoreEventHandle(QObject *parent)
	: QObject(parent)
	, m_pEventTrigger(NULL)
{
	setupUI();
	setupConnect();
}

NCoreEventHandle::~NCoreEventHandle()
{

}

void NCoreEventHandle::setupUI()
{
	m_pEventTrigger = new NEventTrigger(this);
}

void NCoreEventHandle::setupConnect()
{
	NCoreBusControl & coreBusControl = NCoreBusControl::getInstance();
	coreBusControl.RegRecvBin(ECE_CREATEWORKING, std::bind(&NCoreEventHandle::handleCreateWorking, this, std::placeholders::_1, std::placeholders::_2));
	coreBusControl.RegRecvBin(ECE_DEVICEFINISHORBIT, std::bind(&NCoreEventHandle::handleDeviceFinishOrbit, this, std::placeholders::_1, std::placeholders::_2));
	// 注册函数
}

void NCoreEventHandle::handleCreateWorking(QVariant inputContext, QVariant& outContext)
{
	if (inputContext.canConvert<SCoreMessage>())
	{
		SCoreMessage coreMessage = inputContext.value<SCoreMessage>();
		if (EER_IMMEDIATELY == coreMessage.m_eventResponse)
		{
			EErrorCode errorCode = m_WorkingManager.eventCreateWorking(coreMessage.m_strDeviceName);
			outContext = errorCode;
		}
		else
		{
			m_pEventTrigger->addEventTrigger(inputContext);
		}
	}
}

void NCoreEventHandle::handleDeviceFinishOrbit(QVariant inputContext, QVariant &outContext)
{
	SCoreMessage coreMessage;
	while (m_pEventTrigger->getTriggerNextOrbit(coreMessage))
	{
		if(ECE_PLANJOIN == coreMessage.m_coreEvent)
		{
			m_WorkingManager.eventPlanJoin(coreMessage.m_strDeviceName);
		}
	}
}
