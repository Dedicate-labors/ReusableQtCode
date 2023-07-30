#pragma once
#include <QString>

class NBuildMachineCode
{
public:
	NBuildMachineCode();

public:
	QString GetMachineCode();

private:
	void setupUI();
	void setupConnect();
	
private:
	QString retFirstDiskNumber();
	QString retFirstMacAddress();
	
private:
	QString m_strHDDiskCode;
	QString m_strMACAddress;
};
