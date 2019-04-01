#include "stdafx.h"
#include "AeSysApp.h"

#include "EoDlgSetLength.h"

// EoDlgSetLength dialog

IMPLEMENT_DYNAMIC(EoDlgSetLength, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetLength, CDialog)
END_MESSAGE_MAP()

EoDlgSetLength::EoDlgSetLength(CWnd* parent) 
    : CDialog(EoDlgSetLength::IDD, parent)
    , m_Length(0.) {
}

EoDlgSetLength::~EoDlgSetLength() {
}

void EoDlgSetLength::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}
BOOL EoDlgSetLength::OnInitDialog() {
	CDialog::OnInitDialog();
	if (!m_Title.IsEmpty()) {
		SetWindowTextW(m_Title);
	}
	SetDlgItemTextW(IDC_DISTANCE, theApp.FormatLength(m_Length, max(theApp.GetUnits(), AeSysApp::kEngineering)));
	return TRUE;
}
void EoDlgSetLength::OnOK() {
	wchar_t szBuf[32];

	GetDlgItemTextW(IDC_DISTANCE, (LPWSTR) szBuf, 32);
	m_Length = theApp.ParseLength(theApp.GetUnits(), szBuf);

	CDialog::OnOK();
}
