#pragma once

// EoDlgSetupHatch dialog

class EoDlgSetupHatch : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetupHatch)

public:
	EoDlgSetupHatch(CWnd* pParent = NULL);
	virtual ~EoDlgSetupHatch();

// Dialog Data
	enum { IDD = IDD_SETUP_HATCH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

public:

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	double m_HatchXScaleFactor;
	double m_HatchYScaleFactor;
	double m_HatchRotationAngle;
};
