#include "UI/Style/StyleSheet.h"

namespace Style {
    const QString MAIN_STYLE = R"(
        QMainWindow {
            background-color: #F0F5FA;
        }
        
        QWidget {
            font-family: "Microsoft YaHei", "Segoe UI";
        }
        
        QGroupBox {
            border: 1px solid #BED2E3;
            border-radius: 4px;
            margin-top: 8px;
            background-color: white;
        }
        
        QGroupBox::title {
            color: #428BCA;
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 3px;
        }
        
        QLabel {
            color: #333333;
        }
        
        QSpinBox, QComboBox, QTextEdit {
            border: 1px solid #BED2E3;
            border-radius: 3px;
            padding: 2px;
            background-color: white;
        }
    )";

    const QString LIST_STYLE = R"(
        QListWidget {
            border: 1px solid #BED2E3;
            border-radius: 4px;
            background-color: white;
            outline: none;
        }
        
        QListWidget::item {
            border-bottom: 1px solid #E6EEF4;
            padding: 0px;
            margin: 0px;
            color: #333333;
            min-height: 24px;
        }
        
        /* 列表项中的标签样式 */
        QListWidget QLabel {
            color: #333333;
            padding: 0px;
            margin: 0px;
            min-height: 20px;
        }
        
        /* 列表项中的布局样式 */
        QListWidget QWidget {
            background: transparent;
        }
        
        QListWidget QHBoxLayout {
            margin: 0px;
            padding: 0px;
            spacing: 20px;
        }
        
        QListWidget::item:alternate {
            background-color: #F5F9FC;
        }
        
        QListWidget::item:selected {
            background-color: transparent;
            color: #333333;
        }
        
        QListWidget::item:hover {
            background-color: #EBF2F9;
        }
        
        QListWidget::item:checked {
            background-color: #E3F2FD;
        }
        
        /* 滚动条样式 */
        QScrollBar:vertical {
            border: none;
            background: #F5F5F5;
            width: 8px;
            margin: 0;
        }
        
        QScrollBar::handle:vertical {
            background: #BED2E3;
            border-radius: 4px;
            min-height: 30px;
        }
        
        QScrollBar::handle:vertical:hover {
            background: #9CB5CA;
        }
        
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
            width: 0;
        }
        
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
        }
    )";

    const QString BUTTON_STYLE = R"(
        QPushButton {
            background-color: #428BCA;
            border: none;
            color: white;
            padding: 5px 15px;
            border-radius: 3px;
            min-height: 25px;
        }
        
        QPushButton:hover {
            background-color: #3276B1;
        }
        
        QPushButton:pressed {
            background-color: #245682;
        }
    )";

    const QString TAB_STYLE = R"(
        QTabWidget::pane {
            border: 1px solid #BED2E3;
            border-radius: 4px;
            background-color: white;
            top: -1px;
        }
        
        QTabBar::tab {
            background-color: #F5F5F5;
            border: 1px solid #BED2E3;
            padding: 5px 15px;
            margin-right: 2px;
        }
        
        QTabBar::tab:selected {
            background-color: #428BCA;
            color: white;
        }
        
        QTabBar::tab:hover:!selected {
            background-color: #E3F2FD;
        }
    )";

    const QString CHECKBOX_STYLE = R"(
        QCheckBox {
            spacing: 2px;
        }

        QCheckBox::indicator {
            width: 14px;
            height: 14px;
            border: 1px solid #BED2E3;
            border-radius: 2px;
            background-color: white;
        }

        QCheckBox::indicator:unchecked:hover {
            background-color: #E3F2FD;
            border-color: #428BCA;
        }

        QCheckBox::indicator:checked {
            background-color: #428BCA;
            border-color: #428BCA;
        }

        QCheckBox::indicator:checked:hover {
            background-color: #3276B1;
            border-color: #3276B1;
        }

        QCheckBox::indicator:disabled {
            background-color: #F5F5F5;
            border-color: #D9D9D9;
        }
    )";
} 