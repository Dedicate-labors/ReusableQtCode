#include "NRestAPIService.h"
#include "NRestAPIHandle.h"
#include <QDebug>

NRestAPIService::NRestAPIService(QObject *parent)
	: QObject(parent)
	, m_pRestAPIHandle(NULL)
{
	setupUI();
	setupConnect();
}

NRestAPIService::~NRestAPIService()
{
}


void NRestAPIService::initRestAPIService(int nPort)
{
	m_nPort = nPort;
	m_restService.setHttpAcceptedCallback(std::bind(&NRestAPIService::onHttpAccepted, this, std::placeholders::_1));
}

bool NRestAPIService::startService()
{
	return m_restService.listen(QHostAddress::Any, m_nPort);
}

void NRestAPIService::setupUI()
{
	m_pRestAPIHandle = new NRestAPIHandle(this);
}

void NRestAPIService::setupConnect()
{

}

void NRestAPIService::onHttpAccepted(const QPointer< JQHttpServer::Session > &session)
{
	if (QStringLiteral("POST") == session->requestMethod().toUpper())
	{
		m_pRestAPIHandle->handle(session);
	}
}