#include "NViewLog.h"
#include <QDate>
#include <QTime>
#include <QTextStream>


QtMessageHandler NViewLog::defaultHandler = qInstallMessageHandler(nullptr);

NViewLog::NViewLog()
{
	checkLogFile();
}

NViewLog::~NViewLog()
{
	if (logFile.isOpen()) {
		logFile.close();
	}
}

NViewLog & NViewLog::getInstance()
{
	static NViewLog viewLogger;
	return viewLogger;
}

void NViewLog::log(QString &logContent)
{
	checkLogFile();

	// 写入文件（如果文件打开成功）
	if (logFile.isOpen()) {
		QTextStream out(&logFile);
		out << logContent;
		out.flush(); // 强制刷新到文件
	}
}

void NViewLog::checkLogFile()
{
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


void onViewLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString strMessage = QStringLiteral("");
	const char *file = context.file ? context.file : "";
	const char *function = context.function ? context.function : "";
	//QString realMsg = msg; 
	//realMsg = realMsg.replace('\n', "\\n").replace('\t', " ");

#define LOG_SPECIFICATION(msgType, msg, file, line, func) \
    QStringLiteral("[%1]%2:<<%3>> (%4:%5 | %6)\n") \
        .arg(QTime::currentTime().toString("HH:mm:ss")) \
		.arg(msgType) \
        .arg(msg) \
        .arg(file) \
        .arg(line) \
        .arg(func)

	switch (type) {
	case QtDebugMsg:
		strMessage = LOG_SPECIFICATION("DEBUG", msg, file, context.line, function);
		break;
	case QtInfoMsg:
		strMessage = LOG_SPECIFICATION("INFO", msg, file, context.line, function);
		break;
	case QtWarningMsg:
		strMessage = LOG_SPECIFICATION("WARNING", msg, file, context.line, function);
		break;
	case QtCriticalMsg:
		strMessage = LOG_SPECIFICATION("CRITICAL", msg, file, context.line, function);
		break;
	case QtFatalMsg:
		strMessage = LOG_SPECIFICATION("FATAL", msg, file, context.line, function);
		break;
	}

	NViewLog::defaultHandler(type, context, strMessage);
	NViewLog::getInstance().log(strMessage);
}
