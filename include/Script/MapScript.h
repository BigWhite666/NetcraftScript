#pragma once

#include <QObject>
#include <QThread>
#include <QList>
#include <windows.h>
#include "dm/dmutils.h"
#include "Util/CharacterHelper.h"
#include "Util/GameWindow.h"

class MapScript : public QObject {
    Q_OBJECT
    
public:
    explicit MapScript(QObject* parent = nullptr);
    ~MapScript();

    void start(float targetX, float targetY, float targetZ);
    void stop();
    bool isRunning() const { return m_isRunning; }

signals:
    void started();
    void stopped();
    void messageUpdated(const QString& message);

private:
    void moveToPosition(const GameWindow& window, const Vector3& target);
    Vector3 getPlayerPosition(const GameWindow& window);
    bool m_isRunning;
    QList<QThread*> m_threads;
}; 