#include <QtWidgets/QApplication>
#include <iostream>
#include <QDate>
#include <QDir>
#include "NInstallService.h"

/** Window Service **/
SERVICE_STATUS_HANDLE   NInstallService::hServiceStatusHandle;
SERVICE_STATUS          NInstallService::ServiceStatus;
char   NInstallService::pServiceName[] = "AIAA_Service";
SERVICE_TABLE_ENTRYA	NInstallService::lpServiceStartTable[] =
{
	{ NInstallService::pServiceName, &NInstallService::ServiceMain },
	{ NULL, NULL }
};

NInstallService::NInstallService()
{
}


NInstallService::~NInstallService()
{
}

void NInstallService::ServiceMainProc(int argc, char *argv[])
{
	std::string strCommand;
	if (argc >= 2) strCommand = argv[1];

	char pModuleFile[256] = { 0 };
	GetModuleFileNameA(NULL, pModuleFile, 256);

	if (strCommand == "-i" || strCommand == "-I")
		// 注册系统服务并运行
		Install(pModuleFile, pServiceName);
	else if (strCommand == "-k" || strCommand == "-K")
		// 停止系统服务 
		KillService(pServiceName);
	else if (strCommand == "-u" || strCommand == "-U")
		// 删除系统服务
		UnInstall(pServiceName);
	else if (strCommand == "-s" || strCommand == "-S")
		// 运行系统服务  运行后不会直接启动程序，系统服务会不带参数的进行执行，进入ExecuteSubProcess()函数
		RunService(pServiceName);
	else if (strCommand == "-n" || strCommand == "-N")
		NormalStart(argc, argv);
	else if (strCommand == "-c" || strCommand == "-C")
		std::cout << (checkRegister() ? "注册软件成功" : "未注册软件") << std::endl;
	else if (strCommand == "-h" || strCommand == "-H")
	{
		std::cout << "==== aiaaGrab command ====" << std::endl;
		std::cout << "-i 注册系统服务" << std::endl;
		std::cout << "-k 停止系统服务" << std::endl;
		std::cout << "-u 删除系统服务" << std::endl;
		std::cout << "-s 运行系统服务" << std::endl;
	}
	else
	{
		std::cout << "系统服务启动！" << std::endl;
		ExecuteSubProcess();
	}
}

void NInstallService::Install(std::string pPath, std::string pName)
{
	SC_HANDLE schSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (schSCManager == 0)
	{
		MessageBoxA(NULL, "打开系统服务管理器失败", "失败", MB_OK | MB_ICONERROR);
	}
	else
	{
		SC_HANDLE schService = CreateServiceA
		(
			schSCManager,	/* SCManager database      */
			pName.c_str(),			/* name of service         */
			pName.c_str(),			/* service name to display */
			SERVICE_ALL_ACCESS,        /* desired access          */
			SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, /* service type            */
			SERVICE_AUTO_START,      /* start type              */
			SERVICE_ERROR_NORMAL,      /* error control type      */
			pPath.c_str(),			/* service's binary        */
			NULL,                      /* no load ordering group  */
			NULL,                      /* no tag identifier       */
			NULL,                      /* no dependencies         */
			NULL,                      /* LocalSystem account     */
			NULL
		);                     /* no password             */
		if (schService == 0)
		{
			MessageBox(NULL, L"创建系统服务AIAA_Service失败", L"失败", MB_OK | MB_ICONERROR);
		}
		else
		{
			MessageBox(NULL, L"安装系统服务AIAA_Service成功", L"成功", MB_OK | MB_ICONINFORMATION);
		}
		CloseServiceHandle(schSCManager);
	}
}

void NInstallService::UnInstall(std::string pName)
{
	SC_HANDLE schSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == 0)
	{
		MessageBoxA(NULL, "打开系统服务管理器失败", "失败", MB_OK | MB_ICONERROR);
	}
	else
	{
		SC_HANDLE schService = OpenServiceA(schSCManager, pName.c_str(), SERVICE_ALL_ACCESS);
		if (schService == 0)
		{
			MessageBoxA(NULL, "系统服务AIAA_Service已经被删除", "提示", MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			if (!DeleteService(schService))
			{
				MessageBoxA(NULL, "删除服务AIAA_Service失败", "失败", MB_OK | MB_ICONERROR);
			}
			else
			{
				MessageBoxA(NULL, "删除服务AIAA_Service成功", "成功", MB_OK | MB_ICONINFORMATION);
			}
			CloseServiceHandle(schService);
		}
		CloseServiceHandle(schSCManager);
	}
}

