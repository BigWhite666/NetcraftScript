#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QFrame>
#include <QDebug>
#include <QApplication>
#include <windows.h>
#include <vector>
#include "main.h"

#include "Util/MessageHandler.h"
#include "dm/dmutils.h"
#include "Script/ChatScript.h"
#include "Script/MapScript.h"
#include "Script/DebugScript.h"
#include "Script/MemoryScript.h"
#include "UI/MainWindow.h"
#include "UI/Style/StyleSheet.h"
#include "UI/Panels/HomePanel.h"
#include "UI/Panels/LoginPanel.h"
#include "UI/Panels/MemoryPanel.h"
#include "Util/GameWindow.h"
#include "UI/Panels/MapPanel.h"

static std::vector<HWND> g_windows;
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    CHAR className[256];
    GetClassNameA(hwnd, className, sizeof(className));
    
    if (strcmp(className, "CIrrDeviceWin32") == 0) {
        g_windows.push_back(hwnd);
    }
    return TRUE;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_mapScript = new MapScript(this);
    m_debugScript = new DebugScript(this);
    m_memoryScript = new MemoryScript(this);
    setupUI();
    createPanels();
    refreshGameWindowsList();  // 初始化时获取一次窗口列表
}

MainWindow::~MainWindow() {
    // 卸载消息处理器
    MessageHandler::uninstall();
    
    if (m_mapScript) {
        m_mapScript->stop();
    }
}

