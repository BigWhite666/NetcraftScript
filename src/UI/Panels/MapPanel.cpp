#include "UI/Panels/MapPanel.h"
#include "Util/GameWindow.h"
#include <QThread>
#include <QDebug>
#include "Util/WindowHelper.h"
#include "main.h"
#include <thread>

void Paotu(const GameWindow window, const Vector3 target) {
    try {
        QString errorMsg;
        if (!WindowHelper::bindWindow(window.hwnd, errorMsg, true)) {
            qDebug() << errorMsg;
            return;
        }

        qDebug() << QString("开始移动到目标位置 (%1, %2, %3)")
            .arg(target.x).arg(target.y).arg(target.z);

        while (true) {
            Vector3 current = CharacterHelper::getPosition(window.hwnd);
            float distance = CharacterHelper::getDistance(current, target);
            float heightDiff = target.z - current.z;

            // 调整高度
            if (heightDiff > 5.0f) {
                CharacterHelper::flyDown(false);
                CharacterHelper::flyUp(true);
            } else if (heightDiff < -5.0f) {
                CharacterHelper::flyUp(false);
                CharacterHelper::flyDown(true);
            } else {
                CharacterHelper::flyUp(false);
                CharacterHelper::flyDown(false);
            }

            // 移动
            CharacterHelper::moveForward(true);
            if (distance < 5.0f) {
                qDebug() << "已到达目标位置";
                break;
            }

            // 瞄准目标位置
            if (!CharacterHelper::aimTarget(window.hwnd, target)) {
                qDebug() << "瞄准目标失败";
                break;
            }

            Sleep(100);
        }

        // 停止所有移动
        CharacterHelper::moveForward(false);
        CharacterHelper::flyUp(false);
        CharacterHelper::flyDown(false);

        WindowHelper::unbindWindow();
    } catch (const std::exception &e) {
        qDebug() << QString("跑图失败：%1").arg(e.what());
    }
}

void MapPanel::initPanel(QWidget* panel) {
    auto layout = new QVBoxLayout(panel);
    
    // 脚本设置组
    auto scriptGroup = new QGroupBox("脚本设置", panel);
    auto scriptLayout = new QGridLayout(scriptGroup);
    
    // 添加X坐标输入框
    scriptLayout->addWidget(new QLabel("目标X坐标:"), 0, 0);
    auto targetXSpinBox = new QDoubleSpinBox();
    targetXSpinBox->setRange(-999999.0, 999999.0);
    targetXSpinBox->setDecimals(1);
    targetXSpinBox->setValue(5502.0);  // 设置默认值
    scriptLayout->addWidget(targetXSpinBox, 0, 1);
    
    // 添加Y坐标输入框
    scriptLayout->addWidget(new QLabel("目标Y坐标:"), 1, 0);
    auto targetYSpinBox = new QDoubleSpinBox();
    targetYSpinBox->setRange(-999999.0, 999999.0);
    targetYSpinBox->setDecimals(1);
    targetYSpinBox->setValue(953.0);  // 设置默认值
    scriptLayout->addWidget(targetYSpinBox, 1, 1);
    
    // 添加Z坐标输入框
    scriptLayout->addWidget(new QLabel("目标Z坐标:"), 2, 0);
    auto targetZSpinBox = new QDoubleSpinBox();
    targetZSpinBox->setRange(-999999.0, 999999.0);
    targetZSpinBox->setDecimals(1);
    targetZSpinBox->setValue(2.0);  // 设置默认值
    scriptLayout->addWidget(targetZSpinBox, 2, 1);
    
    // 操作按钮
    auto buttonLayout = new QHBoxLayout();
    auto startScriptBtn = new QPushButton("开始跑图");
    startScriptBtn->setStyleSheet(Style::BUTTON_STYLE);
    // auto stopScriptBtn = new QPushButton("停止");
    // stopScriptBtn->setStyleSheet(Style::BUTTON_STYLE);
    // stopScriptBtn->setEnabled(false);
    
    // 连接按钮点击事件
    QObject::connect(startScriptBtn, &QPushButton::clicked, [=]() {
        // 获取目标坐标
        float targetX = targetXSpinBox->value() * 10;
        float targetY = targetYSpinBox->value() * 10;
        float targetZ = targetZSpinBox->value() * 10-5;
        Vector3 target = {targetX, targetY, targetZ};

        // 获取选中的窗口
        QList<HWND> updatedWindows;
        for (const auto& window : g_gameWindows) {
            if (window.isChecked) {
                updatedWindows.append(window.hwnd);
            }
        }

        // 找到主窗口并更新状态
        if (!updatedWindows.isEmpty()) {
            QWidget* mainWindow = panel->window();
            if (auto* mainWin = qobject_cast<MainWindow*>(mainWindow)) {
                // 使用新的通用更新方法
                mainWin->updateGameWindows(updatedWindows, [](GameWindow* window) {
                    window->task.Taskname = "跑图中";
                });
            }
        }

        // 启动跑图线程
        for (const auto& hwnd : updatedWindows) {
            if (GameWindow* window = findGameWindowByHwnd(hwnd)) {
                std::thread followThread(Paotu, *window, target);
                followThread.detach();
            }
        }
    });

    //停止按钮点击事件
    // QObject::connect(stopScriptBtn, &QPushButton::clicked, [=]() {
    //     startScriptBtn->setEnabled(true);
    //     stopScriptBtn->setEnabled(false);
    // });
    
    buttonLayout->addWidget(startScriptBtn);
    // buttonLayout->addWidget(stopScriptBtn);
    
    layout->addWidget(scriptGroup);
    layout->addLayout(buttonLayout);
    layout->addStretch();
}
