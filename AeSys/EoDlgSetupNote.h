#pragma once

// EoDlgSetupNote dialog

class EoDlgSetupNote : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetupNote)

public:
	EoDlgSetupNote(CWnd* parent = NULL);
	EoDlgSetupNote(EoDbFontDefinition* fontDefinition, CWnd* parent = NULL);
	virtual ~EoDlgSetupNote();

// Dialog Data
	enum { IDD = IDD_SETUP_NOTE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

public:
	EoDbFontDefinition* m_FontDefinition;

	CMFCFontComboBox m_MfcFontComboControl;

	double m_Height;
	double m_WidthFactor;
	double m_ObliqueAngle;
	double m_RotationAngle;

protected:
	DECLARE_MESSAGE_MAP()
};