void MainWindow::setupUI() {
    // 设置窗口基本属性
    setWindowTitle("NKScript");
    resize(800, 600);
    
    // 设置全局样式
    qApp->setStyle("Fusion");
    
    // 设置调色板
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(240, 245, 250));      // #F0F5FA
    palette.setColor(QPalette::WindowText, QColor(51, 51, 51));     // #333333
    palette.setColor(QPalette::Base, QColor(255, 255, 255));        // white
    palette.setColor(QPalette::AlternateBase, QColor(245, 249, 252));// #F5F9FC
    palette.setColor(QPalette::Button, QColor(66, 139, 202));       // #428BCA
    palette.setColor(QPalette::ButtonText, QColor(255, 255, 255));  // white
    palette.setColor(QPalette::Highlight, QColor(66, 139, 202));    // #428BCA
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255)); // white
    qApp->setPalette(palette);

    // 应用全局样式
    qApp->setStyleSheet(Style::MAIN_STYLE);
    
    // 创建中央部件
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主布局
    mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建顶部控制栏
    QHBoxLayout* topBarLayout = new QHBoxLayout();
    
    // 初始化成员变量
    refreshButton = new QPushButton("刷新窗口", this);
    refreshButton->setFixedWidth(100);
    refreshButton->setStyleSheet(Style::BUTTON_STYLE);
    topBarLayout->addWidget(refreshButton);
    
    // 添加调试测试按钮
    debugButton = new QPushButton("调试测试", this);
    debugButton->setFixedWidth(100);
    debugButton->setStyleSheet(Style::BUTTON_STYLE);
    topBarLayout->addWidget(debugButton);
    
    windowCountLabel = new QLabel("窗口个数:", this);
    topBarLayout->addWidget(windowCountLabel);
    
    windowCountValue = new QLabel("2", this);
    windowCountValue->setFixedWidth(30);
    topBarLayout->addWidget(windowCountValue);
    
    // 添加弹性空间
    topBarLayout->addStretch();
    
    // 将顶部控制栏添加到主布局
    mainLayout->addLayout(topBarLayout);
    
    // 创建列表
    listWidget = new QListWidget(this);
    listWidget->setMinimumHeight(250);
    listWidget->setMaximumHeight(250);
    listWidget->setAlternatingRowColors(true);
    listWidget->setStyleSheet(Style::LIST_STYLE);
    
    // 添加标题行
    QListWidgetItem* header = new QListWidgetItem();
    header->setFlags(Qt::NoItemFlags);
    
    // 创建标题行的widget
    QWidget* headerWidget = new QWidget();
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setSpacing(20);
    headerLayout->setContentsMargins(5, 2, 5, 2);
    
    // 添加全选复选框
    QCheckBox* selectAllBox = new QCheckBox(headerWidget);
    selectAllBox->setFixedWidth(20);
    selectAllBox->setStyleSheet(Style::CHECKBOX_STYLE);
    headerLayout->addWidget(selectAllBox);
    
    // 添加标题列，并设置字体加粗
    QFont headerFont;
    headerFont.setBold(true);
    
    auto addHeaderLabel = [&headerLayout, &headerFont](const QString& text, int stretch) {
        QLabel* label = new QLabel(text);
        label->setFont(headerFont);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);  // 左对齐
        label->setMinimumHeight(25);
        label->setStyleSheet("color: #428BCA; padding-left: 0px;");  // 移除左内边距
        headerLayout->addWidget(label, stretch);
    };
    
    addHeaderLabel("序号", 1);
    addHeaderLabel("句柄", 2);
    addHeaderLabel("启停按键", 2);
    addHeaderLabel("角色信息", 3);
    addHeaderLabel("当前任务", 2);
    
    // 连接全选复选框的信号
    connect(selectAllBox, &QCheckBox::stateChanged, this, [this](int state) {
        Qt::CheckState checkState = static_cast<Qt::CheckState>(state);
        
        // 从第1行开始遍历(跳过标题行)
        for (int i = 1; i < listWidget->count(); i++) {
            QListWidgetItem* item = listWidget->item(i);
            if (!item) continue;
            
            // 确保项目可以被选中
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(checkState);
            
            // 更新 GameWindow 中的状态
            QWidget* rowWidget = listWidget->itemWidget(item);
            if (!rowWidget) continue;
            
            QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(rowWidget->layout());
            if (!layout) continue;
            
            QLabel* handleLabel = qobject_cast<QLabel*>(layout->itemAt(1)->widget());
            if (!handleLabel) continue;
            
            bool ok;
            HWND hwnd = (HWND)handleLabel->text().toLongLong(&ok, 10);
            if (!ok) continue;
            
            if (GameWindow* window = GameWindows::findByHwnd(hwnd)) {
                window->isChecked = (checkState == Qt::Checked);
            }
        }
        
        // 触发状态改变事件
        for (int i = 1; i < listWidget->count(); i++) {
            if (QListWidgetItem* item = listWidget->item(i)) {
                onItemStateChanged(item);
            }
        }
    });
    
    headerWidget->setLayout(headerLayout);
    headerWidget->setFixedHeight(28);  // 减小标题行高度
    header->setSizeHint(headerWidget->sizeHint());
    listWidget->addItem(header);
    listWidget->setItemWidget(header, headerWidget);
    
    // 设置标题行背景色
    header->setBackground(QColor(236, 242, 247));  // #ECF2F7
    
    // 创建标签页
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(Style::TAB_STYLE);
    
    // 添加到主布局
    mainLayout->addWidget(listWidget);
    mainLayout->addWidget(tabWidget);

    // 连接信号槽
    connect(listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item->flags() & Qt::ItemIsUserCheckable) {
            Qt::CheckState newState = (item->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
            item->setCheckState(newState);
            onItemStateChanged(item);
        }
    });

    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshGameWindowsList);

    // 修改调试按钮的连接
    connect(debugButton, &QPushButton::clicked, this, [this]() {
        QList<HWND> selectedWindows;
        // 获取选中的窗口
        for (int i = 1; i < listWidget->count(); ++i) {
            QListWidgetItem* item = listWidget->item(i);
            if (item && item->checkState() == Qt::Checked) {
                QWidget* rowWidget = listWidget->itemWidget(item);
                if (rowWidget) {
                    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(rowWidget->layout());
                    if (layout) {
                        QLabel* handleLabel = qobject_cast<QLabel*>(layout->itemAt(1)->widget());
                        if (handleLabel) {
                            bool ok;
                            HWND hwnd = (HWND)handleLabel->text().toLongLong(&ok, 10);
                            if (ok) {
                                selectedWindows.append(hwnd);
                            }
                        }
                    }
                }
            }
        }
        
        if (selectedWindows.isEmpty()) {
            QMessageBox::warning(this, "警告", "请选择至少一个窗口！");
            return;
        }
        
        m_debugScript->start(selectedWindows);
    });
}

