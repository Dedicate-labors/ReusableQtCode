#pragma once

#include <QObject>
#include "JQHttpServer"

class NRestAPIHandle;
class NRestAPIService : public QObject
{
	Q_OBJECT

public:
	NRestAPIService(QObject *parent);
	~NRestAPIService();

public:
	// 初始化RestAPI服务
	void initRestAPIService(int nPort);
	bool startService();

private:
	void setupUI();
	void setupConnect();
	void onHttpAccepted(const QPointer< JQHttpServer::Session > &session);

private:
	int m_nPort;
	// 注意jqnet使用的是线程池，所以m_pRestAPIHandle要注意线程安全
	NRestAPIHandle* m_pRestAPIHandle;
	JQHttpServer::TcpServerManage  m_restService;
};
