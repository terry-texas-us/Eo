#include "stdafx.h"
#include "AeSysApp.h"
#include "EoDlgPassword.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

EoDlgPassword::EoDlgPassword(CWnd* parent)
	: CDialog(EoDlgPassword::IDD, parent) {
	m_sFileName = L"";
}

void EoDlgPassword::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PWD, m_pswCtrl);
	DDX_Text(pDX, IDC_PASS_FILE_NAME, m_sFileName);
	if (pDX->m_bSaveAndValidate) {
		CString s;
		m_pswCtrl.GetWindowTextW(s);
		m_password = s;
	}
}

BEGIN_MESSAGE_MAP(EoDlgPassword, CDialog)
END_MESSAGE_MAP()