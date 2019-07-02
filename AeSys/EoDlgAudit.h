#pragma once
#include "EoVarDialog.h"
class OdDbAuditInfo;

class EoDlgAudit final : public EoVarDialog {
public:
	EoDlgAudit(CWnd* parent = nullptr);

	void PrintReport(OdDbAuditInfo* auditInfo);

	void OnCancel() override;

	enum { IDD = IDD_AUDITINFO };

	CListCtrl auditInfoList;
	CListCtrl auditErrorList;
protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

DECLARE_MESSAGE_MAP()
};
