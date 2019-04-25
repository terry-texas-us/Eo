#pragma once

#include "EoCtrlColorsButton.h"

class EoDlgSetupColor : public CDialog {
    DECLARE_DYNAMIC(EoDlgSetupColor)

public:

    EoDlgSetupColor(CWnd* parent = NULL);
    virtual ~EoDlgSetupColor();

    // Dialog Data
    enum { IDD = IDD_SETUP_COLOR };

protected:
    void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;
    void OnOK() final;
    BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* result) final;

    EoCtrlColorsButton m_EvenColorsButton;
    EoCtrlColorsButton m_OddColorsButton;
    EoCtrlColorsButton m_NamedColorsButton;
    EoCtrlColorsButton m_GraysButton;
    EoCtrlColorsButton m_SelectionButton;

    CEdit m_ColorEditControl;

    void DrawSelectionInformation(OdUInt16 index);

public:
    OdUInt16 m_ColorIndex;

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