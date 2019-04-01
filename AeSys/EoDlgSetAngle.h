#pragma once

// EoDlgSetAngle dialog

class EoDlgSetAngle : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetAngle)

public:
	EoDlgSetAngle(CWnd* parent = NULL);
	virtual ~EoDlgSetAngle();

// Dialog Data
	enum { IDD = IDD_SET_ANGLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
public:
	double m_dAngle;
	CString m_strTitle;

protected:
	DECLARE_MESSAGE_MAP()
};
