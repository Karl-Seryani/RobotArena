#ifndef LOGGER_H
#define LOGGER_H

#include <QPlainTextEdit>
#include <QString>

/**
 * @brief The Logger class records all the action done by the player. Primarily used for recovery and checking game actions
 * @author Group 17
 */
class Logger {
public:
    /// Set the text widget that will display log messages.
    static void setLogWidget(QPlainTextEdit* widget);
    /// Log a message (with a timestamp).
    static void log(const QString& message);
    
private:
    static QPlainTextEdit* logWidget;
};

#endif // LOGGER_H
