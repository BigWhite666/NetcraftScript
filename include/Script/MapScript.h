#pragma once

#include <QObject>
#include <QThread>
#include <QList>
#include <windows.h>
#include "dm/dmutils.h"
#include "MemoryRead/GameOffsets.h"
#include <cmath>

// 跑图工作线程类
class MapWorker : public QObject {
    Q_OBJECT
    
public:
    explicit MapWorker(QObject* parent = nullptr);
    void setParams(const QString& mapName, int loopCount, const QList<HWND>& windows);
    void stop();

    void AimTarget(HWND hwnd, float targetX, float targetY, float targetZ);

public slots:
    void doWork();  // 执行跑图任务

signals:
    void messageUpdated(const QString& message);
    void progressUpdated(int current, int total);
    void finished();

private:
    QString m_mapName;
    int m_loopCount;
    QList<HWND> m_windows;
    bool m_running;
    
    // 跑图相关的私有方法
    bool enterMap(HWND hwnd);
    bool moveToNextPoint(HWND hwnd);
    bool collectItems(HWND hwnd);
    bool exitMap(HWND hwnd);
    bool bindWindow(HWND hwnd);
};

class MapScript : public QObject {
    Q_OBJECT
    
public:
    explicit MapScript(QObject* parent = nullptr);
    ~MapScript();

    void start(const QString& mapName, int loopCount, const QList<HWND>& windows);
    void stop();
    bool isRunning() const { return m_thread != nullptr; }

signals:
    void started();
    void stopped();
    void messageUpdated(const QString& message);
    void progressUpdated(int current, int total);

private:
    QThread* m_thread;
    MapWorker* m_worker;
    bool m_isRunning;
}; 