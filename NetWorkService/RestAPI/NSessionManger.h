#pragma once

#include <QObject>
#include <QHash>
#include <QMutex>

// login正确登录后user就会携带session
struct SUser
{
	long long m_nID;
	QString m_strUserName;
	QString m_strPassword;
	long long m_nCreateTime;
	QString m_strShowName;
	QString m_strPosition;
	QString m_strTel;
	QString m_strWorkerID;
	QString m_strSession;
	SUser()
	{
		m_nID = -1;
		m_nCreateTime = 0;
	}
};

class NSessionManger : public QObject
{
	Q_OBJECT

public:
	NSessionManger(QObject *parent);
	~NSessionManger();

public:
	QString createSession(const SUser& user);
	bool checkSession(QString strSession);
	void delSession(QString strSession);
	QString getUserName(QString strSession);
	void cleanSession();

private:
	void setupUI();
	void setupConnect();

public:
	QMutex m_mutexSession;
	QHash<QString, SUser*> m_hashSessionStruct;			// 会话 结构 主管理容器
	QHash<QString, SUser*> m_hashUserStruct;			// 用户名 结构
};
