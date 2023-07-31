#include "NEventTrigger.h"
#include "NConfig.h"

NEventTrigger::NEventTrigger(QObject *parent)
	: QObject(parent)
{
	setupUI();
	setupConnect();
}

NEventTrigger::~NEventTrigger()
{
}

void NEventTrigger::addEventTrigger(QVariant varCoreEvent)
{
	m_vecCoreEvent.push_back(varCoreEvent);
}

bool NEventTrigger::getTriggerNextOrbit(SCoreMessage& varCoreEvent)
{
	return getTrigger(varCoreEvent, EER_NEXTORBIT);
}

bool NEventTrigger::getTriggerNextPlane(SCoreMessage& varCoreEvent)
{
	return getTrigger(varCoreEvent, EER_NEXTPLANE);
}

void NEventTrigger::setupUI()
{

}

void NEventTrigger::setupConnect()
{

}

bool NEventTrigger::getTrigger(SCoreMessage& varCoreEvent, EEventResponse eventResponse)
{
	QString strVarName = QVariant::fromValue(varCoreEvent).typeName();
	for (QVector<QVariant>::iterator iTer = m_vecCoreEvent.begin(); iTer != m_vecCoreEvent.end(); ++iTer)
	{
		if (strVarName == iTer->typeName())
		{
			varCoreEvent = iTer->value<SCoreMessage>();
			if (eventResponse == varCoreEvent.m_eventResponse)
			{
				m_vecCoreEvent.erase(iTer);
				return true;
			}
		}
	}
	return false;
}