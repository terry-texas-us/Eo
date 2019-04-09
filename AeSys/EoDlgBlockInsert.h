#pragma once

// EoDlgBlockInsert dialog

class EoDlgBlockInsert : public CDialog {
	DECLARE_DYNAMIC(EoDlgBlockInsert)

public:
	EoDlgBlockInsert(CWnd* parent = NULL);
	EoDlgBlockInsert(AeSysDoc* document, CWnd* parent = NULL);
	~EoDlgBlockInsert();

// Dialog Data
	enum { IDD = IDD_INSERT_BLOCK };

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnOK() override;

	static OdGePoint3d InsertionPoint;
	AeSysDoc* m_Document;
public:
	CListBox m_BlocksListBoxControl;
	afx_msg void OnLbnSelchangeBlocksList();
	afx_msg void OnBnClickedPurge();

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCancel();
};
