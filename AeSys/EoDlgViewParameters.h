#pragma once

// EoDlgViewParameters dialog

class EoDlgViewParameters : public CDialog {
	DECLARE_DYNAMIC(EoDlgViewParameters)

public:
	EoDlgViewParameters(CWnd* parent = nullptr);
	virtual ~EoDlgViewParameters();

	// Dialog Data
	enum { IDD = IDD_VIEW_PARAMETERS };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;

public:
	BOOL m_PerspectiveProjection;
	unsigned long m_ModelView;

	void OnBnClickedApply();
	void OnEnChangePositionX();
	void OnEnChangePositionY();
	void OnEnChangePositionZ();
	void OnEnChangeTargetX();
	void OnEnChangeTargetY();
	void OnEnChangeTargetZ();
	void OnEnChangeFrontClipDistance();
	void OnEnChangeBackClipDistance();
	void OnEnChangeLensLength();
	void OnBnClickedPerspectiveProjection();

protected:
	DECLARE_MESSAGE_MAP()
};
