#pragma once

#include <QObject>
#include <QJsonObject>
#include "JQHttpServer"

struct SRestAPIUrlModule
{
	QString m_strPart;
	QString m_strVer;
	QString m_strFunction;
};
Q_DECLARE_METATYPE(SRestAPIUrlModule);

enum EErrorCode
{
	EEC_JSONERROR,
	EEC_URLERROR,
	EEC_SUCCESS,
	EEC_SESSIONINVALID,
	EEC_NOFUNCTION
};

class NSessionManger;
class NRestAPIHandle : public QObject
{
	Q_OBJECT

public:
	NRestAPIHandle(QObject *parent);
	~NRestAPIHandle();

public:
	// 处理RestAPI的入口函数
	void handle(const QPointer<JQHttpServer::Session>& session);

private:
	void setupUI();
	void setupConnect();
	
	// 分析URL地址参数
	// true 地址有效，分析成功
	// false 地址无效，分析失败
	bool analysisURL(QString strURL, SRestAPIUrlModule& restAPIUrlModule);

	// 回复客户端消息
	// session 会话
	// 错误响应代码
	// 错误消息
	// 返回给客户端的数据
	void replyMessage(const QPointer<JQHttpServer::Session> &session, EErrorCode errorCode, QString strMessage, const QJsonValue& jsonValue = QJsonValue());
	void replyMessage(const QPointer<JQHttpServer::Session> &session, EErrorCode errorCode, QString strMessage, QString strSession, const QJsonValue& jsonValue = QJsonValue());

	// 处理系统模块会话
	void handle(const QPointer<JQHttpServer::Session> &session, const SRestAPIUrlModule& restAPIUrlModule, const QJsonObject& jsonObject);

	// 系统模块
	void login(const QPointer<JQHttpServer::Session> &session, const QJsonObject& jsonObject);

	void addUserInfo(const QPointer<JQHttpServer::Session> &session, const QJsonObject& jsonObject);

private:
    void writeJsonToFile(QString fileName, const QJsonObject & jsonObject);
	bool readJsonFromFile(QString fileName, QJsonObject & jsonObject);

private:
	NSessionManger* m_pSessionManager;
};
