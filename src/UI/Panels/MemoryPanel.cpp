#include "UI/Panels/MemoryPanel.h"
#include "UI/Style/StyleSheet.h"
#include <QDebug>

void MemoryPanel::initPanel(QWidget* panel, MemoryScript* memoryScript) {
    auto layout = new QVBoxLayout(panel);
    
    // 内存功能组
    auto memoryGroup = new QGroupBox("内存功能", panel);
    auto memoryLayout = new QGridLayout(memoryGroup);
    
    // 添加功能复选框
    int row = 0;
    
    // 秒矿功能
    auto miningCheck = new QCheckBox("秒矿");
    memoryLayout->addWidget(miningCheck, row++, 0);
    
    // 秒矿间隔
    auto miningIntervalCheck = new QCheckBox("秒矿间隔");
    memoryLayout->addWidget(miningIntervalCheck, row++, 0);
    
    // 水下呼吸
    auto breathCheck = new QCheckBox("水下呼吸");
    memoryLayout->addWidget(breathCheck, row++, 0);
    
    // 死亡秒活
    auto reviveCheck = new QCheckBox("死亡秒活");
    memoryLayout->addWidget(reviveCheck, row++, 0);
    
    // 层级透视
    auto wallhackCheck = new QCheckBox("层级透视");
    memoryLayout->addWidget(wallhackCheck, row++, 0);
    
    // 传送秒进
    auto teleportCheck = new QCheckBox("传送秒进");
    memoryLayout->addWidget(teleportCheck, row++, 0);
    
    // 添加一些空间
    memoryLayout->setRowStretch(row, 1);
    
    layout->addWidget(memoryGroup);
    
    // 连接信号
    connectSignals(panel, memoryScript);
}

void MemoryPanel::connectSignals(QWidget* panel, MemoryScript* memoryScript) {
    // 获取所有复选框
    auto checkBoxes = panel->findChildren<QCheckBox*>();
    
    for (auto checkBox : checkBoxes) {
        if (checkBox->text() == "秒矿") {
            QObject::connect(checkBox, &QCheckBox::toggled, panel, [memoryScript](bool checked) {
                qDebug() << "秒矿:" << (checked ? "开启" : "关闭");
                memoryScript->setMining(checked, 0);
            });
        }
        else if (checkBox->text() == "秒矿间隔") {
            QObject::connect(checkBox, &QCheckBox::toggled, panel, [memoryScript](bool checked) {
                qDebug() << "秒矿间隔:" << (checked ? "开启" : "关闭");
                memoryScript->setMiningInterval(checked);
            });
        }
        else if (checkBox->text() == "水下呼吸") {
            QObject::connect(checkBox, &QCheckBox::toggled, panel, [memoryScript](bool checked) {
                qDebug() << "水下呼吸:" << (checked ? "开启" : "关闭");
                memoryScript->setBreathing(checked);
            });
        }
        else if (checkBox->text() == "死亡秒活") {
            QObject::connect(checkBox, &QCheckBox::toggled, panel, [memoryScript](bool checked) {
                qDebug() << "死亡秒活:" << (checked ? "开启" : "关闭");
                memoryScript->setRevive(checked);
            });
        }
        else if (checkBox->text() == "层级透视") {
            QObject::connect(checkBox, &QCheckBox::toggled, panel, [memoryScript](bool checked) {
                qDebug() << "层级透视:" << (checked ? "开启" : "关闭");
                memoryScript->setWallhack(checked);
            });
        }
        else if (checkBox->text() == "传送秒进") {
            QObject::connect(checkBox, &QCheckBox::toggled, panel, [memoryScript](bool checked) {
                qDebug() << "传送秒进:" << (checked ? "开启" : "关闭");
                memoryScript->setTeleport(checked);
            });
        }
    }
} 