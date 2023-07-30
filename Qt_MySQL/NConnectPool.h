#pragma once

#include <QString>
#include <QList>
#include <QMutex>
#include <QSqlDatabase>

/**
 * NConnectPool直接管理Qt的QSqlDatabase，但不再保留QSqlDatabase副本做成员变量。
 * 如果QSqlDatabase保留为成员变量，其析构一定要在QCoreApplication之前，否则会导致未定义析构问题
 * 
 * NAccessDataBase为了方便使用一般是全局变量，所以无法保留QSqlDatabase副本做成员变量
*/
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
    QList<QString> m_listUsedConnection;
    QList<QString> m_listUnusedConnection;

private:
	QString m_strIP;
	int m_nPort;
	QString m_strDBName;
	QString m_strUserName;
	QString m_strPassword;
};
