#include <QString>
#include <QMetaType>

// 报警码
enum EErrorCode
{
	EEC_SUCCESS,
	EEC_FAIL
};

// 核心事件
enum ECoreEvent
{
	ECE_NONE,				  // 无指令
	ECE_CREATEWORKING,		  // 创建作业单
	ECE_DEVICEFINISHORBIT,	  // 设备完成轨道
	ECE_PLANJOIN			  // 计划加入
};
// 事件响应计划
enum EEventResponse
{
	EER_IMMEDIATELY, // 立即响应
	EER_NEXTORBIT,	 // 下一轨道响应
	EER_NEXTPLANE	 // 下一作业面响应
};
// 事件消息体
struct SCoreMessage
{
	ECoreEvent m_coreEvent;								// 核心事件
	EEventResponse m_eventResponse; 					// 事件响应计划
	long long m_nCreateTime;							// 生成时间
	QString m_strDeviceName;							// 设备名
	SCoreMessage()
	{
		m_eventResponse = EER_IMMEDIATELY;
		m_nCreateTime = 0;
	}
};
Q_DECLARE_METATYPE(SCoreMessage);

class NWorkingManager
{
public:
	EErrorCode eventCreateWorking(QString m_strDeviceName)
	{
		return EEC_SUCCESS;
	}

	EErrorCode eventPlanJoin(QString m_strDeviceName)
	{
		return EEC_SUCCESS;
	}
};