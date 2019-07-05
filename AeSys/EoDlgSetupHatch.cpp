#include "stdafx.h"
#include "PrimState.h"
#include "EoDlgSetupHatch.h"
IMPLEMENT_DYNAMIC(EoDlgSetupHatch, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetupHatch, CDialog)
END_MESSAGE_MAP()

EoDlgSetupHatch::EoDlgSetupHatch(CWnd* parent) noexcept
	: CDialog(IDD, parent) {
}

EoDlgSetupHatch::~EoDlgSetupHatch() = default;

void EoDlgSetupHatch::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Text(dataExchange, IDC_FIL_AREA_HAT_X_SCAL, hatchXScaleFactor);
	DDX_Text(dataExchange, IDC_FIL_AREA_HAT_Y_SCAL, hatchYScaleFactor);
	DDX_Text(dataExchange, IDC_FIL_AREA_HAT_ROT_ANG, hatchRotationAngle);
}

BOOL EoDlgSetupHatch::OnInitDialog() {
	CDialog::OnInitDialog();
	SetDlgItemInt(IDC_FIL_AREA_HAT_ID, g_PrimitiveState.HatchInteriorStyleIndex(), FALSE);
	return TRUE;
}

void EoDlgSetupHatch::OnOK() {
	g_PrimitiveState.SetHatchInteriorStyleIndex(GetDlgItemInt(IDC_FIL_AREA_HAT_ID, nullptr, FALSE));
	CDialog::OnOK();
}
