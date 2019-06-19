#include "stdafx.h"

#include "AeSys.h"

#include "EoDlgSetupCustomMouseCharacters.h"

// EoDlgSetupCustomMouseCharacters dialog

IMPLEMENT_DYNAMIC(EoDlgSetupCustomMouseCharacters, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetupCustomMouseCharacters, CDialog)
END_MESSAGE_MAP()

EoDlgSetupCustomMouseCharacters::EoDlgSetupCustomMouseCharacters(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgSetupCustomMouseCharacters::~EoDlgSetupCustomMouseCharacters() {
}

void EoDlgSetupCustomMouseCharacters::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

BOOL EoDlgSetupCustomMouseCharacters::OnInitDialog() {
	CDialog::OnInitDialog();

	SetDlgItemTextW(IDC_LEFT_DOWN, theApp.CustomLButtonDownCharacters);
	SetDlgItemTextW(IDC_LEFT_UP, theApp.CustomLButtonUpCharacters);
	SetDlgItemTextW(IDC_RIGHT_DOWN, theApp.CustomRButtonDownCharacters);
	SetDlgItemTextW(IDC_RIGHT_UP, theApp.CustomRButtonUpCharacters);

	return TRUE;
}

void EoDlgSetupCustomMouseCharacters::OnOK() {
	GetDlgItemTextW(IDC_LEFT_DOWN, theApp.CustomLButtonDownCharacters);
	GetDlgItemTextW(IDC_LEFT_UP, theApp.CustomLButtonUpCharacters);
	GetDlgItemTextW(IDC_RIGHT_DOWN, theApp.CustomRButtonDownCharacters);
	GetDlgItemTextW(IDC_RIGHT_UP, theApp.CustomRButtonUpCharacters);

	CDialog::OnOK();
}
