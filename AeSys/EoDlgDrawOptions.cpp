#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDlgDrawOptions.h"

// EoDlgDrawOptions dialog
IMPLEMENT_DYNAMIC(EoDlgDrawOptions, CDialog)

BEGIN_MESSAGE_MAP(EoDlgDrawOptions, CDialog)
		ON_BN_CLICKED(IDC_PEN, &EoDlgDrawOptions::OnBnClickedPen)
		ON_BN_CLICKED(IDC_LINE, &EoDlgDrawOptions::OnBnClickedLine)
		ON_BN_CLICKED(IDC_TEXT, &EoDlgDrawOptions::OnBnClickedText)
		ON_BN_CLICKED(IDC_FILL, &EoDlgDrawOptions::OnBnClickedFill)
		ON_BN_CLICKED(IDC_CONSTRAINTS, &EoDlgDrawOptions::OnBnClickedConstraints)
END_MESSAGE_MAP()

EoDlgDrawOptions::EoDlgDrawOptions(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgDrawOptions::~EoDlgDrawOptions() {
}

void EoDlgDrawOptions::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

BOOL EoDlgDrawOptions::OnInitDialog() {
	CDialog::OnInitDialog();
	SendDlgItemMessageW(IDC_ARC_3_POINT, BM_SETCHECK, 1, 0L);
	SendDlgItemMessageW(IDC_CURVE_SPLINE, BM_SETCHECK, 1, 0L);
	return TRUE;
}

void EoDlgDrawOptions::OnOK() {
	CDialog::OnOK();
}

void EoDlgDrawOptions::OnBnClickedPen() {
	AeSysDoc::GetDoc()->OnSetupPenColor();
	CDialog::OnOK();
}

void EoDlgDrawOptions::OnBnClickedLine() {
	AeSysDoc::GetDoc()->OnSetupLinetype();
	CDialog::OnOK();
}

void EoDlgDrawOptions::OnBnClickedText() {
	AeSysDoc::GetDoc()->OnSetupNote();
	CDialog::OnOK();
}

void EoDlgDrawOptions::OnBnClickedFill() {
	AeSysDoc::GetDoc()->OnSetupFillHatch();
	CDialog::OnOK();
}

void EoDlgDrawOptions::OnBnClickedConstraints() {
	AeSysView::GetActiveView()->OnSetupConstraints();
	CDialog::OnOK();
}
