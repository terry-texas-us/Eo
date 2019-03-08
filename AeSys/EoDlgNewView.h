#pragma once

class EoDlgNewView : public CDialog {
public:
	EoDlgNewView(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_DIALOG_NEW_VIEW };
	CComboBox m_UCSs;
	CComboBox m_categories;
	CString m_sViewName;
	CString m_sViewCategory;
	BOOL m_bStoreLS;
	BOOL m_bSaveUCS;
	CString m_sUcsName;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
