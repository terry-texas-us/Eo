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
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;
public:
	double	m_Length;
	CString m_Title;

protected:
	DECLARE_MESSAGE_MAP()
};