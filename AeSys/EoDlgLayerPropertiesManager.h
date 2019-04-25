#pragma once

// EoDlgLayerPropertiesManager dialog

class EoDlgLayerPropertiesManager : public CDialog {
	DECLARE_DYNAMIC(EoDlgLayerPropertiesManager)

public:
	EoDlgLayerPropertiesManager(CWnd* parent = NULL);
	EoDlgLayerPropertiesManager(OdDbDatabasePtr database, CWnd* parent = NULL);

	virtual ~EoDlgLayerPropertiesManager();

	// Dialog Data
	enum { IDD = IDD_LAYER_PROPERTIES_MANAGER };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog(void) final;

	DECLARE_MESSAGE_MAP()

public:

	OdDbDatabasePtr m_Database;

	int m_DeltaHeight;
	int m_DeltaWidth;
	int m_InititialHeight;
	int m_InititialWidth;
	OdLyLayerFilterPtr m_RootFilter;
	CTreeCtrl m_TreeFilters;
	CImageList m_TreeImages;

	void UpdateFiltersTree();

public:
	afx_msg int OnCreate(LPCREATESTRUCT createStructure);
	afx_msg void OnNMDblclkLayerFilterTree(NMHDR *pNMHDR, LRESULT* result);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnSizing(UINT side, LPRECT rectangle);
	afx_msg void OnTvnKeydownLayerFilterTree(NMHDR *pNMHDR, LRESULT* result);
};