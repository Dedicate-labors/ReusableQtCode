#include "NRestAPIHandle.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QIODevice>
#include <QUuid>
#include "NSessionManger.h"

NRestAPIHandle::NRestAPIHandle(QObject *parent)
	: QObject(parent)
	, m_pSessionManager(NULL)
{
	setupUI();
	setupConnect();
}

NRestAPIHandle::~NRestAPIHandle()
{
}

void NRestAPIHandle::handle(const QPointer< JQHttpServer::Session > &session)
{
	SRestAPIUrlModule restAPIUrlModule;
	if (analysisURL(session->requestUrl(), restAPIUrlModule))
	{
		QJsonParseError jsonError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(session->requestBody(), &jsonError);
		if (QJsonParseError::NoError == jsonError.error)
		{
			handle(session, restAPIUrlModule, jsonDoc.object());
		}
		else
		{
			replyMessage(session, EEC_JSONERROR, "EEC_JSONERROR 对应错误字符串");
		}
	}
	else
	{
		replyMessage(session, EEC_URLERROR, "EEC_URLERROR 对应错误字符串");
	}
}

void NRestAPIHandle::setupUI()
{
	m_pSessionManager = new NSessionManger(this);
}

void NRestAPIHandle::setupConnect()
{

}

bool NRestAPIHandle::analysisURL(QString strURL, SRestAPIUrlModule& restAPIUrlModule)
{
	QStringList listURL = strURL.split(QStringLiteral("/"), QString::SkipEmptyParts);
	if (3 == listURL.count())
	{
		restAPIUrlModule.m_strPart = listURL[0];
		restAPIUrlModule.m_strVer = listURL[1];
		restAPIUrlModule.m_strFunction = listURL[2];
		return true;
	}
	else
	{
		qWarning(qPrintable(QStringLiteral("receive error restapi request: %1").arg(strURL)));
		return false;
	}
}

void NRestAPIHandle::replyMessage(const QPointer<JQHttpServer::Session> &session, EErrorCode errorCode, QString strMessage, const QJsonValue& jsonValue)
{
	QJsonObject jsonObject;
	jsonObject[QStringLiteral("code")] = QString::number(errorCode);
	jsonObject[QStringLiteral("message")] = strMessage;
	if (!jsonValue.isNull())
	{
		jsonObject[QStringLiteral("data")] = jsonValue;
	}

	QJsonDocument jsonDoc(jsonObject);
	session->replyText(jsonDoc.toJson());
}

void NRestAPIHandle::replyMessage(const QPointer<JQHttpServer::Session> &session, EErrorCode errorCode, QString strMessage, QString strSession, const QJsonValue& jsonValue)
{
	QJsonObject jsonObject;
	jsonObject[QStringLiteral("code")] = QString::number(errorCode);
	jsonObject[QStringLiteral("message")] = strMessage;
	jsonObject[QStringLiteral("session")] = strSession;

	if (!jsonValue.isNull())
	{
		jsonObject[QStringLiteral("data")] = jsonValue;
	}

	QJsonDocument jsonDoc(jsonObject);
    session->replyText(jsonDoc.toJson());
}

void NRestAPIHandle::handle(const QPointer< JQHttpServer::Session > &session, const SRestAPIUrlModule& restAPIUrlModule, const QJsonObject& jsonObject)
{
    if (QStringLiteral("addUserInfo") == restAPIUrlModule.m_strFunction)
    {
        return addUserInfo(session, jsonObject);
    }

	QString strSession = jsonObject[QStringLiteral("session")].toString();
	if (m_pSessionManager->checkSession(strSession))
	{
		if (QStringLiteral("login") == restAPIUrlModule.m_strFunction)
		{
			return login(session, jsonObject);
		}
		else if (QStringLiteral("otherfunction") == restAPIUrlModule.m_strFunction)
		{
			// 其他handle 函数
			return;
		}
		replyMessage(session, EEC_NOFUNCTION, "EEC_NOFUNCTION 对应错误字符串");
	}
	else
	{
		replyMessage(session, EEC_SESSIONINVALID, "EEC_SESSIONINVALID 对应错误字符串");
	}
}

void NRestAPIHandle::login(const QPointer<JQHttpServer::Session> &session, const QJsonObject& jsonObject)
{
	QString strUserName = jsonObject[QStringLiteral("username")].toString();
	QString strPassword = jsonObject[QStringLiteral("password")].toString();

	// SUser user;
	// EErrorCode errorCode = NGlobalVariable::m_gAccessDataBase.getUser(strUserName, user);
	// if (EEC_SUCCESS == errorCode)
	// {
	// 	if (user.m_strPassword == strPassword)
	// 	{
	// 		user.m_strSession = m_pSessionManager->createSession(user);

	// 		QJsonObject jsonReply;
	// 		NJsonConvert::toJson(user, jsonReply);

	// 		replyMessage(session, EEC_SUCCESS, NGlobalVariable::m_gErrorCode.getErrorCode(EEC_SUCCESS), user.m_strSession, jsonReply);
	// 	}
	// 	else
	// 	{
	// 		replyMessage(session, EEC_PASSWORDERROR, NGlobalVariable::m_gErrorCode.getErrorCode(EEC_PASSWORDERROR));
	// 	}
	// }
	// else
	// {
	// 	replyMessage(session, errorCode, NGlobalVariable::m_gErrorCode.getErrorCode(errorCode));
	// }
}

void NRestAPIHandle::writeJsonToFile(QString fileName, const QJsonObject & jsonObject)
{
	QFile jsonFile(fileName);
	if (jsonFile.open(QIODevice::WriteOnly))
	{
		QJsonDocument jdc(jsonObject);
		QByteArray jsonText = jdc.toJson();

		jsonFile.write(jsonText);
		jsonFile.close();
	}
}

bool NRestAPIHandle::readJsonFromFile(QString fileName, QJsonObject & jsonObject)
{
	QFile jsonFile(fileName);
	if (jsonFile.open(QIODevice::ReadOnly))
	{
		QByteArray jsonText = jsonFile.readAll();
		QJsonDocument jdc = QJsonDocument::fromJson(jsonText);
		if (!jdc.isNull())
		{
			if (jdc.isObject())
			{
				jsonObject = jdc.object();
				return true;
			}
		}
	}
	return false;
}
