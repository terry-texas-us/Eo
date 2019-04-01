#pragma once

class EoDlgPassword : public CDialog {
public:
	EoDlgPassword(CWnd* parent = NULL);

	enum { IDD = IDD_PASSWORD_DLG };

	CEdit m_pswCtrl;
	CString m_sFileName;
	OdPassword m_password;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	DECLARE_MESSAGE_MAP()
};
