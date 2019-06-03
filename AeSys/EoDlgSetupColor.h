#pragma once

#include "EoCtrlColorsButton.h"

class EoDlgSetupColor : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetupColor)

public:

	EoDlgSetupColor(CWnd* parent = nullptr);
	~EoDlgSetupColor();

	// Dialog Data
	enum { IDD = IDD_SETUP_COLOR };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
	BOOL OnNotify(WPARAM controlId, LPARAM notificationMessage, LRESULT* result) final;

	EoCtrlColorsButton m_EvenColorsButton;
	EoCtrlColorsButton m_OddColorsButton;
	EoCtrlColorsButton m_NamedColorsButton;
	EoCtrlColorsButton m_GraysButton;
	EoCtrlColorsButton m_SelectionButton;

	CEdit m_ColorEditControl;

	void DrawSelectionInformation(const unsigned short index);

public:
	unsigned short m_ColorIndex;

	void OnBnClickedByblockButton();
	void OnBnClickedBylayerButton();
	void OnClickedEvenColors();
	void OnClickedGrays();
	void OnClickedNamedColors();
	void OnClickedOddColors();
	void OnChangeColorEdit();

protected:
	DECLARE_MESSAGE_MAP()
};