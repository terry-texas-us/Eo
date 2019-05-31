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
	virtual ~EoMfPropertiesDockablePane();

protected:
	int OnCreate(LPCREATESTRUCT createStructure);
	void OnSetFocus(CWnd* oldWindow);
	void OnSettingChange(unsigned flags, const wchar_t* section);
	void OnSize(unsigned type, int cx, int cy);

	LRESULT OnPropertyChanged(WPARAM, LPARAM);

	void OnExpandAllProperties();
	void OnProperties1();
	void OnSortProperties();
	void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	void OnUpdateProperties1(CCmdUI* pCmdUI);
	void OnUpdateSortProperties(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()

protected:
	void AdjustLayout() override;
	void InitializePropertyGrid();
	void SetPropertyGridFont();
	void SetWorkspaceTabsSubItemsState();

public:
	CMFCPropertyGridCtrl& GetPropertyGridCtrl() noexcept {
		return m_PropertyGrid;
	}
	CMFCPropertyGridProperty& GetActiveViewScaleProperty() {
		return *m_PropertyGrid.FindItemByData(kActiveViewScale);
	}
};
