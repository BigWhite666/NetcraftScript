#include "UI/MainWindow.h"
#include "UI/Style/StyleSheet.h"
#include "UI/Panels/HomePanel.h"
#include "UI/Panels/LoginPanel.h"
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
#include "dm/dmutils.h"
#include "MemoryRead/GameOffsets.h"
#include "Script/ChatScript.h"
#include "Script/MapScript.h"
#include "main.h"
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
    setupUI();
    createPanels();
    refreshGameWindowsList();  // 初始化时获取一次窗口列表
}

MainWindow::~MainWindow() {
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
    headerLayout->setContentsMargins(5, 2, 5, 2);  // 减小左边距
    
    // 添加全选复选框
    QCheckBox* selectAllBox = new QCheckBox(headerWidget);
    selectAllBox->setFixedWidth(20);
    selectAllBox->setStyleSheet(Style::CHECKBOX_STYLE);  // 应用复选框样式
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
        for (int i = 1; i < listWidget->count(); ++i) {  // 从1开始跳过标题行
            QListWidgetItem* item = listWidget->item(i);
            if (item) {
                item->setCheckState(static_cast<Qt::CheckState>(state));
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

    // 连接调试按钮的信号槽
    connect(debugButton, &QPushButton::clicked, this, [this]() {
        qDebug() << "调试测试按钮被点击";
        
        // 遍历列表获取选中的窗口
        for (int i = 1; i < listWidget->count(); ++i) {
            QListWidgetItem* item = listWidget->item(i);
            if (item && item->checkState() == Qt::Checked) {
                QWidget* rowWidget = listWidget->itemWidget(item);
                if (rowWidget) {
                    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(rowWidget->layout());
                    if (layout) {
                        QLabel* handleLabel = qobject_cast<QLabel*>(layout->itemAt(1)->widget());
                        if (handleLabel) {
                            QString handleText = handleLabel->text();
                            bool ok;
                            HWND gameHwnd = (HWND)handleText.toLongLong(&ok, 10);
                            if (ok) {
                                qDebug() << "\n游戏窗口句柄:" << (quintptr)gameHwnd;
                                
                                // 遍历所有窗口，找出父窗口是游戏窗口且类名是#32770的窗口
                                EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
                                    HWND parentHwnd = GetParent(hwnd);
                                    HWND gameHwnd = (HWND)lParam;
                                    
                                    if (parentHwnd == gameHwnd) {
                                        char className[256];
                                        GetClassNameA(hwnd, className, sizeof(className));
                                        
                                        if (strcmp(className, "#32770") == 0) {
                                            char title[256];
                                            GetWindowTextA(hwnd, title, sizeof(title));
                                            
                                            qDebug() << "找到登录对话框:";
                                            qDebug() << "  窗口句柄:" << (quintptr)hwnd;
                                            qDebug() << "  父窗口句柄:" << (quintptr)parentHwnd;
                                            qDebug() << "  顶级窗口句柄:" << (quintptr)GetAncestor(hwnd, GA_ROOT);
                                            qDebug() << "  窗口类名:" << className;
                                            qDebug() << "  窗口标题:" << title;
                                            qDebug() << "";
                                            
                                            // 遍历这个对话框的子窗口
                                            EnumChildWindows(hwnd, [](HWND childHwnd, LPARAM lParam) -> BOOL {
                                                char childClassName[256];
                                                GetClassNameA(childHwnd, childClassName, sizeof(childClassName));
                                                
                                                if (strcmp(childClassName, "Internet Explorer_Server") == 0) {
                                                    qDebug() << "找到登录界面，准备截图:";
                                                    qDebug() << "  窗口句柄:" << (quintptr)childHwnd;
                                                    
                                                    // 绑定窗口
                                                    if (DM) {
                                                        long bindResult = DM->BindWindowEx(
                                                            reinterpret_cast<long>(childHwnd),
                                                            "gdi",  // 使用gdi模式截图
                                                            "dx",
                                                            "dx.keypad.input.lock.api",
                                                            "",
                                                            0
                                                        );
                                                        
                                                        if (bindResult == 1) {
                                                            qDebug() << "窗口绑定成功";
                                                            
                                                            // 获取窗口大小
                                                            VARIANT width, height;
                                                            DM->GetClientSize((long)childHwnd, &width, &height);
                                                            
                                                            // 生成文件名
                                                            QString filename = QString("login_capture_%1.bmp").arg((quintptr)childHwnd);
                                                            
                                                            // 截图
                                                            long ret = DM->Capture(0, 0, width.intVal, height.intVal, 
                                                                filename.toStdWString().c_str());
                                                            
                                                            if (ret == 1) {
                                                                qDebug() << "截图成功：" << filename;
                                                            } else {
                                                                qDebug() << "截图失败：" << filename;
                                                            }
                                                            
                                                            // 解绑窗口
                                                            DM->UnBindWindow();
                                                        } else {
                                                            qDebug() << "窗口绑定失败";
                                                        }
                                                    }
                                                }
                                                
                                                return TRUE;  // 继续枚举
                                            }, 0);
                                        }
                                    }
                                    
                                    return TRUE;  // 继续枚举
                                }, (LPARAM)gameHwnd);
                            }
                        }
                    }
                }
            }
        }
    });
}

