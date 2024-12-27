#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include "UI/MainWindow.h"
#include "dm/dmutils.h"
#include "main.h"

// 全局变量定义
Idmsoft* DM = nullptr;

int main(int argc, char *argv[]) {
    // 初始化大漠插件
    DM = initialDMAndRegVIP();
    if (!DM) {
        QMessageBox::critical(nullptr, "错误", "大漠插件初始化失败！");
        return -1;
    }

    QApplication app(argc, argv);
    
    // 设置插件路径
    QDir dir(QCoreApplication::applicationDirPath());
    app.addLibraryPath(dir.absolutePath() + "/plugins");
    
    MainWindow window;
    window.show();
    
    int ret = app.exec();
    
    // 清理资源
    if (DM) {
        DM->Release();
    }
    
    return ret;
}
