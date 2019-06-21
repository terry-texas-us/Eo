#pragma once
class EoMfPropertiesMFCToolBar : public CMFCToolBar {
public:
	void OnUpdateCmdUI(CFrameWnd* /* target */, BOOL disableIfNoHndler) override {
		CMFCToolBar::OnUpdateCmdUI(static_cast<CFrameWnd*>(GetOwner()), disableIfNoHndler);
	}

	BOOL AllowShowOnList() const noexcept override { return FALSE; }
};

class EoMfPropertiesDockablePane : public CDockablePane {
public:
	EoMfPropertiesDockablePane() = default;
	void AdjustLayout() override;
protected:
	CMFCPropertyGridCtrl m_PropertyGrid;
	CFont m_PropertyGridFont;
	CComboBox m_wndObjectCombo;
	EoMfPropertiesMFCToolBar m_PropertiesToolBar;

	enum WorkspaceTabsSubItems { kTabsStyle, kTabLocation, kTabsAutoColor, kTabIcons, kTabBorderSize, kActiveViewScale };

public:
	~EoMfPropertiesDockablePane() = default;
protected:
	int OnCreate(LPCREATESTRUCT createStructure); // hides non-virtual function of parent
	void OnSetFocus(CWnd* oldWindow); // hides non-virtual function of parent
	void OnSettingChange(unsigned flags, const wchar_t* section); // hides non-virtual function of parent
	void OnSize(unsigned type, int cx, int cy); // hides non-virtual function of parent
	LRESULT OnPropertyChanged(WPARAM, LPARAM);
	void OnExpandAllProperties();
	void OnProperties1() noexcept;
	void OnSortProperties();
	void OnUpdateExpandAllProperties(CCmdUI* commandUserInterface) noexcept;
	void OnUpdateProperties1(CCmdUI* commandUserInterface) noexcept;
	void OnUpdateSortProperties(CCmdUI* commandUserInterface);
DECLARE_MESSAGE_MAP()
	void InitializePropertyGrid();
	void SetPropertyGridFont();
	void SetWorkspaceTabsSubItemsState();
	int m_nComboHeight {0};
	static std::vector<const wchar_t*> TabsStyles;
	static std::vector<const wchar_t*> TabsLocations;
public:
	CMFCPropertyGridCtrl& GetPropertyGridCtrl() noexcept {
		return m_PropertyGrid;
	}

	CMFCPropertyGridProperty& GetActiveViewScaleProperty() {
		return *m_PropertyGrid.FindItemByData(kActiveViewScale);
	}
};
