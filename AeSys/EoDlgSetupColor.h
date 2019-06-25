#pragma once
#include "EoCtrlColorsButton.h"

class EoDlgSetupColor final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetupColor)
	EoDlgSetupColor(CWnd* parent = nullptr);
	~EoDlgSetupColor();

	// Dialog Data
	enum { IDD = IDD_SETUP_COLOR };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
	BOOL OnNotify(WPARAM controlId, LPARAM notificationMessage, LRESULT* result) final;
	EoCtrlColorsButton m_EvenColorsButton;
	EoCtrlColorsButton m_OddColorsButton;
	EoCtrlColorsButton m_NamedColorsButton;
	EoCtrlColorsButton m_GraysButton;
	EoCtrlColorsButton m_SelectionButton;
	CEdit m_ColorEditControl;
	void DrawSelectionInformation(unsigned short index);
public:
	unsigned short colorIndex {0};
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
