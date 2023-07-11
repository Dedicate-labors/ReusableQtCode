#include "NAccessDataBase.h"
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QDebug>
#include "NSqlDatabaseWrapper.h"

NAccessDataBase::NAccessDataBase(QObject *parent)
	: QObject(parent)
{
}

NAccessDataBase::~NAccessDataBase()
{
}

bool NAccessDataBase::openDB(QString strIP, int nPort, QString strDBName, QString strUserName, QString strPassword)
{
	return m_connectPool.initConnectPool(strIP, nPort, strDBName, strUserName, strPassword);
}

void NAccessDataBase::closeDB()
{
}

EErrorCode NAccessDataBase::addSchedulerLog(QString strMessage)
{
	QString strSQL = QStringLiteral("insert into t_schedulerlog(sl_message) value(?)");

	NSqlDatabaseWrapper warpper(m_connectPool);
	QSqlQuery sqlQuery(warpper);
	sqlQuery.prepare(strSQL);
	sqlQuery.bindValue(0, strMessage);

	if (sqlQuery.exec())
	{
		return EEC_SUCCESS;
	}
	else
	{
		QSqlError error = sqlQuery.lastError();
		qDebug() << error.databaseText();
	}
	return EEC_EXECFAIL;
}

EErrorCode NAccessDataBase::getSchedulerLog(int nNum, QVector<QString> &vecStrMessage)
{
	QString strSQL = QStringLiteral(
		"select sl_message from t_schedulerlog "
		"order by sl_id DESC limit ?");

	NSqlDatabaseWrapper warpper(m_connectPool);
	QSqlQuery sqlQuery(warpper);
	sqlQuery.prepare(strSQL);
	sqlQuery.bindValue(0, nNum);

	if (sqlQuery.exec())
	{
		while (sqlQuery.next())
		{
			QString strMessage = sqlQuery.value(QStringLiteral("sl_message")).toString();
			vecStrMessage.push_back(strMessage);
		}
		return EEC_SUCCESS;
	}
	else
	{
		QSqlError error = sqlQuery.lastError();
		qDebug() << error.databaseText();
	}

	return EEC_EXECFAIL;
}

EErrorCode NAccessDataBase::getNewID(long long &nNewID, NSqlDatabaseWrapper warpper)
{
	QSqlQuery sqlQuery(QStringLiteral("select LAST_INSERT_ID() as newid"), warpper);
	if (sqlQuery.next())
	{
		nNewID = sqlQuery.value(QStringLiteral("newid")).toLongLong();
		return EEC_SUCCESS;
	}
	else
	{
		QSqlError error = sqlQuery.lastError();
		qDebug() << error.databaseText();
	}
	return EEC_EMPTY;
}

EErrorCode NAccessDataBase::getUser(QString strUserName, SUser &user)
{
	QString strSQL = QStringLiteral("select * from t_user where u_username = ?");

	NSqlDatabaseWrapper warpper(m_connectPool);
	QSqlQuery sqlQuery(warpper);
	sqlQuery.prepare(strSQL);
	sqlQuery.bindValue(0, strUserName);
	if (sqlQuery.exec())
	{
		if (sqlQuery.next())
		{
			user.m_nID = sqlQuery.value(QStringLiteral("u_id")).toLongLong();
			user.m_strUserName = sqlQuery.value(QStringLiteral("u_username")).toString();
			user.m_strPassword = sqlQuery.value(QStringLiteral("u_password")).toString();
			user.m_nCreateTime = sqlQuery.value(QStringLiteral("u_createtime")).toULongLong();
			user.m_strShowName = sqlQuery.value(QStringLiteral("u_showname")).toString();
			user.m_strPosition = sqlQuery.value(QStringLiteral("u_position")).toString();
			user.m_strTel = sqlQuery.value(QStringLiteral("u_tel")).toString();
			user.m_strWorkerID = sqlQuery.value(QStringLiteral("u_workerid")).toString();
			return EEC_SUCCESS;
		}
		return EEC_EMPTY;
	}
	else
	{
		QSqlError error = sqlQuery.lastError();
		qDebug() << error.databaseText();
	}
	return EEC_EXECFAIL;
}

EErrorCode NAccessDataBase::getUserList(QVector<SUser> &vecUser)
{
	QString strSQL = QStringLiteral("select * from t_user");

	NSqlDatabaseWrapper warpper(m_connectPool);
	QSqlQuery sqlQuery(warpper);
	sqlQuery.prepare(strSQL);
	if (sqlQuery.exec())
	{
		while (sqlQuery.next())
		{
			SUser user;
			user.m_nID = sqlQuery.value(QStringLiteral("u_id")).toLongLong();
			user.m_strUserName = sqlQuery.value(QStringLiteral("u_username")).toString();
			user.m_strPassword = sqlQuery.value(QStringLiteral("u_password")).toString();
			user.m_nCreateTime = sqlQuery.value(QStringLiteral("u_createtime")).toULongLong();
			user.m_strShowName = sqlQuery.value(QStringLiteral("u_showname")).toString();
			user.m_strPosition = sqlQuery.value(QStringLiteral("u_position")).toString();
			user.m_strTel = sqlQuery.value(QStringLiteral("u_tel")).toString();
			user.m_strWorkerID = sqlQuery.value(QStringLiteral("u_workerid")).toString();
			vecUser.push_back(user);
		}
		return EEC_SUCCESS;
	}
	else
	{
		QSqlError error = sqlQuery.lastError();
		qDebug() << error.databaseText();
	}
	return EEC_EXECFAIL;
}

