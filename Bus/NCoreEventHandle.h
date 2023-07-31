#pragma once
#include <QObject>
#include "NConfig.h"

/**
 * 注册关系汇聚处,ECoreEvent枚举代表事件信号, EEventResponse枚举代表触发时间，NWorkingManager类对象是共事者之一
*/

class NEventTrigger;
class NCoreEventHandle : public QObject
{
	Q_OBJECT

public:
	NCoreEventHandle(QObject *parent);
	~NCoreEventHandle();

private:
	void setupUI();
	void setupConnect();

private:
	void handleCreateWorking(QVariant inputContext, QVariant& outContext);
	void handleDeviceFinishOrbit(QVariant inputContext, QVariant& outContext);

private:
	NWorkingManager m_WorkingManager;
	NEventTrigger* m_pEventTrigger;
};
