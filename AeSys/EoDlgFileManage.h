#pragma once

// EoDlgFileManage dialog

class EoDlgFileManage : public CDialog {
    DECLARE_DYNAMIC(EoDlgFileManage)

public:
    EoDlgFileManage(CWnd* parent = NULL);
    EoDlgFileManage(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent = NULL);

    virtual ~EoDlgFileManage();

    // Dialog Data
    enum { IDD = IDD_FILE_MANAGE };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;

protected:
    DECLARE_MESSAGE_MAP()

public:
    enum ColumnLabels { Status, Name, On, Freeze, Lock, Color, Linetype, Lineweight, PlotStyle, Plot, VpFreeze, VpColor, VpLinetype, VpLineweight, VpPlotStyle, Descr };

    AeSysDoc* m_Document;
    OdDbDatabasePtr m_Database;
    OdDbObjectId  m_ActiveViewport;

    CListBox m_BlocksList;
    bool m_ClickToColumnStatus;
    int m_Description;
    CEdit m_Groups;
    CListCtrl m_LayersList;
    int m_NumberOfColumns;
    HWND m_PreviewWindowHandle;
    CImageList m_StateImages;

    void DrawItem(CDC& deviceContext, int itemID, int labelIndex, const RECT& rcItem);
    void UpdateCurrentLayerInfoField();

public:
    void OnBnClickedFuse();
    void OnBnClickedMelt();
    void OnBnClickedNewlayer();
    void OnBnClickedSetcurrent();
    void OnDrawItem(int controlIdentifier, LPDRAWITEMSTRUCT drawItemStruct);
    void OnItemchangedLayersListControl(NMHDR* pNMHDR, LRESULT* result);
    void OnLbnSelchangeBlocksList();
    void OnNMClickLayersListControl(NMHDR* pNMHDR, LRESULT* result);
    void OnNMDblclkLayersListControl(NMHDR* pNMHDR, LRESULT* result);
    void OnLvnEndlabeleditLayersListControl(LPNMHDR NMHDR, LRESULT* result);
    void OnLvnBeginlabeleditLayersListControl(LPNMHDR NMHDR, LRESULT* result);
    void OnLvnKeydownLayersListControl(LPNMHDR NMHDR, LRESULT* result);
};
