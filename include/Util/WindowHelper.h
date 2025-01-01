#pragma once

#include <QObject>
#include <windows.h>
#include "dm/dmutils.h"

class WindowHelper : public QObject {
    Q_OBJECT
    
public:
    static HWND findGameWindow();
    static HWND findLatestGameWindow();
    static bool bindWindow(HWND hwnd, QString& errorMsg, bool pressF1 = false);
    static void unbindWindow();
    static void centerWindow(HWND hwnd);
    static void pressF1AndCenter(HWND hwnd);
    static QString getCharacterName(HWND hwnd);

signals:
    void messageUpdated(const QString& message);

private:
    static bool checkDM();
    static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK enumLatestWindowProc(HWND hwnd, LPARAM lParam);
}; 