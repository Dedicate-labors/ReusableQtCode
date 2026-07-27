// 本类重点在发送心跳以及尝试重连的思路，故记录
// 1. 心跳每隔50ms进行发送一次；
// 2. 每发送60次心跳就进行一次重连检测，判断是否重连；

#pragma once
#include <QThread>
#include <chrono>
#include "snap7.h"


class PostToPLCData : public QThread
{
	Q_OBJECT

public:
	PostToPLCData(QObject *parent = nullptr);
	virtual ~PostToPLCData();

public:
	void setS7Info(QString strS7IP, int nRackNum, int nSlotNum, int nDBnum);

protected:
	virtual bool v_init();
	virtual void v_release();
	virtual bool v_start();
	virtual void v_stop();
	virtual void v_recvFromPre(SPluginData* pPluginData);
	virtual void run();

private:
	void updateWatchdog(uint8_t& watchdogValue);
	void checkHeartbeatAndReconnect(uint8_t triggerInterval, uint8_t triggerCount, uint8_t& nPreServerHeartbeat);

private:
	bool connectToPLC();

	// 写入WatchDog变量 (Byte类型，偏移地址1.0)
	bool writeWatchDog(uint8_t value);

	// 读取服务端发送的看门狗
	uint8_t readServerHeartbeat();

	// 写入RedAreaErrorCode变量 (int16类型，偏移地址2.0)
	bool writeRedAreaErrorCode(int16_t value);

	// 写入PartX_intrude变量 (Bool类型，偏移地址4.0)
	bool writePartIntrude(int bitPosition, bool value);

	// 获取最后一次错误信息
	std::string getLastError();

private:
	volatile bool m_bIsTerminaled;

	TS7Client m_S7client;

	QString m_strS7IP;
	int m_nRackNum;
	int m_nSlotNum;
	int m_nDBNum;
	int m_nByteOffset;
	bool m_bIsConnected;

private:
	// 本次红区检测故障码
	// 0正常 其它异常
	int16_t m_nReadAreaErrorCode;
};
