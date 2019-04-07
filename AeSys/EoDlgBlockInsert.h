#pragma once

// EoDlgBlockInsert dialog

class EoDlgBlockInsert : public CDialog {
	DECLARE_DYNAMIC(EoDlgBlockInsert)

public:
	EoDlgBlockInsert(CWnd* parent = NULL);
	EoDlgBlockInsert(AeSysDoc* document, CWnd* parent = NULL);
	virtual ~EoDlgBlockInsert();

// Dialog Data
	enum { IDD = IDD_INSERT_BLOCK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

	static OdGePoint3d InsertionPoint;
	AeSysDoc* m_Document;
public:
	CListBox m_BlocksListBoxControl;
	afx_msg void OnLbnSelchangeBlocksList();
	afx_msg void OnBnClickedPurge();

protected:
	DECLARE_MESSAGE_MAP()
};
