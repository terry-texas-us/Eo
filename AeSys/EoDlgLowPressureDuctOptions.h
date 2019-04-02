#pragma once

// EoDlgLowPressureDuctOptions dialog

class EoDlgLowPressureDuctOptions : public CDialog {
	DECLARE_DYNAMIC(EoDlgLowPressureDuctOptions)

public:
	EoDlgLowPressureDuctOptions(CWnd* parent = NULL);
	virtual ~EoDlgLowPressureDuctOptions();

// Dialog Data
	enum { IDD = IDD_DLGPROC_LPD_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

public:
	double m_Width;
	double m_Depth;
	double m_RadiusFactor;
	bool m_GenerateVanes;
	int m_Justification;
	bool m_BeginWithTransition;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedGenVanes() noexcept;
	afx_msg void OnEnChangeWidth();

protected:
	DECLARE_MESSAGE_MAP()
};
