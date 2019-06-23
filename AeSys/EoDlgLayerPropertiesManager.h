#pragma once
#include "LyLayerFilter.h"

// EoDlgLayerPropertiesManager dialog
class EoDlgLayerPropertiesManager : public CDialog {
DECLARE_DYNAMIC(EoDlgLayerPropertiesManager)
	EoDlgLayerPropertiesManager(CWnd* parent = nullptr);
	EoDlgLayerPropertiesManager(OdDbDatabasePtr database, CWnd* parent = nullptr);
	virtual ~EoDlgLayerPropertiesManager();

	// Dialog Data
	enum { IDD = IDD_LAYER_PROPERTIES_MANAGER };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
DECLARE_MESSAGE_MAP()
public:
	OdDbDatabasePtr m_Database;
	int m_DeltaHeight;
	int m_DeltaWidth;
	int m_InitialHeight;
	int m_InitialWidth;
	OdLyLayerFilterPtr m_RootFilter;
	CTreeCtrl m_TreeFilters;
	CImageList m_TreeImages;
	void UpdateFiltersTree();
	int OnCreate(LPCREATESTRUCT createStructure);
	void OnNMDblclkLayerFilterTree(NMHDR* notifyStructure, LRESULT* result);
	void OnSize(unsigned type, int newWidth, int newHeight);
	void OnSizing(unsigned side, LPRECT rectangle);
	void OnTvnKeydownLayerFilterTree(NMHDR* notifyStructure, LRESULT* result);
};
