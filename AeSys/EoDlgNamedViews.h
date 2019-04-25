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
    AeSysDoc* m_pDoc;
public:
    EoDlgNamedViews(AeSysDoc* pDoc, CWnd* parent = NULL);

    enum {
        kUnchangedItem = 0,
        kNewItem = 1,
        kReplace = 2
    };

    AeSysDoc* document() noexcept {
        return m_pDoc;
    }
    OdDbDatabase* database();

    enum { IDD = IDD_DIALOG_NAMED_VIEWS };
    CNamedViewListCtrl m_views;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

protected:
    BOOL OnInitDialog() override;
    void OnSetcurrentButton();
    void OnDblclkNamedviews(NMHDR* pNMHDR, LRESULT* pResult);
    void OnNewButton();
    void OnUpdateLayersButton();
    void OnDeleteButton();

    DECLARE_MESSAGE_MAP()
};