bool NInstallService::KillService(std::string pName)
{
	// kill service with given name
	SC_HANDLE schSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == 0)
	{
		MessageBoxA(NULL, "打开系统服务管理器失败", "失败", MB_OK | MB_ICONERROR);
	}
	else
	{
		// open the service
		SC_HANDLE schService = OpenServiceA(schSCManager, pName.c_str(), SERVICE_ALL_ACCESS);
		if (schService == 0)
		{
			MessageBoxA(NULL, "打开系统服务AIAA_Service失败", "停止服务失败", MB_OK | MB_ICONERROR);
		}
		else
		{
			// call ControlService to kill the given service
			SERVICE_STATUS status;
			if (ControlService(schService, SERVICE_CONTROL_STOP, &status))
			{
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return true;
			}
			else
			{
				MessageBoxA(NULL, "停止系统服务AIAA_Service失败", "停止服务失败", MB_OK | MB_ICONERROR);
			}
			CloseServiceHandle(schService);
		}
		CloseServiceHandle(schSCManager);
	}
	return false;
}

bool NInstallService::RunService(std::string pName)
{
	// run service with given name
	SC_HANDLE schSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == 0)
	{
		MessageBoxA(NULL, "打开系统服务管理器失败", "失败", MB_OK | MB_ICONERROR);
	}
	else
	{
		// open the service
		SC_HANDLE schService = OpenServiceA(schSCManager, pName.c_str(), SERVICE_ALL_ACCESS);
		if (schService == 0)
		{
			MessageBoxA(NULL, "打开系统服务AIAA_Service失败", "运行服务失败", MB_OK | MB_ICONERROR);
		}
		else
		{
			// call StartService to run the service
			if (StartService(schService, 0, NULL))
			{
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return true;
			}
			else
			{
				MessageBoxA(NULL, "系统服务AIAA_Service已经在运行", "提示", MB_OK | MB_ICONERROR);
			}
			CloseServiceHandle(schService);
		}
		CloseServiceHandle(schSCManager);
	}
	return false;
}


void NInstallService::ExecuteSubProcess()
{
	if (!StartServiceCtrlDispatcherA(lpServiceStartTable))
	{
		MessageBoxA(NULL, "系统服务不能直接双击运行，请使用命令行或在控制面板中启动", "提示", MB_OK | MB_ICONERROR);
	}
}

void NInstallService::NormalStart(int argc, char *argv[])
{
	// license必须设置时间，不能设置为永久
	char pModuleFile[256] = { 0 };
	// 获取程序绝对路径
	GetModuleFileNameA(NULL, pModuleFile, 256);

	/**
	 * 这里有个大坑！！！！！！！！！
	 * 通过系统服务程序启动的程序，其运行相对路径会被修改，所以我们必须写下如下代码进行路径的重置，在进行后续的配置文件读取
	*/
	QString currentPath = QFileInfo(QStringLiteral("%1").arg(pModuleFile)).absoluteDir().absolutePath();
	QDir::setCurrent(currentPath);
	QApplication::addLibraryPath(QStringLiteral("%1/plugins").arg(currentPath));

	QApplication a(argc, argv);
	// 自己的代码
	a.exec();
}

VOID WINAPI NInstallService::ServiceMain(DWORD dwArgc, LPSTR *lpszArgv)
{
	DWORD   status = 0;
	DWORD   specificError = 0xfffffff;

	ServiceStatus.dwServiceType = SERVICE_WIN32;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	hServiceStatusHandle = RegisterServiceCtrlHandlerA(pServiceName, &NInstallService::ServiceHandler);
	if (hServiceStatusHandle == 0)
	{
		return;
	}

	// Initialization complete - report running status 
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;
	SetServiceStatus(hServiceStatusHandle, &ServiceStatus);

	NormalStart(dwArgc, lpszArgv);
}

VOID WINAPI NInstallService::ServiceHandler(DWORD fdwControl)
{
	switch (fdwControl)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwCheckPoint = 0;
		ServiceStatus.dwWaitHint = 0;
		// terminate all processes started by this service before shutdown

		break;
	case SERVICE_CONTROL_PAUSE:
		ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		break;
	};
	SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
}