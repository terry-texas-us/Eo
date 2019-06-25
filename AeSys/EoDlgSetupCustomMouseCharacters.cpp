#include "stdafx.h"
#include "AeSys.h"
#include "EoDlgSetupCustomMouseCharacters.h"

IMPLEMENT_DYNAMIC(EoDlgSetupCustomMouseCharacters, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetupCustomMouseCharacters, CDialog)
END_MESSAGE_MAP()

EoDlgSetupCustomMouseCharacters::EoDlgSetupCustomMouseCharacters(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgSetupCustomMouseCharacters::~EoDlgSetupCustomMouseCharacters() = default;

void EoDlgSetupCustomMouseCharacters::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
}

BOOL EoDlgSetupCustomMouseCharacters::OnInitDialog() {
	CDialog::OnInitDialog();
	SetDlgItemTextW(IDC_LEFT_DOWN, AeSys::CustomLButtonDownCharacters);
	SetDlgItemTextW(IDC_LEFT_UP, AeSys::CustomLButtonUpCharacters);
	SetDlgItemTextW(IDC_RIGHT_DOWN, AeSys::CustomRButtonDownCharacters);
	SetDlgItemTextW(IDC_RIGHT_UP, AeSys::CustomRButtonUpCharacters);
	return TRUE;
}

void EoDlgSetupCustomMouseCharacters::OnOK() {
	GetDlgItemTextW(IDC_LEFT_DOWN, AeSys::CustomLButtonDownCharacters);
	GetDlgItemTextW(IDC_LEFT_UP, AeSys::CustomLButtonUpCharacters);
	GetDlgItemTextW(IDC_RIGHT_DOWN, AeSys::CustomRButtonDownCharacters);
	GetDlgItemTextW(IDC_RIGHT_UP, AeSys::CustomRButtonUpCharacters);
	CDialog::OnOK();
}
