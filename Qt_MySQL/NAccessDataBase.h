#pragma once

#include <QObject>
#include <QHash>
#include "NConnectPool.h"

enum EErrorCode
{
	EEC_SUCCESS,
	EEC_EXECFAIL,
	EEC_EMPTY
};

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
Q_DECLARE_METATYPE(SUser);

class NSqlDatabaseWrapper;

class NAccessDataBase : public QObject
{
	Q_OBJECT

public:
	NAccessDataBase(QObject *parent = nullptr);
	~NAccessDataBase();

public:
	bool openDB(QString strIP, int nPort, QString strDBName, QString strUserName, QString strPassword);
	void closeDB();

	// 日志记录入库
	EErrorCode addSchedulerLog(QString strMessage);
	EErrorCode getSchedulerLog(int nNum, QVector<QString> & vecStrMessage);

	// 姓名是唯一索引，不能添加重复名称的用户
	EErrorCode getUser(QString strUserName, SUser& user);
	EErrorCode getUserList(QVector<SUser> & vecUser);
	EErrorCode addUser(SUser & user);
	EErrorCode addAndUpdateUser(SUser & user);
	EErrorCode updateUser(QString strUserName, SUser & user);
	EErrorCode delUser(QString strUserName);

private:
	// 获取
	EErrorCode getNewID(long long& nNewID, NSqlDatabaseWrapper warpper);

private:
	NConnectPool m_connectPool;
};
