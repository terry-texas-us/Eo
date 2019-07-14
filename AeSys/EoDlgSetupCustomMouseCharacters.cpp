#include "stdafx.h"
#include "AeSys.h"
#include "EoDlgSetupCustomMouseCharacters.h"
IMPLEMENT_DYNAMIC(EoDlgSetupCustomMouseCharacters, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetupCustomMouseCharacters, CDialog)
END_MESSAGE_MAP()

EoDlgSetupCustomMouseCharacters::EoDlgSetupCustomMouseCharacters(CWnd* parent)
	: CDialog(IDD, parent) {}

void EoDlgSetupCustomMouseCharacters::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
}

BOOL EoDlgSetupCustomMouseCharacters::OnInitDialog() {
	CDialog::OnInitDialog();
	SetDlgItemTextW(IDC_LEFT_DOWN, AeSys::customLButtonDownCharacters);
	SetDlgItemTextW(IDC_LEFT_UP, AeSys::customLButtonUpCharacters);
	SetDlgItemTextW(IDC_RIGHT_DOWN, AeSys::customRButtonDownCharacters);
	SetDlgItemTextW(IDC_RIGHT_UP, AeSys::customRButtonUpCharacters);
	return TRUE;
}

void EoDlgSetupCustomMouseCharacters::OnOK() {
	GetDlgItemTextW(IDC_LEFT_DOWN, AeSys::customLButtonDownCharacters);
	GetDlgItemTextW(IDC_LEFT_UP, AeSys::customLButtonUpCharacters);
	GetDlgItemTextW(IDC_RIGHT_DOWN, AeSys::customRButtonDownCharacters);
	GetDlgItemTextW(IDC_RIGHT_UP, AeSys::customRButtonUpCharacters);
	CDialog::OnOK();
}
