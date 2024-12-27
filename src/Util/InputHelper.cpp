#include "Util/InputHelper.h"
#include "main.h"
#include <QDebug>

bool InputHelper::checkDM() {
    if (!DM) {
        qDebug() << "大漠插件未初始化";
        return false;
    }
    return true;
}

bool InputHelper::moveTo(int x, int y) {
    if (!checkDM()) return false;
    try {
        return DM->MoveTo(x, y) == 1;
    } catch (const std::exception& e) {
        qDebug() << "移动鼠标失败：" << e.what();
        return false;
    }
}

bool InputHelper::click(bool rightClick) {
    if (!checkDM()) return false;
    try {
        return rightClick ? DM->RightClick() == 1 : DM->LeftClick() == 1;
    } catch (const std::exception& e) {
        qDebug() << "点击失败：" << e.what();
        return false;
    }
}

bool InputHelper::moveToAndClick(int x, int y, bool rightClick) {
    if (!moveTo(x, y)) return false;
    sleep(50);  // 等待鼠标移动完成
    return click(rightClick);
}

bool InputHelper::doubleClick(bool rightClick) {
    if (!checkDM()) return false;
    try {
        if (rightClick) {
            // 右键双击需要手动实现
            DM->RightClick();
            Sleep(50);
            DM->RightClick();
        } else {
            DM->LeftDoubleClick();
        }
        return true;
    } catch (const std::exception& e) {
        qDebug() << "双击失败：" << e.what();
        return false;
    }
}

bool InputHelper::drag(int fromX, int fromY, int toX, int toY) {
    if (!checkDM()) return false;
    try {
        if (!moveTo(fromX, fromY)) return false;
        sleep(50);
        
        DM->LeftDown();
        sleep(50);
        
        if (!moveTo(toX, toY)) {
            DM->LeftUp();
            return false;
        }
        
        sleep(50);
        DM->LeftUp();
        return true;
    } catch (const std::exception& e) {
        qDebug() << "拖拽失败：" << e.what();
        return false;
    }
}

bool InputHelper::keyPress(int keyCode) {
    if (!checkDM()) return false;
    try {
        return DM->KeyPress(keyCode) == 1;
    } catch (const std::exception& e) {
        qDebug() << "按键失败：" << e.what();
        return false;
    }
}

bool InputHelper::keyDown(int keyCode) {
    if (!checkDM()) return false;
    try {
        return DM->KeyDown(keyCode) == 1;
    } catch (const std::exception& e) {
        qDebug() << "按下按键失败：" << e.what();
        return false;
    }
}

bool InputHelper::keyUp(int keyCode) {
    if (!checkDM()) return false;
    try {
        return DM->KeyUp(keyCode) == 1;
    } catch (const std::exception& e) {
        qDebug() << "释放按键失败：" << e.what();
        return false;
    }
}

bool InputHelper::inputString(const QString& text) {
    if (!checkDM()) return false;
    try {
        return DM->SendString(0, text.toStdWString().c_str()) == 1;
    } catch (const std::exception& e) {
        qDebug() << "输入文本失败：" << e.what();
        return false;
    }
}

bool InputHelper::pressCtrlA() {
    if (!checkDM()) return false;
    try {
        DM->KeyDown(17);  // Ctrl
        sleep(50);
        DM->KeyPress(65);  // A
        sleep(50);
        DM->KeyUp(17);
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Ctrl+A失败：" << e.what();
        return false;
    }
}

bool InputHelper::pressCtrlV() {
    if (!checkDM()) return false;
    try {
        DM->KeyDown(17);  // Ctrl
        sleep(50);
        DM->KeyPress(86);  // V
        sleep(50);
        DM->KeyUp(17);
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Ctrl+V失败：" << e.what();
        return false;
    }
}

bool InputHelper::pressCtrlC() {
    if (!checkDM()) return false;
    try {
        DM->KeyDown(17);  // Ctrl
        sleep(50);
        DM->KeyPress(67);  // C
        sleep(50);
        DM->KeyUp(17);
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Ctrl+C失败：" << e.what();
        return false;
    }
}

bool InputHelper::pressAltTab() {
    if (!checkDM()) return false;
    try {
        DM->KeyDown(18);  // Alt
        sleep(50);
        DM->KeyPress(9);   // Tab
        sleep(50);
        DM->KeyUp(18);
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Alt+Tab失败：" << e.what();
        return false;
    }
}

QPoint InputHelper::getCurrentPos() {
    if (!checkDM()) return QPoint();
    try {
        VARIANT x, y;
        DM->GetCursorPos(&x, &y);
        return QPoint(x.intVal, y.intVal);
    } catch (const std::exception& e) {
        qDebug() << "获取鼠标位置失败：" << e.what();
        return QPoint();
    }
}

bool InputHelper::isKeyPressed(int keyCode) {
    if (!checkDM()) return false;
    try {
        return DM->GetKeyState(keyCode) == 1;
    } catch (const std::exception& e) {
        qDebug() << "获取按键状态失败：" << e.what();
        return false;
    }
}

void InputHelper::sleep(int ms) {
    Sleep(ms);
} 