void MainWindow::createPanels() {
    // 创建各个面板
    homePanel = new QWidget();
    scriptPanel = new QWidget();
    chatPanel = new QWidget();
    
    // 初始化各个面板
    initHomePanel();
    initScriptPanel();
    initChatPanel();
    
    // 添加到标签页
    tabWidget->addTab(homePanel, "主页");
    tabWidget->addTab(scriptPanel, "跑图");
    tabWidget->addTab(chatPanel, "喊话");
    
    // 添加上号器面板
    auto loginPanel = new QWidget();
    LoginPanel::initPanel(loginPanel);
    tabWidget->addTab(loginPanel, "上号器");
}

void MainWindow::initHomePanel() {
    auto layout = new QGridLayout(homePanel);
    
    // 状态组
    auto statusGroup = new QGroupBox("状态", homePanel);
    auto statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->addWidget(new QLabel("当前状态: 未运行"));
    statusLayout->addWidget(new QLabel("运行时间: 0:00:00"));
    
    // 控制组
    auto controlGroup = new QGroupBox("控制", homePanel);
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
    
    layout->addWidget(statusGroup, 0, 0);
    layout->addWidget(controlGroup, 0, 1);
}

void MainWindow::initScriptPanel() {
    auto layout = new QVBoxLayout(scriptPanel);
    
    // 脚本选择组
    auto scriptGroup = new QGroupBox("脚本设置", scriptPanel);
    auto scriptLayout = new QGridLayout(scriptGroup);
    
    scriptLayout->addWidget(new QLabel("选择地图:"), 0, 0);
    auto mapComboBox = new QComboBox();  // 保存为成员变量
    mapComboBox->addItem("测试地图");    // 添加地图选项
    scriptLayout->addWidget(mapComboBox, 0, 1);
    
    scriptLayout->addWidget(new QLabel("循环次数:"), 1, 0);
    auto loopSpinBox = new QSpinBox();   // 保存为成员变量
    loopSpinBox->setMinimum(1);
    loopSpinBox->setMaximum(9999);
    loopSpinBox->setValue(1);
    scriptLayout->addWidget(loopSpinBox, 1, 1);
    
    // 操作按钮
    auto buttonLayout = new QHBoxLayout();
    auto startScriptBtn = new QPushButton("开始跑图");
    startScriptBtn->setStyleSheet(Style::BUTTON_STYLE);
    auto stopScriptBtn = new QPushButton("停止");
    stopScriptBtn->setStyleSheet(Style::BUTTON_STYLE);
    stopScriptBtn->setEnabled(false);
    
    // 连接按钮点击事件
    connect(startScriptBtn, &QPushButton::clicked, this, [=]() {
        QString mapName = mapComboBox->currentText();
        if (mapName.isEmpty()) {
            QMessageBox::warning(this, "警告", "请选择地图！");
            return;
        }
        
        // 获取选中的窗口
        QList<HWND> selectedWindows;
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
        
        // 开始跑图
        startScriptBtn->setEnabled(false);
        stopScriptBtn->setEnabled(true);
        mapComboBox->setEnabled(false);
        loopSpinBox->setEnabled(false);
        
        m_mapScript->start(mapName, loopSpinBox->value(), selectedWindows);
    });
    
    connect(stopScriptBtn, &QPushButton::clicked, this, [=]() {
        try {
            if (m_mapScript) {
                m_mapScript->stop();
            }
            
            // 恢复UI状态
            startScriptBtn->setEnabled(true);
            stopScriptBtn->setEnabled(false);
            mapComboBox->setEnabled(true);
            loopSpinBox->setEnabled(true);
        } catch (const std::exception& e) {
            qDebug() << "停止脚本时发生错误:" << e.what();
        }
    });
    
    // 连接脚本信号
    connect(m_mapScript, &MapScript::messageUpdated, this, [](const QString& msg) {
        qDebug() << msg;  // 或者更新到UI上显示
    });
    
    connect(m_mapScript, &MapScript::progressUpdated, this, [](int current, int total) {
        qDebug() << "Progress:" << current << "/" << total;  // 或者更新到进度条
    });
    
    buttonLayout->addWidget(startScriptBtn);
    buttonLayout->addWidget(stopScriptBtn);
    
    layout->addWidget(scriptGroup);
    layout->addLayout(buttonLayout);
    layout->addStretch();
}

