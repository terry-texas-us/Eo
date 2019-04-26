#pragma once

class EoMfPropertiesMFCToolBar : public CMFCToolBar {
public:
    virtual void OnUpdateCmdUI(CFrameWnd* /* target */, BOOL disableIfNoHndler) {
        CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), disableIfNoHndler);
    }
};

class EoMfPropertiesDockablePane : public CDockablePane {
public:
    EoMfPropertiesDockablePane();

protected:
    CMFCPropertyGridCtrl m_PropertyGrid;
    CFont m_PropertyGridFont;
    CComboBox m_wndObjectCombo;
    EoMfPropertiesMFCToolBar m_PropertiesToolBar;

    enum WorkspaceTabsSubItems { kTabsStyle, kTabLocation, kTabsAutoColor, kTabIcons, kTabBorderSize, kActiveViewScale };

public:
    virtual ~EoMfPropertiesDockablePane(void);

protected:
    int OnCreate(LPCREATESTRUCT createStructure);
    void OnSetFocus(CWnd* oldWindow);
    void OnSettingChange(UINT uFlags, LPCWSTR lpszSection);
    void OnSize(UINT type, int cx, int cy);

    LRESULT OnPropertyChanged(WPARAM, LPARAM);

    void OnExpandAllProperties(void);
    void OnProperties1(void);
    void OnSortProperties(void);
    void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
    void OnUpdateProperties1(CCmdUI* pCmdUI);
    void OnUpdateSortProperties(CCmdUI* pCmdUI);

    DECLARE_MESSAGE_MAP()

protected:
    void AdjustLayout(void) override;
    void InitializePropertyGrid(void);
    void SetPropertyGridFont(void);
    void SetWorkspaceTabsSubItemsState(void);

public:
    CMFCPropertyGridCtrl& GetPropertyGridCtrl(void) noexcept {
        return m_PropertyGrid;
    }
    CMFCPropertyGridProperty& GetActiveViewScaleProperty(void) {
        return *m_PropertyGrid.FindItemByData(kActiveViewScale);
    }
};
