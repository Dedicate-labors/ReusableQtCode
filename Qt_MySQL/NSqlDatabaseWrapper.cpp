#include "NSqlDatabaseWrapper.h"

NSqlDatabaseWrapper::NSqlDatabaseWrapper(NConnectPool& connectPool) :
	m_connectPool(connectPool)
{
	m_sqlDatabase = m_connectPool.getConnect();
}

NSqlDatabaseWrapper::~NSqlDatabaseWrapper()
{
	m_connectPool.releaseConnect(m_sqlDatabase);
}

NSqlDatabaseWrapper::operator QSqlDatabase()
{
	return m_sqlDatabase;
}