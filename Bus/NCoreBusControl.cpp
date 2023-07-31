#include "NCoreBusControl.h"

NCoreBusControl::NCoreBusControl(QObject *parent)
	: QObject(parent)
{

}

NCoreBusControl::~NCoreBusControl()
{

}

void NCoreBusControl::InitBusControl()
{
	connect(this, &NCoreBusControl::postExecBusBin, this, &NCoreBusControl::actionExecBusBin);
}

void NCoreBusControl::RegRecvBin(int nBusType, std::function<void(QVariant inputContext, QVariant& outContext)> fun)
{
	m_mapBusControl.insert(nBusType, fun);
}

void NCoreBusControl::RemoveRecvBin(int nBusType)
{
	m_mapBusControl.remove(nBusType);
}

bool NCoreBusControl::ExecBusBin(int nBusType, QVariant inputContext, QVariant& outContext)
{
	bool bHaveKey = false;
	QList<std::function<void(QVariant inputContext, QVariant& outContext)>> vecValues = m_mapBusControl.values(nBusType);
	for (QList<std::function<void(QVariant inputContext, QVariant& outContext)>>::iterator iTer = vecValues.begin(); iTer != vecValues.end(); ++iTer)
	{
		bHaveKey = true;
		(*iTer)(inputContext, outContext);
	}
	return bHaveKey;
}

bool NCoreBusControl::ExecBusBin(int nBusType, QVariant inputContext)
{
	bool bHaveKey = false;
	QList<std::function<void(QVariant inputContext, QVariant& outContext)>> vecValues = m_mapBusControl.values(nBusType);
	for (QList<std::function<void(QVariant inputContext, QVariant& outContext)>>::iterator iTer = vecValues.begin(); iTer != vecValues.end(); ++iTer)
	{
		bHaveKey = true;
		emit postExecBusBin((*iTer), inputContext);
	}
	return bHaveKey;
}

void NCoreBusControl::actionExecBusBin(std::function<void(QVariant inputContext, QVariant& outContext)> fun, QVariant inputContext)
{
    QVariant var;
    fun(inputContext, var);
}
