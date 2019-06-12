#include "stdafx.h"
#include "AeSys.h"
//#include "AeSysView.h"

#include "EoDbBlockReference.h"

#include "EoDlgSetupNote.h"

// EoDlgSetupNote dialog

IMPLEMENT_DYNAMIC(EoDlgSetupNote, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetupNote, CDialog)
END_MESSAGE_MAP()

EoDlgSetupNote::EoDlgSetupNote(CWnd* parent)
	: CDialog(EoDlgSetupNote::IDD, parent)
	, m_FontDefinition(nullptr)
	, m_Height(0.0)
	, m_WidthFactor(0.0)
	, m_ObliqueAngle(0.0)
	, m_RotationAngle(0.0) {
}

EoDlgSetupNote::EoDlgSetupNote(EoDbFontDefinition* fontDefinition, CWnd* parent)
	: CDialog(EoDlgSetupNote::IDD, parent)
	, m_FontDefinition(fontDefinition)
	, m_Height(0)
	, m_WidthFactor(0)
	, m_ObliqueAngle(0)
	, m_RotationAngle(0) {
}

EoDlgSetupNote::~EoDlgSetupNote() {
}
void EoDlgSetupNote::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TEXT_HEIGHT, m_Height);
	DDX_Text(pDX, IDC_TEXT_EXP_FAC, m_WidthFactor);
	DDX_Text(pDX, IDC_TEXT_INCLIN, m_ObliqueAngle);
	DDX_Text(pDX, IDC_TEXT_ROTATION, m_RotationAngle);
	DDX_Control(pDX, IDC_MFCFONTCOMBO, m_MfcFontComboControl);
}

BOOL EoDlgSetupNote::OnInitDialog() {
	CDialog::OnInitDialog();
	m_MfcFontComboControl.Setup(TRUETYPE_FONTTYPE);
	m_MfcFontComboControl.AddString(L"Simplex.psf");
	m_MfcFontComboControl.SelectString(-1, m_FontDefinition->FontName());

	CString Spacing;
	Spacing.Format(L"%8.4f", m_FontDefinition->CharacterSpacing());
	SetDlgItemTextW(IDC_TEXT_SPACING, Spacing);

	CheckRadioButton(IDC_TEXT_ALIGN_HOR_LEFT, IDC_TEXT_ALIGN_HOR_RIGHT, IDC_TEXT_ALIGN_HOR_LEFT + m_FontDefinition->HorizontalAlignment() - 1);
	CheckRadioButton(IDC_TEXT_ALIGN_VER_BOT, IDC_TEXT_ALIGN_VER_TOP, IDC_TEXT_ALIGN_VER_BOT - m_FontDefinition->VerticalAlignment() + 4);
	CheckRadioButton(IDC_PATH_RIGHT, IDC_PATH_DOWN, IDC_PATH_RIGHT + m_FontDefinition->Path());

	return TRUE;
}
void EoDlgSetupNote::OnOK() {
	CString Spacing;
	GetDlgItemTextW(IDC_TEXT_SPACING, Spacing);
	m_FontDefinition->SetCharacterSpacing(_wtof(Spacing));

	const auto HorizontalAlignment {EoDb::HorizontalAlignment(1 - IDC_TEXT_ALIGN_HOR_LEFT + GetCheckedRadioButton(IDC_TEXT_ALIGN_HOR_LEFT, IDC_TEXT_ALIGN_HOR_RIGHT))};
	m_FontDefinition->SetHorizontalAlignment(HorizontalAlignment);

	const auto VerticalAlignment {EoDb::VerticalAlignment(4 + IDC_TEXT_ALIGN_VER_BOT - GetCheckedRadioButton(IDC_TEXT_ALIGN_VER_BOT, IDC_TEXT_ALIGN_VER_TOP))};
	m_FontDefinition->SetVerticalAlignment(VerticalAlignment);

	const auto Path {EoDb::Path(GetCheckedRadioButton(IDC_PATH_RIGHT, IDC_PATH_DOWN) - IDC_PATH_RIGHT)};
	m_FontDefinition->SetPath(Path);

	const int FontsIndex {m_MfcFontComboControl.GetCurSel()};

	if (FontsIndex != CB_ERR) {
		CString FontsItemName;
		m_MfcFontComboControl.GetLBText(FontsIndex, FontsItemName);
		m_FontDefinition->SetFontName(FontsItemName);
		const auto Precision {EoDb::Precision(FontsItemName.CompareNoCase(L"Simplex.psf") != 0 ? EoDb::kTrueType : EoDb::kStrokeType)};
		m_FontDefinition->SetPrecision(Precision);
	}
	CDialog::OnOK();
}
