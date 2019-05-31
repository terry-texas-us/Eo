#include "stdafx.h"

#include "AeSys.h"
#include "EoDlgAudit.h"
#include "DbAudit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

EoDlgAudit::EoDlgAudit(CWnd* parent)
	: EoVarDialog(EoDlgAudit::IDD, parent) {
}

void EoDlgAudit::DoDataExchange(CDataExchange* pDX) {
	EoVarDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_AUDITSUM_LIST, m_AuditInfoList);
	DDX_Control(pDX, IDC_AUDITINFO_LIST, m_AuditErrList);
}

BEGIN_MESSAGE_MAP(EoDlgAudit, EoVarDialog)
END_MESSAGE_MAP()

BOOL EoDlgAudit::OnInitDialog() {
	EoVarDialog::OnInitDialog();

	m_AuditInfoList.InsertColumn(0, L"Name", LVCFMT_LEFT, 400);
	m_AuditInfoList.DeleteAllItems();

	m_AuditErrList.InsertColumn(0, L"Name", LVCFMT_LEFT, 100);
	m_AuditErrList.InsertColumn(1, L"Value", LVCFMT_LEFT, 100);
	m_AuditErrList.InsertColumn(2, L"Validation", LVCFMT_LEFT, 100);
	m_AuditErrList.InsertColumn(3, L"Default value", LVCFMT_LEFT, 100);
	m_AuditErrList.DeleteAllItems();

	initResizeHelper();
	m_resizeHelper.Fix(IDC_AUDITSUM_LIST, EoDialogResizeHelper::kLeftRight, EoDialogResizeHelper::kNoVFix);
	m_resizeHelper.Fix(IDC_AUDITINFO_LIST, EoDialogResizeHelper::kLeftRight, EoDialogResizeHelper::kNoVFix);
	m_resizeHelper.Fix(IDC_ST_AUDITERR, EoDialogResizeHelper::kLeft, EoDialogResizeHelper::kNoVFix);
	m_resizeHelper.Fix(IDC_ST_AUDITSUM, EoDialogResizeHelper::kLeft, EoDialogResizeHelper::kNoVFix);

	return TRUE;
}
void EoDlgAudit::printReport(OdDbAuditInfo* auditInfo) {
	if (auditInfo->getLastInfo().bIsError) {
		const int NumberOfAuditErrors = m_AuditErrList.GetItemCount();
		m_AuditErrList.InsertItem(NumberOfAuditErrors, auditInfo->getLastInfo().strName);
		m_AuditErrList.SetItemText(NumberOfAuditErrors, 1, auditInfo->getLastInfo().strValue);
		m_AuditErrList.SetItemText(NumberOfAuditErrors, 2, auditInfo->getLastInfo().strValidation);
		m_AuditErrList.SetItemText(NumberOfAuditErrors, 3, auditInfo->getLastInfo().strDefaultValue);
	} else {
		m_AuditInfoList.InsertItem(m_AuditInfoList.GetItemCount(), auditInfo->getLastInfo().strName);
	}
}
void EoDlgAudit::OnCancel() {
	DestroyWindow();
	delete this;
}