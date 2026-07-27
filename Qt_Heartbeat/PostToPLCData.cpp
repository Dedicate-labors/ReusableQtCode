#include "PostToPLCData.h"
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>			   // Windows 下的 htons 头文件
#pragma comment(lib, "ws2_32.lib") // 链接 Windows 网络库（避免编译链接错误）
#else
#include <arpa/inet.h> // Linux/macOS 下的 htons 头文件
#endif


PostToPLCData::PostToPLCData(QObject *parent) : QThread(parent),
                                                NPluginObject(),
                                                m_bIsTerminaled(false),
                                                m_nRackNum(0),
                                                m_nSlotNum(0),
                                                m_nDBNum(0),
                                                m_nByteOffset(0),
                                                m_bIsConnected(false),
                                                m_nReadAreaErrorCode(0)
{
}

PostToPLCData::~PostToPLCData()
{
	m_bIsTerminaled = true;
	wait();
}

void PostToPLCData::setS7Info(QString strS7IP, int nRackNum, int nSlotNum, int nDBnum)
{
	m_strS7IP = strS7IP;
	m_nRackNum = nRackNum;
	m_nSlotNum = nSlotNum;
	m_nDBNum = nDBnum;
}

bool PostToPLCData::v_init()
{
	// 连接到PLC
	return connectToPLC();
}

void PostToPLCData::v_release()
{
	if (m_bIsConnected)
	{
		m_S7client.Disconnect();
		m_bIsConnected = false;
	}
}

bool PostToPLCData::v_start()
{
	m_bIsTerminaled = false;
	start();
	return true;
}

void PostToPLCData::v_stop()
{
	m_bIsTerminaled = true;
}

void PostToPLCData::v_recvFromPre(SPluginData *pPluginData)
{
    // 判断数据是否正确
    // 写逻辑
    // 发送数据到PLC writeRedAreaErrorCode 和 writePartIntrude
}

// 核心函数
void PostToPLCData::run()
{
	uint8_t watchdogValue = 0; // 初始值设为0
	const int intervalMs = 50; // 间隔时间50ms
	auto lastTime = std::chrono::steady_clock::now();

	// 服务端心跳检测频率设定“每60次触发一次”
	uint8_t triggerInterval = 60;
	// 记录当前服务端心跳检测累计次数（从0开始自增）
	uint8_t triggerCount = 0;
	// 初始化上次服务端发送的心跳值
	uint8_t nPreServerHeartbeat = readServerHeartbeat();

	while (!m_bIsTerminaled)
	{
		// 计算当前时间与上次执行的时间差(毫秒)
		auto currentTime = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration<double, std::milli>(currentTime - lastTime).count();

		// 如果时间差超过50ms，则执行操作
		if (elapsedMs >= intervalMs)
		{
            updateWatchdog(watchdogValue);
            checkHeartbeatAndReconnect(triggerInterval, triggerCount, nPreServerHeartbeat);
			// 更新上次执行时间
			lastTime = currentTime;
			// 更新服务端心跳检测频率进度
			triggerCount = (++triggerCount) % triggerInterval;
		}
		else
		{
			// 短暂休眠，减少CPU占用
			// 休眠时间为剩余时间，最少1ms
			int sleepMs = (intervalMs - elapsedMs) > 1 ? (intervalMs - elapsedMs) : 1;
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
		}
	}
}

void PostToPLCData::updateWatchdog(uint8_t &watchdogValue)
{
	if (writeWatchDog(watchdogValue))
    {
        watchdogValue++; // 溢出后自动回绕到0
    }
}

void PostToPLCData::checkHeartbeatAndReconnect(uint8_t triggerInterval, uint8_t triggerCount, uint8_t &nPreServerHeartbeat)
{
	if ((triggerInterval - 1) == triggerCount)
    {
        uint8_t nServerHeartBeat = readServerHeartbeat();
        if (nServerHeartBeat == nPreServerHeartbeat)
        {
            if (connectToPLC())
            {
				qWarning(qPrintable(QStringLiteral("Reconnect to PLC success, heartbeat: %1").arg(nServerHeartBeat)));
            }
            else
            {
				qWarning(qPrintable(QStringLiteral("Reonnect to PLC fail: %1").arg(getLastError().c_str())));
            }
        }
        nPreServerHeartbeat = nServerHeartBeat;
    }
}

bool PostToPLCData::connectToPLC()
{
	int nResult = m_S7client.ConnectTo(qPrintable(m_strS7IP), m_nRackNum, m_nSlotNum);
	if (0 == nResult)
	{
		m_bIsConnected = true;
		return true;
	}
	else
	{
		m_bIsConnected = false;
		return false;
	}
}

bool PostToPLCData::writeWatchDog(uint8_t value)
{
	if (!m_bIsConnected)
	{
		return false;
	}

	// 写入Byte类型到DB3320.DBX1.0
	// 参数说明：数据块号，起始地址，数据大小(字节)，数据指针
	int result = m_S7client.DBWrite(m_nDBNum, 1, 1, &value);
	return result == 0;
}

uint8_t PostToPLCData::readServerHeartbeat()
{
	if (!m_bIsConnected)
	{
		return false; // 未连接直接返回离线
	}

	// 保存读取的有效值
	static uint8_t vaildCurrentValue = 0;
	uint8_t currentValue;
	// 读取DB3320.DBX1.0（与写入地址对应），1个字节
	int result = m_S7client.DBRead(m_nDBNum, 1, 1, &currentValue);
	// qDebug() << "读取PLC心跳的值 " << result << " maybe currentvalue: " << currentValue;

	if (result != 0)
	{
		return vaildCurrentValue;
	}
	vaildCurrentValue = currentValue;
	return currentValue;
}

bool PostToPLCData::writeRedAreaErrorCode(int16_t value)
{
	if (!m_bIsConnected)
	{
		return false;
	}

	// 转换字节序以适应PLC的大端模式
	int16_t swappedValue = htons(value);

	// 写入int16类型到DB3320.DBW2.0
	// int16_t占用2个字节
	int result = m_S7client.DBWrite(m_nDBNum, 2, 2, &swappedValue);
	return result == 0;
}

bool PostToPLCData::writePartIntrude(int bitPosition, bool value)
{
	if (!m_bIsConnected)
	{
		return false;
	}

	// 检查位位置是否有效 (0-7对应4.0-4.7)
	if (bitPosition < 0 || bitPosition > 7)
	{
		return false;
	}

	// 读取字节值 (偏移地址4)
	uint8_t byteValue = 0;
	int result = m_S7client.DBRead(m_nDBNum, m_nByteOffset, 1, &byteValue);
	if (result != 0)
	{
		return false;
	}

	// 根据需要设置或清除指定的位
	if (value)
	{
		// 设置指定的位 (使用位或操作)
		byteValue |= (1 << bitPosition);
	}
	else
	{
		// 清除指定的位 (使用位与和取反操作)
		byteValue &= ~(1 << bitPosition);
	}

	// 写回修改后的字节值
	result = m_S7client.DBWrite(m_nDBNum, 4, 1, &byteValue);
	return result == 0;
}

std::string PostToPLCData::getLastError()
{
	std::stringstream ss;
	ss << "Error code: " << m_S7client.LastError()
		<< ", Message: " << CliErrorText(m_S7client.LastError());
	return ss.str();
}