void MainWindow::createPanels() {
    // 创建标签页
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(Style::TAB_STYLE);
    
    // 创建各个面板
    homePanel = new QWidget();
    scriptPanel = new QWidget();
    chatPanel = new QWidget();
    auto memoryPanel = new QWidget();  // 添加内存面板
    auto loginPanel = new QWidget();   // 添加上号器面板
    
    // 初始化各个面板
    initHomePanel();
    MapPanel::initPanel(scriptPanel, m_mapScript);
    initChatPanel();
    MemoryPanel::initPanel(memoryPanel, m_memoryScript);  // 初始化内存面板
    LoginPanel::initPanel(loginPanel);  // 初始化上号器面板
    
    // 添加标签页
    tabWidget->addTab(homePanel, "主页");
    tabWidget->addTab(scriptPanel, "跑图");
    tabWidget->addTab(chatPanel, "喊话");
    tabWidget->addTab(memoryPanel, "内存功能");
    tabWidget->addTab(loginPanel, "上号器");  // 添加上号器标签页
    
    mainLayout->addWidget(tabWidget);
}

void MainWindow::initHomePanel() {
    auto layout = new QVBoxLayout(homePanel);  // 改为垂直布局
    
    // 上半部分 - 状态和使用说明
    auto upperWidget = new QWidget();
    auto upperLayout = new QGridLayout(upperWidget);
    
    // 左侧面板 - 状态和控制
    auto leftPanel = new QWidget();
    auto leftLayout = new QVBoxLayout(leftPanel);
    
    // 状态组
    auto statusGroup = new QGroupBox("状态", leftPanel);
    auto statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->addWidget(new QLabel("当前状态: 未运行"));
    statusLayout->addWidget(new QLabel("运行时间: 0:00:00"));
    leftLayout->addWidget(statusGroup);
    
    // 控制组
    auto controlGroup = new QGroupBox("控制", leftPanel);
    auto controlLayout = new QGridLayout(controlGroup);
    
    // 创建按钮并应用样式
    auto startBtn = new QPushButton("开始");
    startBtn->setStyleSheet(Style::BUTTON_STYLE);
    auto pauseBtn = new QPushButton("暂停");
    pauseBtn->setStyleSheet(Style::BUTTON_STYLE);
    auto stopBtn = new QPushButton("停止");
    stopBtn->setStyleSheet(Style::BUTTON_STYLE);
    auto settingBtn = new QPushButton("设置");
    settingBtn->setStyleSheet(Style::BUTTON_STYLE);
    
    controlLayout->addWidget(startBtn, 0, 0);
    controlLayout->addWidget(pauseBtn, 0, 1);
    controlLayout->addWidget(stopBtn, 1, 0);
    controlLayout->addWidget(settingBtn, 1, 1);
    leftLayout->addWidget(controlGroup);
    
    // 主页右侧面板 - 使用说明
    auto rightPanel = new QWidget();
    auto rightLayout = new QVBoxLayout(rightPanel);
    
    // 使用说明组
    auto helpGroup = new QGroupBox("使用说明", rightPanel);
    auto helpLayout = new QVBoxLayout(helpGroup);
    
    // 添加使用说明
    auto helpText = new QTextEdit();
    helpText->setReadOnly(true);
    helpText->setStyleSheet("QTextEdit { background-color: transparent; border: none; }");
    helpText->setText(
        "使用步骤：\n\n"
        "1. 点击顶部「刷新窗口」按钮获取游戏窗口\n\n"
        "2. 在窗口列表中勾选需要操作的窗口\n\n"
        "3. 选择功能标签页：\n"
        "   - 跑图：自动跑图功能\n"
        "   - 喊话：自动喊话功能\n"
        "   - 内存功能：各种辅助功能\n"
        "   - 上号器：账号管理与登录\n\n"
        "4. 在相应功能页面进行设置并启动\n\n"
        "注意事项：\n"
        "- 使用前请确保已选择正确的游戏窗口\n"
        "- 此程序需要管理员权限运行\n"
        "- 如遇问题请查看底部日志输出"
    );
    helpLayout->addWidget(helpText);
    
    rightLayout->addWidget(helpGroup);
    
    // 将左右面板添加到上半部分布局
    upperLayout->addWidget(leftPanel, 0, 0);
    upperLayout->addWidget(rightPanel, 0, 1);
    upperLayout->setColumnStretch(0, 1);
    upperLayout->setColumnStretch(1, 1);
    
    // 添加上半部分到主布局
    layout->addWidget(upperWidget);
    
    // 下半部分 - 日志输出
    auto logGroup = new QGroupBox("运行日志");
    auto logLayout = new QVBoxLayout(logGroup);
    
    auto logOutput = new QTextEdit();
    logOutput->setReadOnly(true);
    logOutput->setMaximumHeight(150);  // 限制日志窗口高度
    logLayout->addWidget(logOutput);
    
    // 添加日志组到主布局
    layout->addWidget(logGroup);
    
    // 安装消息处理器
    MessageHandler::install(logOutput);
}

