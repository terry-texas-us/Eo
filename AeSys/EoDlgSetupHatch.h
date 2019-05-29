#pragma once

// EoDlgSetupHatch dialog

class EoDlgSetupHatch : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetupHatch)

public:
	EoDlgSetupHatch(CWnd* parent = nullptr) noexcept;
	~EoDlgSetupHatch();

// Dialog Data
	enum { IDD = IDD_SETUP_HATCH };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;
    void OnOK() final;

protected:
    DECLARE_MESSAGE_MAP()

public:
	double m_HatchXScaleFactor;
	double m_HatchYScaleFactor;
	double m_HatchRotationAngle;
};
