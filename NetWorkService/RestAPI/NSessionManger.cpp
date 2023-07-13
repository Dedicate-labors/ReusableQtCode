#include "NSessionManger.h"
#include <QUuid>
#include <QMutexLocker>

NSessionManger::NSessionManger(QObject *parent)
	: QObject(parent)
{
	setupUI();
	setupConnect();
}

NSessionManger::~NSessionManger()
{
	cleanSession();
}

QString NSessionManger::createSession(const SUser& user)
{
	QString strSession = QUuid::createUuid().toString().mid(1, 36);
	QMutexLocker mutexLocker(&m_mutexSession);

	if (m_hashUserStruct.contains(user.m_strUserName))
	{
		if (SUser* pUser = m_hashUserStruct[user.m_strUserName])
		{
			QString strOldSession = pUser->m_strSession;
			pUser->m_strSession = strSession;
			m_hashSessionStruct.remove(strOldSession);
			m_hashSessionStruct.insert(strSession, pUser);
		}
	}
	else
	{
		SUser *pUser = new SUser();
		*pUser = user;

		pUser->m_strSession = strSession;
		m_hashSessionStruct.insert(pUser->m_strSession, pUser);
		m_hashUserStruct.insert(pUser->m_strUserName, pUser);	
	}
	return strSession;
}

bool NSessionManger::checkSession(QString strSession)
{
	QMutexLocker mutexLocker(&m_mutexSession);
	return m_hashSessionStruct.contains(strSession);
}

void NSessionManger::delSession(QString strSession)
{
	QString strSessionUser;
	QMutexLocker mutexLocker(&m_mutexSession);

	QHash<QString, SUser*>::iterator iTer = m_hashSessionStruct.find(strSession);
	if (iTer != m_hashSessionStruct.end())
	{
		SUser* pDelUser = *iTer;
		strSessionUser = (*iTer)->m_strUserName;
		m_hashSessionStruct.erase(iTer);
		delete pDelUser;
	}

	if (!strSessionUser.isEmpty())
	{
		m_hashUserStruct.remove(strSessionUser);
	}
}

QString NSessionManger::getUserName(QString strSession)
{
	QString strUserName;
	QMutexLocker mutexLocker(&m_mutexSession);

	QHash<QString, SUser*>::iterator iTer = m_hashSessionStruct.find(strSession);
	if (iTer != m_hashSessionStruct.end())
	{
		strUserName = (*iTer)->m_strUserName;
	}
	return strUserName;
}

void NSessionManger::cleanSession()
{
	for (QHash<QString, SUser*>::iterator iTer = m_hashSessionStruct.begin(); iTer != m_hashSessionStruct.end(); ++iTer)
	{
		delete (*iTer);
	}
	m_hashSessionStruct.clear();
	m_hashUserStruct.clear();
}

void NSessionManger::setupUI()
{

}

void NSessionManger::setupConnect()
{

}
