#include "stdafx.h"
#include "AeSys.h"
#include "EoDlgPassword.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
EoDlgPassword::EoDlgPassword(CWnd* parent)
	: CDialog(IDD, parent) {
	m_sFileName = L"";
}

void EoDlgPassword::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_EDIT_PWD, m_pswCtrl);
	DDX_Text(dataExchange, IDC_PASS_FILE_NAME, m_sFileName);
	if (dataExchange->m_bSaveAndValidate) {
		CString s;
		m_pswCtrl.GetWindowTextW(s);
		m_password = s;
	}
}

BEGIN_MESSAGE_MAP(EoDlgPassword, CDialog)
END_MESSAGE_MAP()