void MainWindow::initChatPanel() {
    auto layout = new QVBoxLayout(chatPanel);
    
    // 喊话设置组
    auto chatGroup = new QGroupBox("喊话设置", chatPanel);
    auto chatLayout = new QGridLayout(chatGroup);
    
    chatLayout->addWidget(new QLabel("喊话内容:"), 0, 0);
    auto chatContentEdit = new QTextEdit();
    chatLayout->addWidget(chatContentEdit, 0, 1);
    
    chatLayout->addWidget(new QLabel("间隔(秒):"), 1, 0);
    auto intervalSpinBox = new QSpinBox();
    intervalSpinBox->setMinimum(1);
    intervalSpinBox->setMaximum(3600);
    intervalSpinBox->setValue(30);  // 默认30秒
    chatLayout->addWidget(intervalSpinBox, 1, 1);
    
    // 操作按钮
    auto buttonLayout = new QHBoxLayout();
    auto startChatBtn = new QPushButton("开始喊话");
    startChatBtn->setStyleSheet(Style::BUTTON_STYLE);
    auto stopChatBtn = new QPushButton("停止");
    stopChatBtn->setStyleSheet(Style::BUTTON_STYLE);
    stopChatBtn->setEnabled(false);
    
    // 创建喊话脚本实例
    m_chatScript = new ChatScript(this);
    
    // 连接按钮点击事件
    connect(startChatBtn, &QPushButton::clicked, this, [=]() {
        QString content = chatContentEdit->toPlainText();
        if (content.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入喊话内容！");
            return;
        }
        
        // 获取选中的窗口
        QList<HWND> selectedWindows;
        for (const auto& window : GameWindows::windows) {
            if (window.isChecked) {
                selectedWindows.append(window.hwnd);
            }
        }
        
        if (selectedWindows.isEmpty()) {
            QMessageBox::warning(this, "警告", "请选择至少一个窗口！");
            return;
        }
        
        // 更新UI状态
        startChatBtn->setEnabled(false);
        stopChatBtn->setEnabled(true);
        
        // 启动喊话脚本
        m_chatScript->start(selectedWindows, content, intervalSpinBox->value());
        
        // 保存选中的窗口列表，用于更新状态
        m_selectedWindows = selectedWindows;
    });
    
    connect(stopChatBtn, &QPushButton::clicked, this, [=]() {
        m_chatScript->stop();
        startChatBtn->setEnabled(true);
        stopChatBtn->setEnabled(false);
    });
    
    // 连接脚本信号
    connect(m_chatScript, &ChatScript::started, this, [this]() {
        updateTaskStatus(m_selectedWindows, "喊话中");
    });
    
    connect(m_chatScript, &ChatScript::stopped, this, [this]() {
        updateTaskStatus(m_selectedWindows, "等待中");
    });
    
    connect(m_chatScript, &ChatScript::messageUpdated, this, [](const QString& msg) {
        qDebug() << msg;
    });
    
    buttonLayout->addWidget(startChatBtn);
    buttonLayout->addWidget(stopChatBtn);
    
    layout->addWidget(chatGroup);
    layout->addLayout(buttonLayout);
    layout->addStretch();
}

void MainWindow::addCheckableItem(const QString& text) {
    QListWidgetItem* item = new QListWidgetItem(text);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);  // 移除 ItemIsSelectable
    item->setCheckState(Qt::Unchecked); // 默认未选中
    listWidget->addItem(item);
}

void MainWindow::onItemStateChanged(QListWidgetItem* item) {
    if (item->flags() & Qt::ItemIsUserCheckable) {
        bool checked = item->checkState() == Qt::Checked;
        
        // 获取窗口句柄
        QWidget* rowWidget = listWidget->itemWidget(item);
        if (!rowWidget) return;
        
        QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(rowWidget->layout());
        if (!layout) return;
        
        QLabel* handleLabel = qobject_cast<QLabel*>(layout->itemAt(1)->widget());
        if (!handleLabel) return;
        
        bool ok;
        HWND hwnd = (HWND)handleLabel->text().toLongLong(&ok, 10);
        if (!ok) return;
        
        // 更新 GameWindow 结构体中的选中状态
        if (GameWindow* window = GameWindows::findByHwnd(hwnd)) {
            window->isChecked = checked;
        }
    }
}

