#pragma once

class EoDlgLowPressureDuctOptions : public CDialog {
DECLARE_DYNAMIC(EoDlgLowPressureDuctOptions)
	EoDlgLowPressureDuctOptions(CWnd* parent = nullptr);
	virtual ~EoDlgLowPressureDuctOptions();

	enum { IDD = IDD_DLGPROC_LPD_OPTIONS };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
public:
	double width {0.0};
	double depth {0.0};
	double radiusFactor {0.0};
	bool generateVanes {false};
	int justification {0};
	bool beginWithTransition {false};
	void OnBnClickedOk();
	void OnBnClickedGenVanes() noexcept;
	void OnEnChangeWidth();
protected:
DECLARE_MESSAGE_MAP()
};
