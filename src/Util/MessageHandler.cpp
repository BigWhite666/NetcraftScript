#include "Util/MessageHandler.h"
#include <QDateTime>
#include <QColor>

QTextEdit* MessageHandler::s_output = nullptr;
QtMessageHandler MessageHandler::s_previousHandler = nullptr;

void MessageHandler::install(QTextEdit* output) {
    s_output = output;
    s_previousHandler = qInstallMessageHandler(messageOutput);
}

void MessageHandler::uninstall() {
    qInstallMessageHandler(s_previousHandler);
    s_output = nullptr;
}

void MessageHandler::messageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if (!s_output) return;

    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString text;
    QColor color;

    switch (type) {
        case QtDebugMsg:
            text = QString("[%1] %2").arg(time).arg(msg);
            color = QColor(0, 0, 0);  // 黑色
            break;
        case QtWarningMsg:
            text = QString("[%1] Warning: %2").arg(time).arg(msg);
            color = QColor(255, 165, 0);  // 橙色
            break;
        case QtCriticalMsg:
            text = QString("[%1] Critical: %2").arg(time).arg(msg);
            color = QColor(255, 0, 0);  // 红色
            break;
        case QtFatalMsg:
            text = QString("[%1] Fatal: %2").arg(time).arg(msg);
            color = QColor(139, 0, 0);  // 深红色
            break;
        default:
            text = QString("[%1] %2").arg(time).arg(msg);
            color = QColor(0, 0, 0);
    }

    // 在主线程中更新 UI
    QMetaObject::invokeMethod(s_output, "append", Qt::QueuedConnection,
                             Q_ARG(QString, QString("<font color='%1'>%2</font>")
                                     .arg(color.name())
                                     .arg(text)));
} 