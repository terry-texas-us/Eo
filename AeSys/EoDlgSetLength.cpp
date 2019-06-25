#include "stdafx.h"
#include "AeSys.h"
#include "EoDlgSetLength.h"

IMPLEMENT_DYNAMIC(EoDlgSetLength, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetLength, CDialog)
END_MESSAGE_MAP()

EoDlgSetLength::EoDlgSetLength(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgSetLength::~EoDlgSetLength() = default;

void EoDlgSetLength::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
}

BOOL EoDlgSetLength::OnInitDialog() {
	CDialog::OnInitDialog();
	if (!title.IsEmpty()) {
		SetWindowTextW(title);
	}
	SetDlgItemTextW(IDC_DISTANCE, theApp.FormatLength(length, max(theApp.GetUnits(), AeSys::kEngineering)));
	return TRUE;
}

void EoDlgSetLength::OnOK() {
	wchar_t String[32];
	GetDlgItemTextW(IDC_DISTANCE, String, 32);
	length = AeSys::ParseLength(theApp.GetUnits(), String);
	CDialog::OnOK();
}
