#pragma once

#include "EoVarDialog.h"

class OdDbAuditInfo;

class EoDlgAudit : public EoVarDialog {
public:
	EoDlgAudit(CWnd* parent = NULL);

	void printReport (OdDbAuditInfo* auditInfo);
	void OnCancel();

	enum { IDD = IDD_AUDITINFO };

	CListCtrl m_AuditInfoList;
	CListCtrl m_AuditErrList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
