#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include "UI/Style/StyleSheet.h"
#include "Util/GameWindow.h"

class HomePanel {
public:
    static void initPanel(QWidget* panel);
    
private:
    static void setupWindowList(QListWidget* list);
    static void updateWindowList(QListWidget* list);
    static void onWindowItemChanged(QListWidgetItem* item);
}; 