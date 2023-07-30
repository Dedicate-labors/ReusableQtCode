#include "NAESEncryption.h"
#include <QCryptographicHash>
#include "qaesencryption.h"
#include <QString>

QByteArray NAESEncryption::m_key = "casit-jszx";

QByteArray NAESEncryption::aesEncrypt(const QByteArray& data)
{
	QByteArray hash = QCryptographicHash::hash(m_key.toUpper(), QCryptographicHash::Sha256);
	QByteArray iv(hash.mid(0, 16));
	// QAESEncryption::ZERO 确保剩余Byte位填充为0
	QAESEncryption aes(QAESEncryption::AES_128, QAESEncryption::CBC, QAESEncryption::ZERO);
	QByteArray encrypted = aes.encode(data, hash, iv);
	return encrypted.toBase64();
}

QString NAESEncryption::aesDecrypt(const QByteArray& data)
{
	QByteArray hash = QCryptographicHash::hash(m_key.toUpper(), QCryptographicHash::Sha256);
	QByteArray iv(hash.mid(0, 16));
	QAESEncryption aes(QAESEncryption::AES_128, QAESEncryption::CBC, QAESEncryption::ZERO);
	QByteArray decrypted = aes.decode(QByteArray::fromBase64(data), hash, iv);
	QString retStr = decrypted;
	return retStr;
	// return retStr.mid(0, retStr.size() - 1);
}

void NAESEncryption::aesEncrypt(const SPatient & patient, SAESPatient & aesPatient)
{
	aesPatient.m_strAge = aesEncrypt(QString::number(patient.m_nAge).toUtf8());
	aesPatient.m_strHeight = aesEncrypt(QString::number(patient.m_nHeight).toUtf8());
	aesPatient.m_strSex = aesEncrypt(QString::number(patient.m_nSex).toUtf8());
	aesPatient.m_strWeight = aesEncrypt(QString::number(patient.m_dWeight, 'f', 3).toUtf8());
	aesPatient.m_strPatientID = aesEncrypt(patient.m_strPatientID.toUtf8());
	aesPatient.m_strPatientName = aesEncrypt(patient.m_strPatientName.toUtf8());
}

void NAESEncryption::aesDecrypt(const SAESPatient & aesPatient, SPatient & patient)
{
	patient.m_nAge = aesDecrypt(aesPatient.m_strAge).toInt();
	patient.m_nHeight = aesDecrypt(aesPatient.m_strHeight).toInt();
	patient.m_nSex = aesDecrypt(aesPatient.m_strSex).toInt();
	patient.m_dWeight = aesDecrypt(aesPatient.m_strWeight).toDouble();
	patient.m_strPatientID = aesDecrypt(aesPatient.m_strPatientID);
	patient.m_strPatientName = aesDecrypt(aesPatient.m_strPatientName);
}
