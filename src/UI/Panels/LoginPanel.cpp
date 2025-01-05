#include "UI/Panels/LoginPanel.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QDebug>
#include <QProcess>
#include "main.h"
#include "Util/WindowHelper.h"

std::vector<AccountInfo> LoginPanel::accounts;
bool LoginPanel::isLoginRunning = false;
LoginScript* LoginPanel::loginScript = nullptr;
QString LoginPanel::defaultGamePath = "C:\\Program Files (x86)\\netcraft_m4399\\bin";

void LoginPanel::initPanel(QWidget* panel) {
    auto layout = new QVBoxLayout(panel);
    
    // 添加游戏路径设置
    auto pathLayout = new QHBoxLayout();
    auto pathLabel = new QLabel("游戏路径:");
    auto pathEdit = new QLineEdit(defaultGamePath);
    auto browseBtn = new QPushButton("浏览");
    
    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseBtn);
    
    // 连接浏览按钮
    QObject::connect(browseBtn, &QPushButton::clicked, [pathEdit]() {
        selectGamePath(pathEdit);
    });
    
    // 创建账号列表
    auto table = new QTableWidget();
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"账号", "密码", "状态"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // 创建按钮组
    auto buttonLayout = new QHBoxLayout();
    auto addBtn = new QPushButton("添加账号");
    auto removeBtn = new QPushButton("删除账号");
    auto importBtn = new QPushButton("导入账号");
    auto exportBtn = new QPushButton("导出账号");
    auto startBtn = new QPushButton("开始登录");
    auto stopBtn = new QPushButton("停止登录");
    stopBtn->setEnabled(false);
    
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(removeBtn);
    buttonLayout->addWidget(importBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addWidget(startBtn);
    buttonLayout->addWidget(stopBtn);
    
    // 连接信号槽
    QObject::connect(addBtn, &QPushButton::clicked, [table]() {
        addAccount(table);
    });
    
    QObject::connect(removeBtn, &QPushButton::clicked, [table]() {
        removeAccount(table);
    });
    
    QObject::connect(importBtn, &QPushButton::clicked, [table]() {
        importAccounts(table);
    });
    
    QObject::connect(exportBtn, &QPushButton::clicked, [table]() {
        exportAccounts(table);
    });
    
    QObject::connect(startBtn, &QPushButton::clicked, [table, startBtn, stopBtn, pathEdit]() {
        QString gamePath = pathEdit->text();
        if (!QFile::exists(gamePath + "\\updater.exe")) {
            QMessageBox::warning(nullptr, "错误", "找不到更新程序！\n请确认游戏路径是否正确。");
            return;
        }
        
        startBtn->setEnabled(false);
        stopBtn->setEnabled(true);
        startLogin(table, gamePath);
    });
    
    QObject::connect(stopBtn, &QPushButton::clicked, [startBtn, stopBtn]() {
        startBtn->setEnabled(true);
        stopBtn->setEnabled(false);
        stopLogin();
    });
    
    layout->addLayout(pathLayout);
    layout->addWidget(table);
    layout->addLayout(buttonLayout);
}

void LoginPanel::addAccount(QTableWidget* table) {
    int row = table->rowCount();
    table->insertRow(row);
    
    // 设置账号和密码单元格可编辑
    table->setItem(row, 0, new QTableWidgetItem(""));
    table->setItem(row, 1, new QTableWidgetItem(""));
    table->setItem(row, 2, new QTableWidgetItem("等待中"));
}

void LoginPanel::removeAccount(QTableWidget* table) {
    auto selectedRows = table->selectionModel()->selectedRows();
    for (int i = selectedRows.count() - 1; i >= 0; i--) {
        table->removeRow(selectedRows.at(i).row());
    }
}

void LoginPanel::importAccounts(QTableWidget* table) {
    QString fileName = QFileDialog::getOpenFileName(nullptr,
        "导入账号", "", "文本文件 (*.txt)");
        
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "错误", "无法打开文件！");
        return;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() >= 2) {
            int row = table->rowCount();
            table->insertRow(row);
            
            // 设置账号和密码
            table->setItem(row, 0, new QTableWidgetItem(parts[0].trimmed()));
            table->setItem(row, 1, new QTableWidgetItem(parts[1].trimmed()));
            table->setItem(row, 2, new QTableWidgetItem("等待中"));
        }
    }
    
    file.close();
}

void LoginPanel::exportAccounts(QTableWidget* table) {
    QString fileName = QFileDialog::getSaveFileName(nullptr,
        "导出账号", "", "文本文件 (*.txt)");
        
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "错误", "无法创建文件！");
        return;
    }
    
    QTextStream out(&file);
    for (int row = 0; row < table->rowCount(); row++) {
        QString account = table->item(row, 0)->text();
        QString password = table->item(row, 1)->text();
        out << account << "," << password << "\n";
    }
    
    file.close();
}

void LoginPanel::selectGamePath(QLineEdit* pathEdit) {
    QString dir = QFileDialog::getExistingDirectory(nullptr, 
        "选择游戏目录",
        pathEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        
    if (!dir.isEmpty()) {
        pathEdit->setText(dir);
    }
}

void LoginPanel::updateGameWindow(HWND hwnd, const QString& status) {
    WindowHelper::updateWindowStatus(hwnd, status);
}

void LoginPanel::startLogin(QTableWidget* table, const QString& gamePath) {
    if (!loginScript) {
        loginScript = new LoginScript();
        
        // 连接信号
        QObject::connect(loginScript, &LoginScript::messageUpdated, 
            loginScript, [](const QString& msg) {
                qDebug() << msg;
            }, Qt::QueuedConnection);
            
        QObject::connect(loginScript, &LoginScript::statusUpdated,
            loginScript, [table](int row, const QString& status) {
                if (row >= 0 && row < table->rowCount()) {
                    table->item(row, 2)->setText(status);
                    
                    // 找到对应的游戏窗口并更新状态
                    HWND hwnd = WindowHelper::findLatestGameWindow();
                    updateGameWindow(hwnd, status);
                }
            }, Qt::QueuedConnection);
    }
    
    // 收集账号信息
    QList<LoginAccount> accounts;
    for (int row = 0; row < table->rowCount(); row++) {
        // 检查必要的单元格是否存在
        auto accountItem = table->item(row, 0);
        auto passwordItem = table->item(row, 1);
        
        if (!accountItem || !passwordItem) {
            QMessageBox::warning(nullptr, "错误", 
                QString("第 %1 行的账号信息不完整！").arg(row + 1));
            return;
        }
        
        LoginAccount info;
        info.account = accountItem->text();
        info.password = passwordItem->text();
        info.interval = 10;  // 使用固定的30秒间隔
        
        // 检查账号密码是否为空
        if (info.account.isEmpty() || info.password.isEmpty()) {
            QMessageBox::warning(nullptr, "错误", 
                QString("第 %1 行的账号或密码不能为空！").arg(row + 1));
            return;
        }
        
        accounts.append(info);
    }
    
    // 保存游戏路径
    QString updatePath = gamePath + "\\updater.exe";
    if (!QFile::exists(updatePath)) {
        QMessageBox::warning(nullptr, "错误", "找不到更新程序！");
        return;
    }
    
    // 直接启动脚本，让脚本处理每个账号的登录
    loginScript->start(accounts, gamePath);
}

void LoginPanel::stopLogin() {
    if (loginScript) {
        // 停止脚本
        loginScript->stop();
        
        // 使用 deleteLater 而不是直接删除
        loginScript->deleteLater();
        loginScript = nullptr;
    }
} 