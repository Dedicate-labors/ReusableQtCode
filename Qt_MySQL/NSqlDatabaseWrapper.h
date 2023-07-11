#pragma once

#include <QObject>
#include <QSqlDatabase>
#include "NConnectPool.h"

class NSqlDatabaseWrapper
{
public:
	NSqlDatabaseWrapper(NConnectPool& connectPool);
	~NSqlDatabaseWrapper();

public:
	operator QSqlDatabase();

private:
	NConnectPool& m_connectPool;
	QSqlDatabase m_sqlDatabase;
};
