#pragma once

class AeSysDoc;
class OdDbViewTableRecord;
typedef OdSmartPtr<OdDbViewTableRecord> OdDbViewTableRecordPtr;

class CNamedViewListCtrl : public CListCtrl {
  void setViewId(int nItem, const OdDbObjectId& id);
  void setView(int nItem, const OdDbViewTableRecord* pView);
public:
  OdDbObjectId viewId(int n) const;
  OdDbViewTableRecordPtr view(int n);
  OdDbViewTableRecordPtr selectedView();
  void InsertItem(int i, const OdDbViewTableRecord* pView);
};

class EoDlgNamedViews : public CDialog {
  AeSysDoc *m_pDoc;
public:
  // Construction
	EoDlgNamedViews(AeSysDoc* pDoc, CWnd* pParent = NULL);   // standard constructor

  enum {
    kUnchangedItem  = 0,
    kNewItem        = 1,
    kReplace        = 2
  };

  AeSysDoc* document() {
	  return m_pDoc;
  }
  OdDbDatabase* database();

	enum { IDD = IDD_DIALOG_NAMED_VIEWS };
	CNamedViewListCtrl m_views;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnSetcurrentButton();
	afx_msg void OnDblclkNamedviews(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNewButton();
	afx_msg void OnUpdateLayersButton();
	afx_msg void OnDeleteButton();

	DECLARE_MESSAGE_MAP()
};
