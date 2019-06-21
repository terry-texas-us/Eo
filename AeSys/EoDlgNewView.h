#pragma once
class EoDlgNewView : public CDialog {
public:
	EoDlgNewView(CWnd* parent = nullptr);

	enum { IDD = IDD_DIALOG_NEW_VIEW };

	CComboBox m_UCSs;
	CComboBox m_categories;
	CString m_sViewName;
	CString m_sViewCategory;
	BOOL m_bStoreLS;
	BOOL m_bSaveUCS;
	CString m_sUcsName;
protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
DECLARE_MESSAGE_MAP()
};
