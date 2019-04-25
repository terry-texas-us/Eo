#pragma once

#include "DbDatabase.h"

class EoDlgSetActiveLayout : public CDialog {
	OdDbDatabase* m_pDb;
	int m_nOldActiveLayout;

public:
	int m_nNewActiveLayout;
	bool m_bCreateNewLayout;
	CString m_sNewLayoutName;

	EoDlgSetActiveLayout(OdDbDatabase* database, CWnd* parent = NULL);
	void FillListBox();

	enum { IDD = IDD_SET_ACTIVE_LAYOUT };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;

protected:
	afx_msg void OnLayoutDlgClose();
	afx_msg void OnSelchangeLayoutlist();
	afx_msg void OnDblclkLayoutlist();
	afx_msg void OnRename();
	afx_msg void OnDelete();
	afx_msg void OnCopy();
	afx_msg void OnNew();
	afx_msg void OnFromTemplate();

	DECLARE_MESSAGE_MAP()
};
