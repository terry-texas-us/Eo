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
    int OnCreate(LPCREATESTRUCT createStructure);
    void OnNMDblclkLayerFilterTree(NMHDR* pNMHDR, LRESULT* result);
    void OnSize(UINT type, int cx, int cy);
    void OnSizing(UINT side, LPRECT rectangle);
    void OnTvnKeydownLayerFilterTree(NMHDR* pNMHDR, LRESULT* result);
};