void MainWindow::initChatPanel() {
    auto layout = new QVBoxLayout(chatPanel);
    
    // 喊话设置组
    auto chatGroup = new QGroupBox("喊话设置", chatPanel);
    auto chatLayout = new QGridLayout(chatGroup);
    
    chatLayout->addWidget(new QLabel("喊话内容:"), 0, 0);
    auto chatContentEdit = new QTextEdit();  // 保存为成员变量以便后续访问
    chatLayout->addWidget(chatContentEdit, 0, 1);
    
    chatLayout->addWidget(new QLabel("间隔(秒):"), 1, 0);
    auto intervalSpinBox = new QSpinBox();   // 保存为成员变量以便后续访问
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
    stopChatBtn->setEnabled(false);  // 初始时停止按钮不可用
    
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
        for (int i = 1; i < listWidget->count(); ++i) {  // 从1开始跳过标题行
            QListWidgetItem* item = listWidget->item(i);
            if (item && item->checkState() == Qt::Checked) {
                // 从item的widget中获取句柄文本
                QWidget* rowWidget = listWidget->itemWidget(item);
                if (rowWidget) {
                    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(rowWidget->layout());
                    if (layout) {
                        // 句柄在第二列
                        QLabel* handleLabel = qobject_cast<QLabel*>(layout->itemAt(1)->widget());
                        if (handleLabel) {
                            QString handleText = handleLabel->text();
                            // 直接使用十进制转换
                            bool ok;
                            HWND hwnd = (HWND)handleText.toLongLong(&ok, 10);  // 使用十进制转换
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
        
        // 开始喊话
        startChatBtn->setEnabled(false);
        stopChatBtn->setEnabled(true);
        chatContentEdit->setEnabled(false);
        intervalSpinBox->setEnabled(false);
        
        m_chatScript->start(content, intervalSpinBox->value(), selectedWindows);
    });
    
    connect(stopChatBtn, &QPushButton::clicked, this, [=]() {
        m_chatScript->stop();
        
        // 恢复UI状态
        startChatBtn->setEnabled(true);
        stopChatBtn->setEnabled(false);
        chatContentEdit->setEnabled(true);
        intervalSpinBox->setEnabled(true);
    });
    
    // 连接脚本信号
    connect(m_chatScript, &ChatScript::messageUpdated, this, [](const QString& msg) {
        qDebug() << msg;  // 或者更新到UI上显示
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
        qDebug() << "Item" << item->text() << "is" << (checked ? "checked" : "unchecked");
        // 在这里处理选中状态变化
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
    DWORD pid = GetPid(hwnd);
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!handle) {
        return "无法访问进程";
    }

    QString result;
    char buffer[18] = {0};
    SIZE_T bytesRead = 0;

    GameOffsets::Initialize(hwnd);
    if (ReadProcessMemory(handle, (LPVOID)GameOffsets::Character::NAME,
                         buffer, sizeof(buffer), &bytesRead)) {
        std::string utf8String(buffer, bytesRead);
        std::string nameStr = GetUtf8String(utf8String);
        result = QString::fromLocal8Bit(nameStr.c_str());
        
        // 设置窗口标题为角色名称
        if (!result.isEmpty() && result != "未登录") {
            SetWindowTextA(hwnd, nameStr.c_str());
        }
    } else {
        result = "未登录";
    }

    CloseHandle(handle);
    return result;
}

void MainWindow::refreshGameWindowsList() {
    // 清除现有项目（保留标题行）
    while (listWidget->count() > 1) {
        listWidget->takeItem(1);
    }

    // 获取游戏窗口并更新计数
    auto windows = findGameWindows();
    updateWindowCount(windows.size());

    // 添加窗口到列表
    int index = 1;
    for (HWND hwnd : windows) {
        addTableRow(
            QString::number(index),
            QString::number((quintptr)hwnd),
            QString("F%1").arg(index),
            getCharacterName(hwnd),
            "等待中"
        );
        index++;
    }
} 