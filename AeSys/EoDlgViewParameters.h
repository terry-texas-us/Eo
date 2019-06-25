#pragma once

class EoDlgViewParameters final : public CDialog {
DECLARE_DYNAMIC(EoDlgViewParameters)
	EoDlgViewParameters(CWnd* parent = nullptr);
	virtual ~EoDlgViewParameters();

	enum { IDD = IDD_VIEW_PARAMETERS };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
public:
	BOOL perspectiveProjection {FALSE};
	unsigned long modelView {0};
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
