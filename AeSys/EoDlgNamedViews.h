#pragma once
class AeSysDoc;
class OdDbViewTableRecord;
using OdDbViewTableRecordPtr = OdSmartPtr<OdDbViewTableRecord>;

class CNamedViewListCtrl : public CListCtrl {
	void setViewId(int item, const OdDbObjectId& id);
	void setView(int item, const OdDbViewTableRecord* view);
public:
	[[nodiscard]] OdDbObjectId viewId(int item) const;
	OdDbViewTableRecordPtr view(int item);
	OdDbViewTableRecordPtr selectedView();
	void InsertItem(int i, const OdDbViewTableRecord* pView);
};

class EoDlgNamedViews : public CDialog {
	AeSysDoc* m_pDoc;
public:
	EoDlgNamedViews(AeSysDoc* pDoc, CWnd* parent = nullptr);

	enum { kUnchangedItem = 0, kNewItem = 1, kReplace = 2 };

	AeSysDoc* document() noexcept {
		return m_pDoc;
	}

	OdDbDatabase* database();

	enum { IDD = IDD_DIALOG_NAMED_VIEWS };

	CNamedViewListCtrl m_views;
protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnSetcurrentButton();
	void OnDblclkNamedviews(NMHDR* notifyStructure, LRESULT* pResult);
	void OnNewButton();
	void OnUpdateLayersButton();
	void OnDeleteButton();
DECLARE_MESSAGE_MAP()
};
