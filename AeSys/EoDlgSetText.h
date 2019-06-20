#pragma once

// EoDlgSetText dialog

class EoDlgSetText : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetText)

	EoDlgSetText(CWnd* parent = nullptr);
	virtual ~EoDlgSetText();

// Dialog Data
	enum { IDD = IDD_SET_TEXT };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;

public:
	CString m_sText;
	CString m_strTitle;

protected:
	DECLARE_MESSAGE_MAP()
};
