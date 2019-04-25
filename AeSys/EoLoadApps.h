#pragma once

#include "RxDLinkerReactor.h"

class EoLoadApps : public CDialog {
public:
    class LoadedApps : public OdArray<OdString>, public OdStaticRxObject<OdRxDLinkerReactor> {
    public:
        ODRX_HEAP_OPERATORS();
        LoadedApps() noexcept
            : m_pListBox(0) {}
        CListBox* m_pListBox;
        void rxAppLoaded(OdRxModule* appModule) override;
        void rxAppUnloaded(const OdString& appName) override;
    };
private:
    friend class LoadedApps;
    static LoadedApps* m_LoadedApps;

public:
    EoLoadApps(CWnd* parent = NULL);

    static void rxInit();
    static void rxUninit();

    enum { IDD = IDD_LOAD_APPS };

    CButton m_UnloadButton;
    CListBox m_AppsList;

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;

protected:
    afx_msg void OnLoadApp();
    afx_msg void OnUnloadApp();
    afx_msg void OnAppsListEvent();
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()
};
