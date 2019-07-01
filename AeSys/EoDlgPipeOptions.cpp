#include "stdafx.h"
#include "EoDlgPipeOptions.h"
IMPLEMENT_DYNAMIC(EoDlgPipeOptions, CDialog)

BEGIN_MESSAGE_MAP(EoDlgPipeOptions, CDialog)
END_MESSAGE_MAP()

EoDlgPipeOptions::EoDlgPipeOptions(CWnd* parent)
	: CDialog(IDD, parent) {
}

void EoDlgPipeOptions::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
}

BOOL EoDlgPipeOptions::OnInitDialog() {
	CDialog::OnInitDialog();
	return TRUE;
}

void EoDlgPipeOptions::OnOK() {
	CDialog::OnOK();
}
