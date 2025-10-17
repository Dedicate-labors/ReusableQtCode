#pragma once
#include <QFile>
#include <QDebug>

class NViewLog
{
public:
	static NViewLog & getInstance();
	~NViewLog();

	NViewLog(const NViewLog &) = delete;
	NViewLog & operator=(const NViewLog &) = delete;

private:
	NViewLog();

public:
	// 日志记录函数
	void log(QString &logContent);
	// 默认的QDebug输出方法，相比std::cout更稳定
	static QtMessageHandler defaultHandler;

private:
	// 检查并切换日志文件（按日期）
	void checkLogFile();

private:
	QFile logFile;
	QString currentFileName;
};

void onViewLog(QtMsgType type, const QMessageLogContext &context, const QString &msg);
