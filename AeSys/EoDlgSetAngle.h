#pragma once

// EoDlgSetAngle dialog

class EoDlgSetAngle : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetAngle)

	EoDlgSetAngle(CWnd* parent = nullptr);
	virtual ~EoDlgSetAngle();

	// Dialog Data
	enum { IDD = IDD_SET_ANGLE };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;

public:
	double m_dAngle;
	CString m_strTitle;

protected:
	DECLARE_MESSAGE_MAP()
};
