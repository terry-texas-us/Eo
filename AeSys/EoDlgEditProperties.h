#pragma once

class EoDlgEditProperties : public CDialog {
    DECLARE_DYNAMIC(EoDlgEditProperties)

public:
    EoDlgEditProperties(OdDbObjectId& id, CWnd* parent = NULL);
    virtual ~EoDlgEditProperties();

    enum { IDD = IDD_PROPERTIES };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;

    OdDbObjectId& m_pObjectId;
    OdResBufPtr m_pResBuf;
    int m_nCurItem;
    CButton m_doset;
    CListCtrl m_propList;
    CString m_sValue;

    void OnSetfocusValue();
    void OnButton();
    void OnClickProplist(NMHDR* pNMHDR, LRESULT* result);
    void OnKeydownProplist(NMHDR* pNMHDR, LRESULT* result);

    DECLARE_MESSAGE_MAP()
};