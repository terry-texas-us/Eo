#pragma once
class EoDlgPassword final : public CDialog {
public:
	EoDlgPassword(CWnd* parent = nullptr);

	enum { IDD = IDD_PASSWORD_DLG };

	CEdit m_pswCtrl;
	CString m_sFileName;
	OdPassword m_password;
protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
DECLARE_MESSAGE_MAP()
};
