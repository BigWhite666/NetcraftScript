#pragma once

#include <QObject>
#include <QTextEdit>
#include <QtMessageHandler>

class MessageHandler : public QObject {
    Q_OBJECT

public:
    static void install(QTextEdit* output);
    static void uninstall();

private:
    static void messageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    static QTextEdit* s_output;
    static QtMessageHandler s_previousHandler;
}; 