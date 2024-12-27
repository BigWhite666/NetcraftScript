//
// Created by Administrator on 24-12-21.
//
#include <iostream>
#include <sstream>
#include "dmutils.h"

using namespace std;

Idmsoft *GetDmObject() {
    Idmsoft *m_dm = nullptr;
    bool m_bInit = false;
    typedef HRESULT(_stdcall
    *pfnGCO)(REFCLSID, REFIID, void**);
    pfnGCO fnGCO = nullptr;
    HINSTANCE hdllInst = LoadLibrary(DM_LIB_PATH);
    if (hdllInst == nullptr) {
        cout << "Load library 'dm.dll' failed ! DM_LIB_PATH = " << DM_LIB_PATH << endl;
        return nullptr;
    }
    fnGCO = (pfnGCO) GetProcAddress(hdllInst, "DllGetClassObject");
    if (fnGCO != nullptr) {
        IClassFactory *pcf = nullptr;
        HRESULT hr = (fnGCO)(__uuidof(dmsoft), IID_IClassFactory, (void **) &pcf);
        if (SUCCEEDED(hr) && (pcf != nullptr)) {
            hr = pcf->CreateInstance(nullptr, __uuidof(Idmsoft), (void **) &m_dm);
            if ((SUCCEEDED(hr) && (m_dm != nullptr)) == FALSE) {
                cout << "Create instance 'Idmsoft' failed !" << endl;
                return nullptr;
            }
        }
        pcf->Release();
        m_bInit = true;
    }
    return m_dm;
}

Idmsoft *initialDMAndRegVIP() {
    Idmsoft *pDm = GetDmObject();
    if (pDm == nullptr) {
        cout << "===> dm.dll registration failed !" << endl;
        return nullptr;
    }
    // 注册dm.dll成功，打印版本
    cout << "===> DM version: " << (char *) pDm->Ver() << endl;
    // 注册用户（同一程序下，只需注册一次，后续不用重复注册）
    long regResult = pDm->Reg(L"newone8aa3bb91f6c3deeae364b552ee31032f", L"newone666");
    if (regResult != 1) {
        cout << "===> Account registration failed ! code = " << regResult << endl;
        return nullptr;
    }
    cout << "===> Account registration successful ! " << endl;
    // long releaseRes = pDm->ReleaseRef();
    // cout << "===> ReleaseCode = " << releaseRes << endl;

    return pDm;
}

void doCaptureWindow(Idmsoft &pDm, long hwnd) {
    // 绑定窗口句柄
    long dmBind = pDm.BindWindowEx(
            hwnd,
            "normal",
            "normal",
            "normal",
            "",
            0
    );
    if (dmBind == 1) {
        // 恢复并激活指定窗口，置顶窗口，
        pDm.SetWindowState(hwnd, 12);
        pDm.SetWindowState(hwnd, 8);
        pDm.delay(600);
        // 延迟一下截图，存到相对路径
        wstring filename = wstring(L"./capture_window_").append(std::to_wstring(hwnd)).append(L".bmp");
        long retCap = pDm.Capture(0, 0, 2000, 2000, filename.c_str());
        if (retCap != 1) {
            cout << "capture failed" << endl;
        } else {
            cout << "capture success" << endl;
        }
        // 取消置顶窗口
        pDm.SetWindowState(hwnd, 9);
    } else {
        cout << "DM BindWindow failed" << endl;
    }
    pDm.UnBindWindow();
}
