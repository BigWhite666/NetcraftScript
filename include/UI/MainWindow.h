#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QCheckBox>
#include <windows.h>
#include <vector>
#include "Script/ChatScript.h"
#include "Script/MapScript.h"
#include "Script/DebugScript.h"
#include "Script/MemoryScript.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUI();
    void createPanels();
    void initHomePanel();
    void initScriptPanel();
    void initChatPanel();
    void addCheckableItem(const QString& text);
    void onItemStateChanged(QListWidgetItem* item);
    void addTableRow(const QString& index, const QString& handle, 
                    const QString& hotkey, const QString& role, 
                    const QString& task);
    void updateWindowCount(int count);
    std::vector<HWND> findGameWindows();
    QString getCharacterName(HWND hwnd);
    void refreshGameWindowsList();
    void updateTaskStatus(const QList<HWND>& windows, const QString& status);

    // UI 组件
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QListWidget *listWidget;
    QTabWidget *tabWidget;

    // 功能面板
    QWidget *homePanel;
    QWidget *scriptPanel;
    QWidget *chatPanel;

    // 顶部控制栏组件
    QPushButton* refreshButton;
    QPushButton* debugButton;
    QLabel* windowCountLabel;
    QLabel* windowCountValue;

    // 喊话相关控件
    ChatScript* m_chatScript;
    QTextEdit* chatContentEdit;
    QSpinBox* intervalSpinBox;
    QPushButton* startChatBtn;
    QPushButton* stopChatBtn;

    // 喊话相关函数
    void startChat();
    void stopChat();
    void sendChatMessage();

    MapScript* m_mapScript;

    // 添加 DebugScript 成员
    DebugScript* m_debugScript;

    MemoryScript* m_memoryScript;
}; 