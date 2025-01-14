#include "UI/Panels/HomePanel.h"
#include <QDebug>

void HomePanel::initPanel(QWidget* panel) {
    auto layout = new QVBoxLayout(panel);
    
    // 添加欢迎信息
    auto welcomeLabel = new QLabel("欢迎使用 NKScript");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 24px; color: #428BCA;");
    layout->addWidget(welcomeLabel);
    
    // 添加窗口列表
    auto listGroup = new QGroupBox("游戏窗口列表");
    auto listLayout = new QVBoxLayout(listGroup);
    
    auto windowList = new QListWidget();
    setupWindowList(windowList);
    listLayout->addWidget(windowList);
    
    // 添加刷新按钮
    auto refreshBtn = new QPushButton("刷新窗口列表");
    refreshBtn->setStyleSheet(Style::BUTTON_STYLE);
    QObject::connect(refreshBtn, &QPushButton::clicked, [windowList]() {
        updateWindowList(windowList);
    });
    listLayout->addWidget(refreshBtn);
    
    layout->addWidget(listGroup);
}

void HomePanel::setupWindowList(QListWidget* list) {
    list->setSelectionMode(QAbstractItemView::NoSelection);
    QObject::connect(list, &QListWidget::itemChanged, [](QListWidgetItem* item) {
        onWindowItemChanged(item);
    });
    updateWindowList(list);
}

void HomePanel::updateWindowList(QListWidget* list) {
    list->clear();
    refreshGameWindows();  // 刷新全局游戏窗口列表
    
    for (const auto& window : g_gameWindows) {
        QString text = QString("窗口句柄: %1 | 角色: %2 | 状态: %3")
            .arg((quintptr)window.hwnd)
            .arg(window.role.isEmpty() ? "未登录" : window.role)
            .arg(window.task.Taskname);
        
        auto item = new QListWidgetItem(text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(window.isChecked ? Qt::Checked : Qt::Unchecked);
        list->addItem(item);
    }
}

void HomePanel::onWindowItemChanged(QListWidgetItem* item) {
    if (!(item->flags() & Qt::ItemIsUserCheckable)) return;
    
    int index = item->listWidget()->row(item);
    if (index >= 0 && index < g_gameWindows.size()) {
        g_gameWindows[index].isChecked = (item->checkState() == Qt::Checked);
    }
} 