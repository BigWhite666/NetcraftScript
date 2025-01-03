#include "UI/Panels/MapPanel.h"
#include "Util/GameWindow.h"
#include <QThread>
#include <QDebug>

void MapPanel::initPanel(QWidget* panel, MapScript* mapScript) {
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
    auto stopScriptBtn = new QPushButton("停止");
    stopScriptBtn->setStyleSheet(Style::BUTTON_STYLE);
    stopScriptBtn->setEnabled(false);
    
    // 连接按钮点击事件
    QObject::connect(startScriptBtn, &QPushButton::clicked, [=]() {
        // 获取目标坐标
        float targetX = targetXSpinBox->value();
        float targetY = targetYSpinBox->value();
        float targetZ = targetZSpinBox->value();
        Vector3 target = {targetX, targetY, targetZ};

        // 获取选中的窗口
        for (const auto& window : GameWindows::windows) {
            if (window.isChecked && window.task == "等待中") {
                // 为每个选中的窗口创建一个线程执行跑图任务
                QThread* thread = QThread::create([mapScript, window, target]() {
                    mapScript->run(window, target);
                });
                
                QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
                thread->start();
            }
        }
        
        startScriptBtn->setEnabled(false);
        stopScriptBtn->setEnabled(true);
    });
    
    QObject::connect(stopScriptBtn, &QPushButton::clicked, [=]() {
        mapScript->stop();
        startScriptBtn->setEnabled(true);
        stopScriptBtn->setEnabled(false);
    });
    
    // 连接消息更新信号
    QObject::connect(mapScript, &MapScript::messageUpdated, [](const QString& msg) {
        qDebug() << msg;
    });
    
    buttonLayout->addWidget(startScriptBtn);
    buttonLayout->addWidget(stopScriptBtn);
    
    layout->addWidget(scriptGroup);
    layout->addLayout(buttonLayout);
    layout->addStretch();
} 