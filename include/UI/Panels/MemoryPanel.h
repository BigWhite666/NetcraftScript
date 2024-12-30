#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include "Script/MemoryScript.h"

class MemoryPanel {
public:
    static void initPanel(QWidget* panel, MemoryScript* memoryScript);
    
private:
    static void setupMemoryFeatures(QWidget* panel, MemoryScript* memoryScript);
    static void connectSignals(QWidget* panel, MemoryScript* memoryScript);
}; 