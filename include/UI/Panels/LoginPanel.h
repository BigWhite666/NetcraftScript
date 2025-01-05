#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QHeaderView>
#include <QSpinBox>
#include <vector>
#include "Script/LoginScript.h"
#include "Util/GameWindow.h"

struct AccountInfo {
    QString account;
    QString password;
    QString status;  // 状态：等待中、登录中、已登录、失败等
    int interval;  // 添加间隔时间
};

class LoginPanel {
public:
    static void initPanel(QWidget* panel);
    
private:
    static void addAccount(QTableWidget* table);
    static void removeAccount(QTableWidget* table);
    static void startLogin(QTableWidget* table, const QString& gamePath);
    static void stopLogin();
    static void importAccounts(QTableWidget* table);
    static void exportAccounts(QTableWidget* table);
    static void selectGamePath(QLineEdit* pathEdit);
    static void updateGameWindow(HWND hwnd, const QString& status);
    
    static std::vector<AccountInfo> accounts;
    static bool isLoginRunning;
    static LoginScript* loginScript;
    static QString defaultGamePath;
}; 