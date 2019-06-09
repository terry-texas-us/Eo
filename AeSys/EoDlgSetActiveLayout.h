#pragma once

#include "DbDatabase.h"

class EoDlgSetActiveLayout : public CDialog {
	OdDbDatabase* m_Database;
	int m_OldActiveLayout;

public:
	int m_NewActiveLayout;
	bool m_CreateNewLayout;
	CString m_NewLayoutName;

	EoDlgSetActiveLayout(OdDbDatabase* database, CWnd* parent = nullptr);
	void FillListBox();

	enum { IDD = IDD_SET_ACTIVE_LAYOUT };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;

protected:
	void OnLayoutDlgClose();
	void OnSelchangeLayoutlist();
	void OnDblclkLayoutlist();
	void OnRename();
	void OnDelete();
	void OnCopy();
	void OnNew();
	void OnFromTemplate();

	DECLARE_MESSAGE_MAP()
};
