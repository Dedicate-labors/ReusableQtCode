#ifndef NLICENSEFILEMANAGER_H
#define NLICENSEFILEMANAGER_H

#include <QString>

struct SLicense
{
	// 授权服务器机器码
	QString m_strDevideCode;
	// 授权文件生成日期
	QString m_strCreateDate;
	// 授权到期日期，如果为空，则表示无日期限制
	QString m_strLicenseDate;
	// 用户信息
	QString m_strUserInfo;
	// 授权路数
	int m_nLicenseCount;
	SLicense()
	{
		m_nLicenseCount = 0;
	}
};

class NLicenseFileManager
{
public:
    NLicenseFileManager();

public:
    bool LoadLicenseConfig(QString strLicenseFile = QStringLiteral("./license.info"));
    inline SLicense GetLicense(){return m_license;};

private:
    SLicense String2License(QString strJson);
    QString License2String(SLicense lic);

private:
    QString m_strKey;
    SLicense m_license;
};

#endif // NLICENSECONFIG_H
