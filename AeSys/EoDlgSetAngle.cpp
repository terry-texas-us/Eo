#include "stdafx.h"
#include "EoDlgSetAngle.h"

IMPLEMENT_DYNAMIC(EoDlgSetAngle, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetAngle, CDialog)
END_MESSAGE_MAP()

EoDlgSetAngle::EoDlgSetAngle(CWnd* parent)
	: CDialog(IDD, parent) {
}

void EoDlgSetAngle::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Text(dataExchange, IDC_ANGLE, angle);
	DDV_MinMaxDouble(dataExchange, angle, -360.0, 360.0);
}

BOOL EoDlgSetAngle::OnInitDialog() {
	CDialog::OnInitDialog();
	if (!title.IsEmpty()) { SetWindowTextW(title); }
	return TRUE;
}
