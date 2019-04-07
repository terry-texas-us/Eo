#pragma once

// EoDlgSelectIsometricView dialog

class EoDlgSelectIsometricView : public CDialog {
	DECLARE_DYNAMIC(EoDlgSelectIsometricView)

public:
	EoDlgSelectIsometricView(CWnd* parent = NULL);
	virtual ~EoDlgSelectIsometricView();

// Dialog Data
	enum { IDD = IDD_SELECT_ISOMETRIC_VIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;

public:
	int m_LeftRight;
	int m_FrontBack;
	int m_AboveUnder;

protected:
	DECLARE_MESSAGE_MAP()
};
