#pragma once
class EoDlgSelectIsometricView final : public CDialog {
DECLARE_DYNAMIC(EoDlgSelectIsometricView)

	EoDlgSelectIsometricView(CWnd* parent = nullptr);

	enum { IDD = IDD_SELECT_ISOMETRIC_VIEW };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

public:
	int leftRight {0};
	int frontBack {0};
	int aboveUnder {0};
DECLARE_MESSAGE_MAP()
};
