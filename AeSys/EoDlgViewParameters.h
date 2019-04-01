#pragma once

// EoDlgViewParameters dialog

class EoDlgViewParameters : public CDialog {
	DECLARE_DYNAMIC(EoDlgViewParameters)

public:
	EoDlgViewParameters(CWnd* parent = NULL);
	virtual ~EoDlgViewParameters();

// Dialog Data
	enum { IDD = IDD_VIEW_PARAMETERS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

public:
	BOOL m_PerspectiveProjection;
	DWORD m_ModelView;
	afx_msg void OnBnClickedApply();
	afx_msg void OnEnChangePositionX();
	afx_msg void OnEnChangePositionY();
	afx_msg void OnEnChangePositionZ();
	afx_msg void OnEnChangeTargetX();
	afx_msg void OnEnChangeTargetY();
	afx_msg void OnEnChangeTargetZ();
	afx_msg void OnEnChangeFrontClipDistance();
	afx_msg void OnEnChangeBackClipDistance();
	afx_msg void OnEnChangeLensLength();
	afx_msg void OnBnClickedPerspectiveProjection();

protected:
	DECLARE_MESSAGE_MAP()
};
