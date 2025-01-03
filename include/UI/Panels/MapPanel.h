#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "Script/MapScript.h"
#include "UI/Style/StyleSheet.h"

class MapPanel {
public:
    static void initPanel(QWidget* panel, MapScript* mapScript);
    
private:
    static void setupMapControls(QWidget* panel, MapScript* mapScript);
    static void connectSignals(QWidget* panel, MapScript* mapScript);
}; 