EErrorCode NAccessDataBase::addUser(SUser &user)
{
	QString strSQL = QStringLiteral("insert into t_user(u_username, u_password, u_createtime, u_showname, u_position, u_tel, u_workerid) value(?,?,?,?,?,?,?)");

	NSqlDatabaseWrapper warpper(m_connectPool);
	QSqlQuery sqlQuery(warpper);
	sqlQuery.prepare(strSQL);
	sqlQuery.bindValue(0, user.m_strUserName);
	sqlQuery.bindValue(1, user.m_strPassword);
	sqlQuery.bindValue(2, user.m_nCreateTime);
	sqlQuery.bindValue(3, user.m_strShowName);
	sqlQuery.bindValue(4, user.m_strPosition);
	sqlQuery.bindValue(5, user.m_strTel);
	sqlQuery.bindValue(6, user.m_strWorkerID);

	if (sqlQuery.exec())
	{
		long long nNewID = -1;
		EErrorCode nResult = getNewID(nNewID, warpper);
		if (EEC_SUCCESS == nResult)
		{
			user.m_nID = nNewID;
			return EEC_SUCCESS;
		}
		return nResult;
	}
	else
	{
		QSqlError error = sqlQuery.lastError();
		qDebug() << error.databaseText();
	}
	return EEC_EXECFAIL;
}

EErrorCode NAccessDataBase::addAndUpdateUser(SUser &user)
{
    QString strSQL = QStringLiteral(
		"insert into t_user(u_username, u_password, u_createtime, u_showname, u_position, u_tel, u_workerid) value(?,?,?,?,?,?,?) "
		"on duplicate key "
		"update u_username=?, u_password=?");

	NSqlDatabaseWrapper warpper(m_connectPool);
	QSqlQuery sqlQuery(warpper);
	sqlQuery.prepare(strSQL);
	sqlQuery.bindValue(0, user.m_strUserName);
	sqlQuery.bindValue(1, user.m_strPassword);
	sqlQuery.bindValue(2, user.m_nCreateTime);
	sqlQuery.bindValue(3, user.m_strShowName);
	sqlQuery.bindValue(4, user.m_strPosition);
	sqlQuery.bindValue(5, user.m_strTel);
	sqlQuery.bindValue(6, user.m_strWorkerID);
	sqlQuery.bindValue(7, user.m_strUserName);
	sqlQuery.bindValue(8, user.m_strPassword);
	if (sqlQuery.exec())
	{
		long long nNewID = -1;
		EErrorCode nResult = getNewID(nNewID, warpper);
		if (EEC_SUCCESS == nResult)
		{
			user.m_nID = nNewID;
			return EEC_SUCCESS;
		}
		return nResult;
	}
	else
	{
		QSqlError error = sqlQuery.lastError();
		qDebug() << error.databaseText();
	}
}

EErrorCode NAccessDataBase::updateUser(QString strUserName, SUser &user)
{
	QString strSQL = QStringLiteral("update t_user set u_password=?, u_createtime=?, u_showname=?, u_position=?, u_tel=?, u_workerid=? where u_username = '%1'").arg(user.m_strUserName, 1);

	NSqlDatabaseWrapper warpper(m_connectPool);
	QSqlQuery sqlQuery(warpper);
	sqlQuery.prepare(strSQL);
	sqlQuery.bindValue(0, user.m_strPassword);
	sqlQuery.bindValue(1, user.m_nCreateTime);
	sqlQuery.bindValue(2, user.m_strShowName);
	sqlQuery.bindValue(3, user.m_strPosition);
	sqlQuery.bindValue(4, user.m_strTel);
	sqlQuery.bindValue(5, user.m_strWorkerID);

	if (sqlQuery.exec())
	{
		return EEC_SUCCESS;
	}
	else
	{
		QSqlError error = sqlQuery.lastError();
		qDebug() << error.databaseText();
	}
	return EEC_EXECFAIL;
}

EErrorCode NAccessDataBase::delUser(QString strUserName)
{
	QString strSQL = QStringLiteral("delete from t_user where u_username = ?");

	NSqlDatabaseWrapper warpper(m_connectPool);
	QSqlQuery sqlQuery(warpper);
	sqlQuery.prepare(strSQL);
	sqlQuery.bindValue(0, strUserName);

	if (sqlQuery.exec())
	{
		return EEC_SUCCESS;
	}
	else
	{
		QSqlError error = sqlQuery.lastError();
		qDebug() << error.databaseText();
	}
	return EEC_EXECFAIL;
}
