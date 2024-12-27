#pragma once

#include <QObject>
#include <QPoint>
#include "dm/dmutils.h"

class InputHelper : public QObject {
    Q_OBJECT
    
public:
    static bool moveTo(int x, int y);
    static bool moveToAndClick(int x, int y, bool rightClick = false);
    static bool click(bool rightClick = false);
    static bool doubleClick(bool rightClick = false);
    static bool drag(int fromX, int fromY, int toX, int toY);
    
    // 键盘操作
    static bool keyPress(int keyCode);
    static bool keyDown(int keyCode);
    static bool keyUp(int keyCode);
    static bool inputString(const QString& text);
    
    // 组合键
    static bool pressCtrlA();
    static bool pressCtrlV();
    static bool pressCtrlC();
    static bool pressAltTab();
    
    // 实用功能
    static QPoint getCurrentPos();
    static bool isKeyPressed(int keyCode);
    static void sleep(int ms);

private:
    static bool checkDM();
}; 