#pragma once

#include <string>
#include <windows.h>
#include <QString>

class NInstallService
{
public:
	NInstallService();
	virtual ~NInstallService();

public:
	void ServiceMainProc(int argc, char *argv[]);

private:
	void Install(std::string strFilePath, std::string pServiceName);
	// 在注册表项中加入重启功能
	bool SetServiceRecoveryOptions(std::string pServiceName);
	bool KillService(std::string pServiceName);
	void UnInstall(std::string pServiceName);
	bool RunService(std::string pServiceName);
	void ExecuteSubProcess();
	static bool checkRegister();

public:
	static void NormalStart(int argc, char *argv[]);
	static VOID WINAPI ServiceMain(DWORD dwArgc, LPSTR *lpszArgv);
	static VOID WINAPI ServiceHandler(DWORD fdwControl);

private:
	static SERVICE_STATUS_HANDLE   hServiceStatusHandle;
	static SERVICE_STATUS          ServiceStatus;

	static char   pServiceName[];
	static SERVICE_TABLE_ENTRYA	lpServiceStartTable[];
};