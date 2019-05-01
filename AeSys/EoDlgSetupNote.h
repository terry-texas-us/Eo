#pragma once

#include "EoDbFontDefinition.h"

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
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;

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
