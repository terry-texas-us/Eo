#pragma once

// EoDlgSetLength dialog

class EoDlgSetLength : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetLength)

public:
	EoDlgSetLength(CWnd* parent = NULL);
	virtual ~EoDlgSetLength();

// Dialog Data
	enum { IDD = IDD_SET_LENGTH };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
public:
	double	m_Length;
	CString m_Title;

protected:
	DECLARE_MESSAGE_MAP()
};