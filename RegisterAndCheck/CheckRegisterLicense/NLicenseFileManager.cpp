#include "NLicenseFileManager.h"
#include "qaesencryption.h"
#include <QFile>
#include <QDate>
#include <QTextStream>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

NLicenseFileManager::NLicenseFileManager()
{
    m_strKey = "casit-jszx";
}

bool NLicenseFileManager::LoadLicenseConfig(QString strLicenseFile)
{
    QFile file(strLicenseFile);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream ts(&file);
        QString strLicenseCode = ts.readAll();
        if(!strLicenseCode.isEmpty())
        {
            QByteArray baDecodeData = QByteArray::fromHex(strLicenseCode.toUtf8());
            QAESEncryption encryption(QAESEncryption::AES_128, QAESEncryption::ECB, QAESEncryption::ZERO);
            QByteArray hashKey = QCryptographicHash::hash(m_strKey.toUtf8(), QCryptographicHash::Md5);

            QString strDecodeText = encryption.decode(baDecodeData, hashKey.toUpper());
            if(strDecodeText.length() > 9)
            {
                strDecodeText = strDecodeText.mid(9);
                m_license = String2License(strDecodeText);
                return true;
            }
        }
    }
    return false;
}

SLicense NLicenseFileManager::String2License(QString strJson)
{
    SLicense license;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8());
    if(jsonDoc.isObject())
    {
        QJsonObject jsonObject = jsonDoc.object();
        license.m_strDevideCode = jsonObject[QStringLiteral("DeviceCode")].toString();
        license.m_strCreateDate = jsonObject[QStringLiteral("CreateDate")].toString();
        license.m_strLicenseDate = jsonObject[QStringLiteral("LicenseDate")].toString();
        license.m_strUserInfo = jsonObject[QStringLiteral("UserInfo")].toString();
        license.m_nLicenseCount = jsonObject[QStringLiteral("LicenseCount")].toInt();
    }
    return license;
}

QString NLicenseFileManager::License2String(SLicense lic)
{
    QJsonObject jsonObject;
    jsonObject.insert(QStringLiteral("DeviceCode"), lic.m_strDevideCode);
    jsonObject.insert(QStringLiteral("CreateDate"), lic.m_strCreateDate);
    jsonObject.insert(QStringLiteral("LicenseDate"), lic.m_strLicenseDate);
    jsonObject.insert(QStringLiteral("UserInfo"), lic.m_strUserInfo);
    jsonObject.insert(QStringLiteral("LicenseCount"), lic.m_nLicenseCount);

    QJsonDocument jsonDoc(jsonObject);
    return jsonDoc.toJson();
}
