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

    void run(const GameWindow& window, const Vector3& target);
    void stop();

signals:
    void messageUpdated(const QString& message);

private:
    bool m_isRunning;
}; 