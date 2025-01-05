#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include "UI/Style/StyleSheet.h"
#include "UI/MainWindow.h"
#include "Util/GameWindow.h"
#include "Util/CharacterHelper.h"

class MapPanel {
public:
    static void initPanel(QWidget* panel);
}; 