void MainWindow::addTableRow(const QString& index, const QString& handle, 
                           const QString& hotkey, const QString& role, 
                           const QString& task) {
    QListWidgetItem* item = new QListWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    item->setCheckState(Qt::Unchecked);
    
    QWidget* rowWidget = new QWidget();
    QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setSpacing(20);
    rowLayout->setContentsMargins(5, 2, 5, 2);  // 减小左边距
    
    // 添加各列数据，并设置垂直居中
    auto addLabel = [&rowLayout](const QString& text, int stretch) {
        QLabel* label = new QLabel(text);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        label->setMinimumHeight(20);
        label->setStyleSheet("padding: 0px; margin-left: 20px;");  // 给标签添加左边距
        rowLayout->addWidget(label, stretch);
    };
    
    addLabel(index, 1);
    addLabel(handle, 2);
    addLabel(hotkey, 2);
    addLabel(role, 3);
    addLabel(task, 2);
    
    rowWidget->setLayout(rowLayout);
    rowWidget->setFixedHeight(24);  // 设置固定行高
    item->setSizeHint(rowWidget->sizeHint());
    
    listWidget->addItem(item);
    listWidget->setItemWidget(item, rowWidget);
}

void MainWindow::updateWindowCount(int count) {
    windowCountValue->setText(QString::number(count));
}

std::vector<HWND> MainWindow::findGameWindows() {
    g_windows.clear();
    EnumWindows(EnumWindowsProc, 0);
    return g_windows;
}

QString MainWindow::getCharacterName(HWND hwnd) {
    if (GameWindow* window = GameWindows::findByHwnd(hwnd)) {
        HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, window->pid);
        if (handle) {
            char buffer[18] = {0};
            SIZE_T bytesRead = 0;
            
            if (ReadProcessMemory(handle, (LPVOID)window->addresses.characterName,
                                buffer, sizeof(buffer), &bytesRead)) {
                QString name = QString::fromLocal8Bit(buffer);
                CloseHandle(handle);
                return name.isEmpty() ? "未登录" : name;
            }
            CloseHandle(handle);
        }
    }
    return "未登录";
}

void MainWindow::refreshGameWindowsList() {
    GameWindows::refresh();
    listWidget->clear();
    
    // 添加表头
    addTableRow("序号", "句柄", "快捷键", "角色", "状态");
    
    // 添加窗口数据
    int index = 1;
    for (const auto& window : GameWindows::windows) {
        // 获取角色名称
        QString roleName = window.role;
        if (roleName.isEmpty()) {
            HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, window.pid);
            if (handle) {
                char buffer[18] = {0};
                SIZE_T bytesRead = 0;
                
                if (ReadProcessMemory(handle, (LPVOID)window.addresses.characterName,
                                    buffer, sizeof(buffer), &bytesRead)) {
                    roleName = QString::fromLocal8Bit(buffer);
                }
                CloseHandle(handle);
            }
        }
        
        // 添加行
        addTableRow(
            QString::number(index++),
            QString::number((quintptr)window.hwnd),
            window.hotkey,
            roleName.isEmpty() ? "未登录" : roleName,
            window.task
        );
    }
    
    updateWindowCount(GameWindows::windows.size());
}

// 更新任务状态
void MainWindow::updateTaskStatus(const QList<HWND>& windows, const QString& status) {
    for (HWND hwnd : windows) {
        if (GameWindow* window = GameWindows::findByHwnd(hwnd)) {
            window->task = status;
            
            // 更新UI显示
            for (int i = 1; i < listWidget->count(); ++i) {
                QListWidgetItem* item = listWidget->item(i);
                if (!item) continue;
                
                QWidget* rowWidget = listWidget->itemWidget(item);
                if (!rowWidget) continue;
                
                QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(rowWidget->layout());
                if (!layout) continue;
                
                QLabel* handleLabel = qobject_cast<QLabel*>(layout->itemAt(1)->widget());
                if (!handleLabel) continue;
                
                if (handleLabel->text() == QString::number((quintptr)hwnd)) {
                    QLabel* taskLabel = qobject_cast<QLabel*>(layout->itemAt(4)->widget());
                    if (taskLabel) {
                        taskLabel->setText(status);
                    }
                    break;
                }
            }
        }
    }
} 