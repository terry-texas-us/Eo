#pragma once

// EoDlgEditTrapCommandsQuery dialog

class EoDlgEditTrapCommandsQuery : public CDialog {
	DECLARE_DYNAMIC(EoDlgEditTrapCommandsQuery)

public:
	EoDlgEditTrapCommandsQuery(CWnd* parent = NULL);
	virtual ~EoDlgEditTrapCommandsQuery();

// Dialog Data
	enum { IDD = IDD_EDIT_TRAPCOMMANDS_QUERY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	CTreeCtrl m_GroupTreeViewControl;
	CListCtrl m_GeometryListViewControl;
	CListCtrl m_ExtraListViewControl;

public:
	void FillExtraList(EoDbPrimitive* primitive);
	void FillGeometryList(EoDbPrimitive* primitive);

	afx_msg void OnTvnSelchangedGroupTree(NMHDR *pNMHDR, LRESULT* result);

protected:
	DECLARE_MESSAGE_MAP()
};
