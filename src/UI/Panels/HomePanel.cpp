#include "UI/Panels/HomePanel.h"

void HomePanel::initPanel(QWidget* panel) {
    auto layout = new QGridLayout(panel);
    
    // 状态组
    auto statusGroup = new QGroupBox("状态", panel);
    auto statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->addWidget(new QLabel("当前状态: 未运行"));
    statusLayout->addWidget(new QLabel("运行时间: 0:00:00"));
    
    // 控制组
    auto controlGroup = new QGroupBox("控制", panel);
    auto controlLayout = new QGridLayout(controlGroup);
    controlLayout->addWidget(new QPushButton("开始"), 0, 0);
    controlLayout->addWidget(new QPushButton("暂停"), 0, 1);
    controlLayout->addWidget(new QPushButton("停止"), 1, 0);
    controlLayout->addWidget(new QPushButton("设置"), 1, 1);
    
    layout->addWidget(statusGroup, 0, 0);
    layout->addWidget(controlGroup, 0, 1);
} 