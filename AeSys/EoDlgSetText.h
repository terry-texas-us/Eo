#pragma once

// EoDlgSetText dialog

class EoDlgSetText : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetText)

public:
	EoDlgSetText(CWnd* parent = NULL);
	virtual ~EoDlgSetText();

// Dialog Data
	enum { IDD = IDD_SET_TEXT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog(void);

public:
	CString m_sText;
	CString m_strTitle;

protected:
	DECLARE_MESSAGE_MAP()
};
