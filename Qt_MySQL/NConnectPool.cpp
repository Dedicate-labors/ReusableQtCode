#include "NConnectPool.h"
#include <QDebug>

NConnectPool::NConnectPool()
{
}

NConnectPool::~NConnectPool()
{
    // QMutexLocker locker(&m_mutex);
    // foreach(QSqlDatabase sqlDatabase, m_listUnusedConnection)
    // {
    //     sqlDatabase.close();
    //     QSqlDatabase::removeDatabase(sqlDatabase.connectionName());
    // }

    // foreach(QSqlDatabase sqlDatabase, m_listUsedConnection)
    // {
    //     sqlDatabase.close();
    //     QSqlDatabase::removeDatabase(sqlDatabase.connectionName());
    // }
}

bool NConnectPool::initConnectPool(QString strIP, int nPort, QString strDBName, QString strUserName, QString strPassword)
{
    m_strIP = strIP;
    m_nPort = nPort;
    m_strDBName = strDBName;
    m_strUserName = strUserName;
    m_strPassword = strPassword;

    for (int nIndex = 0; nIndex < 5; ++nIndex)
    {
        QSqlDatabase sqlDatabase;
        // 建议使用随机名称
        QString strConnectName = QStringLiteral("Driver_%1").arg(nIndex);
        sqlDatabase = QSqlDatabase::addDatabase(QStringLiteral("QMYSQL"), strConnectName);
        sqlDatabase.setHostName(strIP);
        sqlDatabase.setPort(nPort);
        sqlDatabase.setDatabaseName(strDBName);
        sqlDatabase.setUserName(strUserName);
        sqlDatabase.setPassword(strPassword);
        sqlDatabase.setConnectOptions(QStringLiteral("MYSQL_OPT_RECONNECT=1"));
        if (!sqlDatabase.open())
        {
            qDebug() << "Open MySQL fail";
            return false;
        }
        m_listUnusedConnection.push_back(sqlDatabase);
    }
    qInfo() << "Open MySQL success";
    return true;
}

QSqlDatabase NConnectPool::getConnect()
{
    QMutexLocker locker(&m_mutex);
    if (m_listUnusedConnection.size() > 0)
    {
        QSqlDatabase sqlDatabase = m_listUnusedConnection.takeFirst();
        m_listUsedConnection.push_back(sqlDatabase);
        return sqlDatabase;
    }
    else
    {
        int nCount = m_listUnusedConnection.size() + m_listUsedConnection.size();

        QSqlDatabase sqlDatabase;
        QString strConnectName = QStringLiteral("Driver_%1").arg(nCount);
        sqlDatabase = QSqlDatabase::addDatabase(QStringLiteral("QMYSQL"), strConnectName);
        sqlDatabase.setHostName(m_strIP);
        sqlDatabase.setPort(m_nPort);
        sqlDatabase.setDatabaseName(m_strDBName);
        sqlDatabase.setUserName(m_strUserName);
        sqlDatabase.setPassword(m_strPassword);
        sqlDatabase.setConnectOptions(QStringLiteral("MYSQL_OPT_RECONNECT=1"));
        sqlDatabase.open();
        m_listUsedConnection.push_back(sqlDatabase);
        return sqlDatabase;
    }
}

void NConnectPool::releaseConnect(QSqlDatabase sqlDatabase)
{
    QMutexLocker locker(&m_mutex);
    for (QList<QSqlDatabase>::iterator iTer = m_listUsedConnection.begin(); iTer != m_listUsedConnection.end(); ++iTer)
    {
        if (iTer->connectionName() == sqlDatabase.connectionName())
        {
            m_listUsedConnection.erase(iTer);
            break;;
        }
    }
    m_listUnusedConnection.push_back(sqlDatabase);
}
