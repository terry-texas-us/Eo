#include "stdafx.h"

#include "EoDlgSetText.h"

// EoDlgSetText dialog

IMPLEMENT_DYNAMIC(EoDlgSetText, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetText, CDialog)
END_MESSAGE_MAP()

EoDlgSetText::EoDlgSetText(CWnd* parent)
	: CDialog(EoDlgSetText::IDD, parent) {
}

EoDlgSetText::~EoDlgSetText() {
}

void EoDlgSetText::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TEXT, m_sText);
}

BOOL EoDlgSetText::OnInitDialog() {
	if (!m_strTitle.IsEmpty()) {
		SetWindowTextW(m_strTitle);
	}
	CDialog::OnInitDialog();
	return TRUE;
}

