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

	enum WorkspaceTabsSubItems {
		kTabsStyle,
		kTabLocation,
		kTabsAutoColor,
		kTabIcons,
		kTabBorderSize,
		kActiveViewScale
	};

public:
	virtual ~EoMfPropertiesDockablePane(void);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT createStructure);
	afx_msg void OnSetFocus(CWnd* oldWindow);
	afx_msg void OnSettingChange(UINT uFlags, LPCWSTR lpszSection);
	afx_msg void OnSize(UINT type, int cx, int cy);

	afx_msg LRESULT OnPropertyChanged(WPARAM, LPARAM);

	afx_msg void OnExpandAllProperties(void);
	afx_msg void OnProperties1(void);
	afx_msg void OnSortProperties(void);
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	afx_msg void OnUpdateProperties1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()

protected:
	void AdjustLayout(void);
	void InitializePropertyGrid(void);
	void SetPropertyGridFont(void);
	void SetWorkspaceTabsSubItemsState(void);

public:
	CMFCPropertyGridCtrl& GetPropertyGridCtrl(void) {
		return m_PropertyGrid;
	}
	CMFCPropertyGridProperty& GetActiveViewScaleProperty(void) {
		return *m_PropertyGrid.FindItemByData(kActiveViewScale);
	}
};
