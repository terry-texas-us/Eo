#include "stdafx.h"
#include "AeSys.h"
#include "EoDlgAudit.h"
#include <DbAudit.h>

EoDlgAudit::EoDlgAudit(CWnd* parent)
	: EoVarDialog(IDD, parent) {
}

void EoDlgAudit::DoDataExchange(CDataExchange* dataExchange) {
	EoVarDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_AUDITSUM_LIST, auditInfoList);
	DDX_Control(dataExchange, IDC_AUDITINFO_LIST, auditErrorList);
}

BEGIN_MESSAGE_MAP(EoDlgAudit, EoVarDialog)
END_MESSAGE_MAP()

BOOL EoDlgAudit::OnInitDialog() {
	EoVarDialog::OnInitDialog();
	auditInfoList.InsertColumn(0, L"Name", LVCFMT_LEFT, 400);
	auditInfoList.DeleteAllItems();
	auditErrorList.InsertColumn(0, L"Name", LVCFMT_LEFT, 100);
	auditErrorList.InsertColumn(1, L"Value", LVCFMT_LEFT, 100);
	auditErrorList.InsertColumn(2, L"Validation", LVCFMT_LEFT, 100);
	auditErrorList.InsertColumn(3, L"Default value", LVCFMT_LEFT, 100);
	auditErrorList.DeleteAllItems();
	initResizeHelper();
	m_resizeHelper.Fix(IDC_AUDITSUM_LIST, EoDialogResizeHelper::kLeftRight, EoDialogResizeHelper::kNoVFix);
	m_resizeHelper.Fix(IDC_AUDITINFO_LIST, EoDialogResizeHelper::kLeftRight, EoDialogResizeHelper::kNoVFix);
	m_resizeHelper.Fix(IDC_ST_AUDITERR, EoDialogResizeHelper::kLeft, EoDialogResizeHelper::kNoVFix);
	m_resizeHelper.Fix(IDC_ST_AUDITSUM, EoDialogResizeHelper::kLeft, EoDialogResizeHelper::kNoVFix);
	return TRUE;
}

void EoDlgAudit::PrintReport(OdDbAuditInfo* auditInfo) {
	if (auditInfo->getLastInfo().bIsError) {
		const auto NumberOfAuditErrors {auditErrorList.GetItemCount()};
		auditErrorList.InsertItem(NumberOfAuditErrors, auditInfo->getLastInfo().strName);
		auditErrorList.SetItemText(NumberOfAuditErrors, 1, auditInfo->getLastInfo().strValue);
		auditErrorList.SetItemText(NumberOfAuditErrors, 2, auditInfo->getLastInfo().strValidation);
		auditErrorList.SetItemText(NumberOfAuditErrors, 3, auditInfo->getLastInfo().strDefaultValue);
	} else {
		auditInfoList.InsertItem(auditInfoList.GetItemCount(), auditInfo->getLastInfo().strName);
	}
}

void EoDlgAudit::OnCancel() {
	DestroyWindow();
	delete this;
}
