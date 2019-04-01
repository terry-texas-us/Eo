#pragma once

class EoDlgEditProperties : public CDialog {
	DECLARE_DYNAMIC(EoDlgEditProperties)

public:
	EoDlgEditProperties(OdDbObjectId &id, CWnd* parent = NULL);
	virtual ~EoDlgEditProperties();

	enum { IDD = IDD_PROPERTIES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	OdDbObjectId& m_pObjectId;
	OdResBufPtr m_pResBuf;
	int m_nCurItem;
	CButton m_doset;
	CListCtrl m_propList;
	CString m_sValue;

	virtual BOOL OnInitDialog();
	afx_msg void OnSetfocusValue();
	afx_msg void OnButton();
	afx_msg void OnClickProplist(NMHDR* pNMHDR, LRESULT* result);
	afx_msg void OnKeydownProplist(NMHDR* pNMHDR, LRESULT* result);

	DECLARE_MESSAGE_MAP()
};