#ifndef NSIMPLELOGGER_H
#define NSIMPLELOGGER_H

#include <sstream>
#include <QDate>
#include <QTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>

class SimpleLogger {
private:
	QFile logFile;
	QString currentFileName;

	// 检查并切换日志文件（按日期）
	void checkLogFile() {
		QString dateStr = QDate::currentDate().toString("yyyy-MM-dd");
		QString newFileName = dateStr + ".log";

		if (newFileName != currentFileName || !logFile.isOpen()) {
			if (logFile.isOpen()) {
				logFile.close();
			}
			currentFileName = newFileName;
			// 以追加模式打开文件，支持中文
			logFile.setFileName(currentFileName);
			logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
		}
	}

	// 递归终止函数
	static void streamHelper(std::stringstream&) {}

	// 递归拼接参数到stringstream
	template<typename T, typename... Args>
	static void streamHelper(std::stringstream& ss, T&& first, Args&&... rest) {
		ss << std::forward<T>(first);
		streamHelper(ss, std::forward<Args>(rest)...);
	}

public:
	SimpleLogger() {
		// 初始化时检查一次日志文件
		checkLogFile();
	}

	// 日志记录函数（同时输出到qDebug和文件）
	template<typename... Args>
	void log(Args&&... args) {
		checkLogFile();

		std::stringstream ss;
		// 添加时间前缀
		ss << "[" << QTime::currentTime().toString("HH:mm:ss").toStdString() << "] ";

		// 拼接所有参数
		streamHelper(ss, std::forward<Args>(args)...);
		ss << std::endl;

		// 转换为QString，确保中文正常显示
		QString logContent = QString::fromStdString(ss.str());

		// 1. 输出到qDebug
		// qDebug() << logContent;

		// 2. 写入文件（如果文件打开成功）
		if (logFile.isOpen()) {
			QTextStream out(&logFile);
			out << logContent;
			out.flush(); // 强制刷新到文件
		}
	}

	~SimpleLogger() {
		if (logFile.isOpen()) {
			logFile.close();
		}
	}
};

#endif // NSIMPLELOGGER_H
