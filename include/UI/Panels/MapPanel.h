#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "UI/Style/StyleSheet.h"

class MapPanel {
public:
    static void initPanel(QWidget* panel);
    
private:
    static void setupMapControls(QWidget* panel);
    static void connectSignals(QWidget* panel);
}; 