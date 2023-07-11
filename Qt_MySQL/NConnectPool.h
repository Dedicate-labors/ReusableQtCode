#pragma once

#include <QString>
#include <QList>
#include <QMutex>
#include <QSqlDatabase>

class NConnectPool
{
public:
	NConnectPool();
	~NConnectPool();

public:
	bool initConnectPool(QString strIP, int nPort, QString strDBName, QString strUserName, QString strPassword);
	QSqlDatabase getConnect();
	void releaseConnect(QSqlDatabase sqlDatabase);

private:
	QMutex m_mutex;
	QList<QSqlDatabase> m_listUsedConnection;
	QList<QSqlDatabase> m_listUnusedConnection;

private:
	QString m_strIP;
	int m_nPort;
	QString m_strDBName;
	QString m_strUserName;
	QString m_strPassword;
};
