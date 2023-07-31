#pragma once

#include <QObject>
#include <QVector>
#include <QVariant>

enum EEventResponse;
struct SCoreMessage;
class NEventTrigger : public QObject
{
	Q_OBJECT

public:
	NEventTrigger(QObject *parent);
	~NEventTrigger();

public:
	void addEventTrigger(QVariant varCoreEvent);
	bool getTriggerNextOrbit(SCoreMessage& varCoreEvent);
	bool getTriggerNextPlane(SCoreMessage& varCoreEvent);

private:
	void setupUI();
	void setupConnect();
	bool getTrigger(SCoreMessage& varCoreEvent, EEventResponse eventResponse);

private:
	QVector<QVariant> m_vecCoreEvent;
};
