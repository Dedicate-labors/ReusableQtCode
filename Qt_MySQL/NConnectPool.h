#pragma once

#include <QString>
#include <QList>
#include <QMutex>
#include <QSqlDatabase>

/**
 * NConnectPool直接管理Qt的QSqlDatabase，因为QSqlDatabase的Qt静态部分无法跨线程，为了跨线程使用，所以保留QSqlDatabase为成员变量
 * 因为NAccessDataBase一般是全局对象无法在NCoreApplication之前析构，所以QSqlDatabase成员变量的析构需要在main结束前主动调用
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
    QList<QSqlDatabase> m_listUsedConnection;
    QList<QSqlDatabase> m_listUnusedConnection;

private:
    QString m_strIP;
    int m_nPort;
    QString m_strDBName;
    QString m_strUserName;
    QString m_strPassword;
};
