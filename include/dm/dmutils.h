//
// Created by Administrator on 24-12-21.
//

#ifndef DMUTILS_H
#define DMUTILS_H

//#import "file:../external/dm.dll" no_namespace
#include "../external/dm.tlh"
// 设置dm.dll的绝对路径；或者配置相对路径，等下将dm.dll拷贝到生成的执行程序旁
// #define DM_LIB_PATH L"C:/Users/xxx/projects/dm/dm_demo/external/dm.dll"
#define DM_LIB_PATH L"../external/dm.dll"

using namespace std;

/**
 * 注册dm.dll，获取大漠实例
 * @return 大漠实例
 */
Idmsoft *GetDmObject();

/**
 * 初始化大漠插件，并注册用户VIP
 * @return 大漠实例
 */
Idmsoft *initialDMAndRegVIP();

/**
 * 截图
 * @param pDm 大漠实例
 * @param hwnd 窗口句柄
 */
void doCaptureWindow(Idmsoft &pDm, long hwnd);

#endif //DMUTILS_H
