#pragma once
#include <QByteArray>

struct SPatient
{
	int m_nAge;
	int m_nHeight;
	int m_nSex;
	double m_dWeight;
	QString m_strPatientID;
	QString m_strPatientName;
};
struct SAESPatient
{
	QByteArray m_strAge;
	QByteArray m_strHeight;
	QByteArray m_strSex;
	QByteArray m_strWeight;
	QByteArray m_strPatientID;
	QByteArray m_strPatientName;
};
class NAESEncryption
{
public:
	static QByteArray aesEncrypt(const QByteArray& data);
	static QString aesDecrypt(const QByteArray& data);

public:
	static void aesEncrypt(const SPatient & patient, SAESPatient & aesPatient);
	static void aesDecrypt(const SAESPatient & aesPatient, SPatient & patient);

private:
	// 出于是单机系统的原因，系统有理由能查看全部数据，所以密钥需要恒定不变
	static QByteArray m_key;
};