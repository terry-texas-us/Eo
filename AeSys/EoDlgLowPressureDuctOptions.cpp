#include "stdafx.h"
#include "AeSys.h"
#include "AeSysView.h"
#include "EoDlgLowPressureDuctOptions.h"

IMPLEMENT_DYNAMIC(EoDlgLowPressureDuctOptions, CDialog)

BEGIN_MESSAGE_MAP(EoDlgLowPressureDuctOptions, CDialog)
		ON_BN_CLICKED(IDOK, &EoDlgLowPressureDuctOptions::OnBnClickedOk)
		ON_BN_CLICKED(IDC_GEN_VANES, &EoDlgLowPressureDuctOptions::OnBnClickedGenVanes)
END_MESSAGE_MAP()

EoDlgLowPressureDuctOptions::EoDlgLowPressureDuctOptions(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgLowPressureDuctOptions::~EoDlgLowPressureDuctOptions() = default;

void EoDlgLowPressureDuctOptions::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Text(dataExchange, IDC_RADIUS_FACTOR, radiusFactor);
}

BOOL EoDlgLowPressureDuctOptions::OnInitDialog() {
	CDialog::OnInitDialog();
	const auto Units {max(theApp.GetUnits(), AeSys::kInches)};
	SetDlgItemTextW(IDC_WIDTH, theApp.FormatLength(width, Units, 12, 3));
	SetDlgItemTextW(IDC_DEPTH, theApp.FormatLength(depth, Units, 12, 3));
	CheckRadioButton(IDC_LEFT, IDC_RIGHT, IDC_CENTER + justification);
	CheckDlgButton(IDC_GEN_VANES, generateVanes ? 1u : 0u);
	CheckDlgButton(IDC_BEGINWITHTRANSITION, beginWithTransition ? 1u : 0u);
	return TRUE;
}

// EoDlgLowPressureDuctOptions message handlers
void EoDlgLowPressureDuctOptions::OnBnClickedOk() {
	wchar_t String[32];
	GetDlgItemTextW(IDC_WIDTH, String, 32);
	width = AeSys::ParseLength(theApp.GetUnits(), String);
	GetDlgItemTextW(IDC_DEPTH, String, 32);
	depth = AeSys::ParseLength(theApp.GetUnits(), String);
	justification = GetCheckedRadioButton(IDC_LEFT, IDC_RIGHT) - IDC_CENTER;
	generateVanes = IsDlgButtonChecked(IDC_GEN_VANES) == 0 ? false : true;
	beginWithTransition = IsDlgButtonChecked(IDC_BEGINWITHTRANSITION) == 0 ? false : true;
	OnOK();
}

void EoDlgLowPressureDuctOptions::OnBnClickedGenVanes() noexcept {
	// <tas="No implementation for event OnBnClickedGenVanes"</tas>
}
