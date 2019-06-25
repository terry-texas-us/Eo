#include "stdafx.h"
#include "EoDlgSetText.h"

IMPLEMENT_DYNAMIC(EoDlgSetText, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetText, CDialog)
END_MESSAGE_MAP()

EoDlgSetText::EoDlgSetText(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgSetText::~EoDlgSetText() = default;

void EoDlgSetText::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Text(dataExchange, IDC_TEXT, text);
}

BOOL EoDlgSetText::OnInitDialog() {
	if (!title.IsEmpty()) {
		SetWindowTextW(title);
	}
	CDialog::OnInitDialog();
	return TRUE;
}
