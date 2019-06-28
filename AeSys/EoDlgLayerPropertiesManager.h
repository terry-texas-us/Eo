#pragma once
#include "LyLayerFilter.h"

class EoDlgLayerPropertiesManager final : public CDialog {
DECLARE_DYNAMIC(EoDlgLayerPropertiesManager)
	EoDlgLayerPropertiesManager(CWnd* parent = nullptr);
	EoDlgLayerPropertiesManager(OdDbDatabasePtr database, CWnd* parent = nullptr);
	virtual ~EoDlgLayerPropertiesManager();

	enum { IDD = IDD_LAYER_PROPERTIES_MANAGER };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
DECLARE_MESSAGE_MAP()
public:
	OdDbDatabasePtr m_Database;
	int deltaHeight;
	int deltaWidth;
	int initialHeight;
	int initialWidth;
	OdLyLayerFilterPtr rootFilter;
	CTreeCtrl treeFilters;
	CImageList treeImages;
	void UpdateFiltersTree();
	int OnCreate(LPCREATESTRUCT createStructure); // hides non-virtual function of parent
	void OnNMDblclkLayerFilterTree(NMHDR* notifyStructure, LRESULT* result);
	void OnSize(unsigned type, int newWidth, int newHeight); // hides non-virtual function of parent
	void OnSizing(unsigned side, LPRECT rectangle); // hides non-virtual function of parent
	void OnTvnKeydownLayerFilterTree(NMHDR* notifyStructure, LRESULT* result);
};
