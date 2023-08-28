#ifndef NCOREBUSCONTROL_H
#define NCOREBUSCONTROL_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <functional>

class NCoreBusControl : public QObject
{
	Q_OBJECT

public:
	NCoreBusControl(QObject *parent = NULL);
	~NCoreBusControl();

	static NCoreBusControl & getInstance()
	{
		return m_gCoreBusControl;
	}

signals:
	void postExecBusBin(std::function<void(QVariant inputContext, QVariant& outContext)> fun, QVariant inputContext);

public:
	// 总线管理器:此管理器用于路由多个类之间的消息传递（函数调用），使用总线技术可以将模块之间的耦合性降到最低
	//			比如窗口类A需要发送一个消息到窗口类B，但这两个类之间无任何包含及依赖关系，此时可使用总线技术
	//			窗口类B先向总线注册一个消息处理函数，并标识此函数是用于接收序号为1的消息
	//			窗口类A直接向总线发送一个消息，并标识此消息序号为1，同时传递对应的参数
	//			所有类型序号为1的消息处理函数都会被总线调用一次
	// 使用说明: 0.程序启动后先对总线进行初始化
	//			1.总线消息接收者需要向总线注册消息接收函数，同时向总线传递需要接收的消息类型
	//			2.消息发送者可直接向总线发送消息，消息会被自动路由到每一个接收者
	//			3.消息发送者可选择使用同步发送消息或异步发送消息
	//			4.同步发送消息，接收者函数与发送者函数在同一线程里调用，调用完成后可返回结果
	//			5.异步发送消息，接收都函数不一定与发送者函数在同一线程，调用完成后不能得到结果

	// 初始化总线
	void InitBusControl();

	// 注册及删除接收者
	// nBusType:	消息类型
	// pOwner:		接收者
	// pFunRecv:	接收者处理函数（静态函数）
	// std::function<void(QVariant inputContext, QVariant& outContext)>如果用到信号与槽中需要注册 
	void RegRecvBin(int nBusType, std::function<void(QVariant inputContext, QVariant& outContext)> fun);
	void RemoveRecvBin(int nBusType);

	// 同步调用总线
	// pInputContext：需要传递给接收者的参数
	// pOutPutContext：返回最后一个被调用的收者的输出值
	bool ExecBusBin(int nBusType, QVariant inputContext, QVariant& outContext);

	// 异步调用总线 -- 只适合Qt信号与槽体系
	// pInputContext：需要传递给接收者的参数
	bool ExecBusBin(int nBusType, QVariant inputContext);

private slots:
	void actionExecBusBin(std::function<void(QVariant inputContext, QVariant& outContext)> fun, QVariant inputContext);

private:
	QMultiMap<int, std::function<void(QVariant inputContext, QVariant& outContext) >> m_mapBusControl;
	static NCoreBusControl m_gCoreBusControl;
};
#endif // NCOREBUSCONTROL_H
