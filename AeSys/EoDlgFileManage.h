#pragma once

// EoDlgFileManage dialog
class EoDlgFileManage : public CDialog {
DECLARE_DYNAMIC(EoDlgFileManage)
	EoDlgFileManage(CWnd* parent = nullptr);
	EoDlgFileManage(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent = nullptr);
	virtual ~EoDlgFileManage();

	// Dialog Data
	enum { IDD = IDD_FILE_MANAGE };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
DECLARE_MESSAGE_MAP()
public:
	enum ColumnLabels { Status, Name, On, Freeze, Lock, Color, Linetype, Lineweight, PlotStyle, Plot, VpFreeze, VpColor, VpLinetype, VpLineweight, VpPlotStyle, Descr };

	AeSysDoc* m_Document;
	OdDbDatabasePtr m_Database;
	OdDbObjectId m_ActiveViewport;
	CListBox m_BlocksList;
	bool m_ClickToColumnStatus;
	int m_Description;
	CEdit m_Groups;
	CListCtrl m_LayersList;
	int m_NumberOfColumns;
	HWND m_PreviewWindowHandle;
	CImageList m_StateImages;
	void DrawItem(CDC& deviceContext, int itemID, int labelIndex, const RECT& itemRectangle);
	void UpdateCurrentLayerInfoField();
	void OnBnClickedFuse();
	void OnBnClickedMelt();
	void OnBnClickedNewlayer();
	void OnBnClickedSetcurrent();
	void OnDrawItem(int controlIdentifier, LPDRAWITEMSTRUCT drawItemStruct);
	void OnItemchangedLayersListControl(NMHDR* notifyStructure, LRESULT* result);
	void OnLbnSelchangeBlocksList();
	void OnNMClickLayersListControl(NMHDR* notifyStructure, LRESULT* result);
	void OnNMDblclkLayersListControl(NMHDR* notifyStructure, LRESULT* result);
	void OnLvnEndlabeleditLayersListControl(NMHDR* notifyStructure, LRESULT* result);
	void OnLvnBeginlabeleditLayersListControl(NMHDR* notifyStructure, LRESULT* result);
	void OnLvnKeydownLayersListControl(NMHDR* notifyStructure, LRESULT* result);
};
