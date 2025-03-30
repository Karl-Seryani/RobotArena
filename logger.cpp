#include "logger.h"
#include <QDateTime>
#include <QDebug>

QPlainTextEdit* Logger::logWidget = nullptr;

void Logger::setLogWidget(QPlainTextEdit* widget) {
    logWidget = widget;
}

void Logger::log(const QString& message) {
    QString timeStamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logMessage = QString("[%1] %2").arg(timeStamp, message);
    if (logWidget) {
        logWidget->appendPlainText(logMessage);
    }
    // Also output to the debug console.
    qDebug() << logMessage;
}
