#pragma once

class EoDlgFileManage final : public CDialog {
DECLARE_DYNAMIC(EoDlgFileManage)
	EoDlgFileManage(CWnd* parent = nullptr);
	EoDlgFileManage(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent = nullptr);

	// Dialog Data
	enum { IDD = IDD_FILE_MANAGE };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
DECLARE_MESSAGE_MAP()
	enum ColumnLabels { kStatus, kName, kOn, kFreeze, kLock, kColor, kLinetype, kLineweight, kPlotStyle, kPlot, kVpFreeze, kVpColor, kVpLinetype, kVpLineweight, kVpPlotStyle, kDescr };

	AeSysDoc* m_Document;
	OdDbDatabasePtr m_Database;
	OdDbObjectId m_ActiveViewport;
	CListBox m_BlocksList;
	bool m_ClickToColumnStatus {false};
	int m_Description {0};
	CEdit m_Groups;
	CListCtrl m_LayersList;
	int m_NumberOfColumns {0};
	HWND m_PreviewWindowHandle {nullptr};
	CImageList m_StateImages;
public:
	void DrawItem(CDC& deviceContext, int itemId, int labelIndex, const RECT& itemRectangle);
	void UpdateCurrentLayerInfoField();
	void OnBnClickedFuse();
	void OnBnClickedMelt();
	void OnBnClickedNewLayer();
	void OnBnClickedSetCurrent();
	void OnDrawItem(int controlIdentifier, LPDRAWITEMSTRUCT drawItemStruct); // hides non-virtual function of parent
	void OnItemchangedLayersListControl(NMHDR* notifyStructure, LRESULT* result);
	void OnLbnSelchangeBlocksList();
	void OnNMClickLayersListControl(NMHDR* notifyStructure, LRESULT* result);
	void OnNMDblclkLayersListControl(NMHDR* notifyStructure, LRESULT* result);
	void OnLvnEndlabeleditLayersListControl(NMHDR* notifyStructure, LRESULT* result);
	void OnLvnBeginlabeleditLayersListControl(NMHDR* notifyStructure, LRESULT* result);
	void OnLvnKeydownLayersListControl(NMHDR* notifyStructure, LRESULT* result);
};
