#pragma once

#include "EoVarDialog.h"

class OdDbAuditInfo;

class EoDlgAudit : public EoVarDialog {
public:
	EoDlgAudit(CWnd* parent = nullptr);

	void printReport (OdDbAuditInfo* auditInfo);
	void OnCancel() override;

	enum { IDD = IDD_AUDITINFO };

	CListCtrl m_AuditInfoList;
	CListCtrl m_AuditErrList;

protected:
	void DoDataExchange(CDataExchange* pDX) final;
    BOOL OnInitDialog() final;

	DECLARE_MESSAGE_MAP()
};
