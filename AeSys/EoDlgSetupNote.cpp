#include "stdafx.h"
#include "AeSys.h"
//#include "AeSysView.h"
#include "EoDbBlockReference.h"
#include "EoDlgSetupNote.h"

IMPLEMENT_DYNAMIC(EoDlgSetupNote, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetupNote, CDialog)
END_MESSAGE_MAP()

EoDlgSetupNote::EoDlgSetupNote(CWnd* parent)
	: CDialog(IDD, parent)
	, fontDefinition(nullptr) {
}

EoDlgSetupNote::EoDlgSetupNote(EoDbFontDefinition* fontDefinition, CWnd* parent)
	: CDialog(IDD, parent)
	, fontDefinition(fontDefinition) {
}

EoDlgSetupNote::~EoDlgSetupNote() = default;

void EoDlgSetupNote::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Text(dataExchange, IDC_TEXT_HEIGHT, height);
	DDX_Text(dataExchange, IDC_TEXT_EXP_FAC, widthFactor);
	DDX_Text(dataExchange, IDC_TEXT_INCLIN, obliqueAngle);
	DDX_Text(dataExchange, IDC_TEXT_ROTATION, rotationAngle);
	DDX_Control(dataExchange, IDC_MFCFONTCOMBO, mfcFontComboControl);
}

BOOL EoDlgSetupNote::OnInitDialog() {
	CDialog::OnInitDialog();
	mfcFontComboControl.Setup(TRUETYPE_FONTTYPE);
	mfcFontComboControl.AddString(L"Simplex.psf");
	mfcFontComboControl.SelectString(-1, fontDefinition->FontName());
	CString Spacing;
	Spacing.Format(L"%8.4f", fontDefinition->CharacterSpacing());
	SetDlgItemTextW(IDC_TEXT_SPACING, Spacing);
	CheckRadioButton(IDC_TEXT_ALIGN_HOR_LEFT, IDC_TEXT_ALIGN_HOR_RIGHT, static_cast<int>(IDC_TEXT_ALIGN_HOR_LEFT + fontDefinition->HorizontalAlignment() - 1));
	CheckRadioButton(IDC_TEXT_ALIGN_VER_BOT, IDC_TEXT_ALIGN_VER_TOP, static_cast<int>(IDC_TEXT_ALIGN_VER_BOT - fontDefinition->VerticalAlignment() + 4));
	CheckRadioButton(IDC_PATH_RIGHT, IDC_PATH_DOWN, static_cast<int>(IDC_PATH_RIGHT + fontDefinition->Path()));
	return TRUE;
}

void EoDlgSetupNote::OnOK() {
	CString Spacing;
	GetDlgItemTextW(IDC_TEXT_SPACING, Spacing);
	fontDefinition->SetCharacterSpacing(_wtof(Spacing));
	const auto HorizontalAlignment {EoDb::HorizontalAlignment(1 - IDC_TEXT_ALIGN_HOR_LEFT + GetCheckedRadioButton(IDC_TEXT_ALIGN_HOR_LEFT, IDC_TEXT_ALIGN_HOR_RIGHT))};
	fontDefinition->SetHorizontalAlignment(HorizontalAlignment);
	const auto VerticalAlignment {EoDb::VerticalAlignment(4 + IDC_TEXT_ALIGN_VER_BOT - GetCheckedRadioButton(IDC_TEXT_ALIGN_VER_BOT, IDC_TEXT_ALIGN_VER_TOP))};
	fontDefinition->SetVerticalAlignment(VerticalAlignment);
	const auto Path {EoDb::Path(GetCheckedRadioButton(IDC_PATH_RIGHT, IDC_PATH_DOWN) - IDC_PATH_RIGHT)};
	fontDefinition->SetPath(Path);
	const auto FontsIndex {mfcFontComboControl.GetCurSel()};
	if (FontsIndex != CB_ERR) {
		CString FontsItemName;
		mfcFontComboControl.GetLBText(FontsIndex, FontsItemName);
		fontDefinition->SetFontName(FontsItemName);
		const auto Precision {EoDb::Precision(FontsItemName.CompareNoCase(L"Simplex.psf") != 0 ? EoDb::kTrueType : EoDb::kStrokeType)};
		fontDefinition->SetPrecision(Precision);
	}
	CDialog::OnOK();
}
