#include "NBuildMachineCode.h"
#include <QNetworkInterface>
#include <QCryptographicHash>
#define _WIN32_DCOM
#include <cstdlib>
#include <comdef.h>
#include <Wbemidl.h>
#include <QDebug>
#pragma comment(lib, "wbemuuid.lib")

NBuildMachineCode::NBuildMachineCode()
{
	setupUI();
	setupConnect();
}

QString NBuildMachineCode::GetMachineCode()
{
	QString strSourceCode = QStringLiteral("%1-%2").arg(m_strHDDiskCode).arg(m_strMACAddress);
	//qDebug() << "-----------" << strSourceCode;
	QString strHashCode = QCryptographicHash::hash(strSourceCode.toUtf8(), QCryptographicHash::Md5).toHex();
	return strHashCode.toUpper();
}

void NBuildMachineCode::setupUI()
{
	m_strHDDiskCode = retFirstDiskNumber();
	m_strMACAddress = retFirstMacAddress();
}

void NBuildMachineCode::setupConnect()
{

}


QString NBuildMachineCode::retFirstDiskNumber()
{
	HRESULT hres;
	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
		qDebug() << "Failed to initialize COM library. Error code = 0x"
			<< hex << hres;
		return "error"; // Program has failed.
	}
	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------
	// Note: If you are using Windows 2000, you need to specify -
	// the default authentication credentials for a user by using
	// a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
	// parameter of CoInitializeSecurity ------------------------
	hres = CoInitializeSecurity(
		NULL,
		-1, // COM authentication
		NULL, // Authentication services
		NULL, // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT, // Default authentication
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
		NULL, // Authentication info
		EOAC_NONE, // Additional capabilities
		NULL // Reserved
	);
	if (FAILED(hres))
	{
		qDebug() << "Failed to initialize security. Error code = 0x"
			<< hex << hres;
		CoUninitialize();
		return "error"; // Program has failed.
	}
	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------
	IWbemLocator* pLoc = NULL;
	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres))
	{
		qDebug() << "Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< hex << hres;
		CoUninitialize();
		return "error"; // Program has failed.
	}
	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method
	IWbemServices* pSvc = NULL;
	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL, // User name. NULL = current user
		NULL, // User password. NULL = current
		0, // Locale. NULL indicates current
		NULL, // Security flags.
		0, // Authority (e.g. Kerberos)
		0, // Context object
		&pSvc // pointer to IWbemServices proxy
	);
	if (FAILED(hres))
	{
		qDebug() << "Could not connect. Error code = 0x"
			<< hex << hres;
		pLoc->Release();
		CoUninitialize();
		return "error"; // Program has failed.
	}
	//qDebug << "Connected to ROOT\\CIMV2 WMI namespace";
	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------
	hres = CoSetProxyBlanket(
		pSvc, // Indicates the proxy to set
		RPC_C_AUTHN_WINNT, // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE, // RPC_C_AUTHZ_xxx
		NULL, // Server principal name
		RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL, // client identity
		EOAC_NONE // proxy capabilities
	);
	if (FAILED(hres))
	{
		qDebug() << "Could not set proxy blanket. Error code = 0x"
			<< hex << hres;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "error"; // Program has failed.
	}
	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----
	// For example, get the name of the operating system
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_PhysicalMedia"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);
	if (FAILED(hres))
	{
		qDebug() << "Query for physical media failed."
			<< " Error code = 0x"
			<< hex << hres;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return "error"; // Program has failed.
	}
	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------
	IWbemClassObject* pclsObj;
	ULONG uReturn = 0;
	QString diskNumber;
	char buffer[1024] = { 0 };
	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);
		if (0 == uReturn)
		{
			break;
		}
		VARIANT vtProp;
		// Get the value of the Name property
		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		// 这里就获取了wchar_t的的硬盘序列号
		//        wcout << "Serial Number : " << vtProp.bstrVal << endl;
		memset(buffer, 0, 1024);
		wcstombs(buffer, vtProp.bstrVal, 1024);
		diskNumber = buffer;
		VariantClear(&vtProp);
		// 不退出就循环获取所有硬盘
		break;
	}
	// Cleanup
	// ========
	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	pclsObj->Release();
	CoUninitialize();
	// Program successfully completed.
	return diskNumber.simplified();
}

QString NBuildMachineCode::retFirstMacAddress()
{
	QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
	//获取所有网络接口的列表, 这个是Qt框架的foreach循环
	for (QNetworkInterface itemInterface : interfaceList)
	{   
		return itemInterface.hardwareAddress();
	}
	return QStringLiteral("");
}

