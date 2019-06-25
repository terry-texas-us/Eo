#pragma once

class EoDlgSetupHatch final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetupHatch)
	EoDlgSetupHatch(CWnd* parent = nullptr) noexcept;
	~EoDlgSetupHatch();

	enum { IDD = IDD_SETUP_HATCH };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
DECLARE_MESSAGE_MAP()
public:
	double hatchXScaleFactor {1.0};
	double hatchYScaleFactor {1.0};
	double hatchRotationAngle {0.0};
};
