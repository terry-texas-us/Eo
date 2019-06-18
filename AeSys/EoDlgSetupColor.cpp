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
	: CDialog(EoDlgSetupColor::IDD, parent)
	, m_ColorIndex(0) {
}

EoDlgSetupColor::~EoDlgSetupColor() {
}

void EoDlgSetupColor::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EVEN_COLORS, m_EvenColorsButton);
	DDX_Control(pDX, IDC_ODD_COLORS, m_OddColorsButton);
	DDX_Control(pDX, IDC_NAMED_COLORS, m_NamedColorsButton);
	DDX_Control(pDX, IDC_GRAYS, m_GraysButton);
	DDX_Control(pDX, IDC_SELECTION_COLOR, m_SelectionButton);
	DDX_Control(pDX, IDC_COLOR_EDIT, m_ColorEditControl);
}

BOOL EoDlgSetupColor::OnInitDialog() {
	CDialog::OnInitDialog();

	EoCtrlColorsButton::SetPalette(ColorPalette);
	EoCtrlColorsButton::SetCurrentIndex(m_ColorIndex);

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
	m_SelectionButton.SetSequenceRange(m_ColorIndex, m_ColorIndex);
	m_SelectionButton.SizeToContent();

	DrawSelectionInformation(m_ColorIndex);

	SetDlgItemInt(IDC_COLOR_EDIT, gsl::narrow_cast<unsigned>(m_ColorIndex), FALSE);
	return TRUE;
}

void EoDlgSetupColor::OnOK() {
	m_ColorIndex = gsl::narrow_cast<unsigned short>(GetDlgItemInt(IDC_COLOR_EDIT));
	m_ColorIndex = (m_ColorIndex < 255u) ? m_ColorIndex : 255u;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnClickedNamedColors() {
	m_ColorIndex = m_NamedColorsButton.m_SubItem;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnClickedGrays() {
	m_ColorIndex = m_GraysButton.m_SubItem;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnClickedEvenColors() {
	m_ColorIndex = m_EvenColorsButton.m_SubItem;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnClickedOddColors() {
	m_ColorIndex = m_OddColorsButton.m_SubItem;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnBnClickedByblockButton() {
	m_ColorIndex = EoDbPrimitive::COLORINDEX_BYBLOCK;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnBnClickedBylayerButton() {
	m_ColorIndex = EoDbPrimitive::COLORINDEX_BYLAYER;
	CDialog::OnOK();
}

void EoDlgSetupColor::OnChangeColorEdit() {
	const auto Index {gsl::narrow_cast<unsigned short>(GetDlgItemInt(IDC_COLOR_EDIT))};
	DrawSelectionInformation(Index);
}

BOOL EoDlgSetupColor::OnNotify(WPARAM controlId, LPARAM notificationMessage, LRESULT* result) {
	auto NotifyMessage {reinterpret_cast<NMHDR*>(notificationMessage)};
	
	if (NotifyMessage->hwndFrom != nullptr) {
		const auto ColorsButton {dynamic_cast<EoCtrlColorsButton*>(CWnd::FromHandle(NotifyMessage->hwndFrom))};

		if (ColorsButton != nullptr && ColorsButton->IsKindOf(RUNTIME_CLASS(EoCtrlColorsButton))) { DrawSelectionInformation(ColorsButton->m_SubItem); }
	}
	return CDialog::OnNotify(controlId, notificationMessage, result);
}

void EoDlgSetupColor::DrawSelectionInformation(const unsigned short index) {
	SetDlgItemInt(IDC_INDEX_COLOR, gsl::narrow_cast<unsigned>(index), FALSE);

	m_SelectionButton.SetSequenceRange(index, index);
	m_SelectionButton.Invalidate();

	CString ColorRGBDescription;
	ColorRGBDescription.Format(L"(%i,%i,%i)", GetRValue(ColorPalette[index]), GetGValue(ColorPalette[index]), GetBValue(ColorPalette[index]));
	SetDlgItemTextW(IDC_RGBNUMBERS, ColorRGBDescription);
}
