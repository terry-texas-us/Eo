#include "stdafx.h"
#include "AeSys.h"
#include "EoDbPrimitive.h"
#include "EoDlgSetupColor.h"
IMPLEMENT_DYNAMIC(EoDlgSetupColor, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetupColor, CDialog)
		ON_BN_CLICKED(IDC_BYBLOCK_BUTTON, &EoDlgSetupColor::OnBnClickedByblockButton)
		ON_BN_CLICKED(IDC_BYLAYER_BUTTON, &EoDlgSetupColor::OnBnClickedBylayerButton)
		ON_BN_CLICKED(IDC_EVEN_COLORS, &EoDlgSetupColor::OnClickedEvenColors)
		ON_BN_CLICKED(IDC_GRAYS, &EoDlgSetupColor::OnClickedGrays)
		ON_BN_CLICKED(IDC_NAMED_COLORS, &EoDlgSetupColor::OnClickedNamedColors)
		ON_BN_CLICKED(IDC_ODD_COLORS, &EoDlgSetupColor::OnClickedOddColors)
		ON_EN_CHANGE(IDC_COLOR_EDIT, &EoDlgSetupColor::OnChangeColorEdit)
		ON_WM_GETDLGCODE()
		ON_WM_PAINT()
END_MESSAGE_MAP()

EoDlgSetupColor::EoDlgSetupColor(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgSetupColor::~EoDlgSetupColor() = default;

void EoDlgSetupColor::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_EVEN_COLORS, m_EvenColorsButton);
	DDX_Control(dataExchange, IDC_ODD_COLORS, m_OddColorsButton);
	DDX_Control(dataExchange, IDC_NAMED_COLORS, m_NamedColorsButton);
	DDX_Control(dataExchange, IDC_GRAYS, m_GraysButton);
	DDX_Control(dataExchange, IDC_SELECTION_COLOR, m_SelectionButton);
	DDX_Control(dataExchange, IDC_COLOR_EDIT, m_ColorEditControl);
}

BOOL EoDlgSetupColor::OnInitDialog() {
	CDialog::OnInitDialog();
	EoCtrlColorsButton::SetPalette(g_ColorPalette);
	EoCtrlColorsButton::SetCurrentIndex(colorIndex);
	m_EvenColorsButton.SetLayout(EoCtrlColorsButton::GridUp5RowsEvenOnly, CSize(13, 13));
	m_EvenColorsButton.SetSequenceRange(10, 248);
	m_EvenColorsButton.SizeToContent();
	m_OddColorsButton.SetLayout(EoCtrlColorsButton::GridDown5RowsOddOnly, CSize(13, 13));
	m_OddColorsButton.SetSequenceRange(11, 249);
	m_OddColorsButton.SizeToContent();
	m_NamedColorsButton.SetLayout(EoCtrlColorsButton::SimpleSingleRow, CSize(17, 17));
	m_NamedColorsButton.SetSequenceRange(1, 9);
	m_NamedColorsButton.SizeToContent();
	m_GraysButton.SetLayout(EoCtrlColorsButton::SimpleSingleRow, CSize(17, 17));
	m_GraysButton.SetSequenceRange(250, 255);
	m_GraysButton.SizeToContent();
	m_SelectionButton.SetLayout(EoCtrlColorsButton::SimpleSingleRow, CSize(80, 20));
	m_SelectionButton.SetSequenceRange(colorIndex, colorIndex);
	m_SelectionButton.SizeToContent();
	DrawSelectionInformation(colorIndex);
	SetDlgItemInt(IDC_COLOR_EDIT, gsl::narrow_cast<unsigned>(colorIndex), FALSE);
	return TRUE;
}

void EoDlgSetupColor::OnOK() {
	colorIndex = gsl::narrow_cast<unsigned short>(GetDlgItemInt(IDC_COLOR_EDIT));
	colorIndex = colorIndex < 255u ? colorIndex : 255u;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnClickedNamedColors() {
	colorIndex = m_NamedColorsButton.m_SubItem;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnClickedGrays() {
	colorIndex = m_GraysButton.m_SubItem;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnClickedEvenColors() {
	colorIndex = m_EvenColorsButton.m_SubItem;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnClickedOddColors() {
	colorIndex = m_OddColorsButton.m_SubItem;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnBnClickedByblockButton() {
	colorIndex = EoDbPrimitive::COLORINDEX_BYBLOCK;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnBnClickedBylayerButton() {
	colorIndex = EoDbPrimitive::COLORINDEX_BYLAYER;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnChangeColorEdit() {
	const auto Index {gsl::narrow_cast<unsigned short>(GetDlgItemInt(IDC_COLOR_EDIT))};
	DrawSelectionInformation(Index);
}

BOOL EoDlgSetupColor::OnNotify(const WPARAM controlId, const LPARAM notificationMessage, LRESULT* result) {
	const auto NotifyMessage {reinterpret_cast<NMHDR*>(notificationMessage)};
	if (NotifyMessage->hwndFrom != nullptr) {
		const auto ColorsButton {dynamic_cast<EoCtrlColorsButton*>(FromHandle(NotifyMessage->hwndFrom))};
		if (ColorsButton != nullptr && ColorsButton->IsKindOf(RUNTIME_CLASS(EoCtrlColorsButton))) { DrawSelectionInformation(ColorsButton->m_SubItem); }
	}
	return CDialog::OnNotify(controlId, notificationMessage, result);
}

void EoDlgSetupColor::DrawSelectionInformation(unsigned short index) {
	SetDlgItemInt(IDC_INDEX_COLOR, gsl::narrow_cast<unsigned>(index), FALSE);
	m_SelectionButton.SetSequenceRange(index, index);
	m_SelectionButton.Invalidate();
	CString ColorRGBDescription;
	ColorRGBDescription.Format(L"(%i,%i,%i)", GetRValue(g_ColorPalette[index]), GetGValue(g_ColorPalette[index]), GetBValue(g_ColorPalette[index]));
	SetDlgItemTextW(IDC_RGBNUMBERS, ColorRGBDescription);
}
