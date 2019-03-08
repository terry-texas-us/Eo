#pragma once

class EoMfOutputListBox : public CListBox {
public:
	EoMfOutputListBox();

public:
	virtual ~EoMfOutputListBox();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
};

class EoMfOutputDockablePane : public CDockablePane {
public:
	EoMfOutputDockablePane();

protected:
	CFont m_Font;
	CMFCTabCtrl	m_wndTabs;

	EoMfOutputListBox m_OutputMessagesList;
	EoMfOutputListBox m_OutputReportsList;

public:
	virtual ~EoMfOutputDockablePane();
	void ModifyCaption(const CString& string) {
		SetWindowTextW(string);
	}
	void AddStringToMessageList(const CString& string) {
		m_OutputMessagesList.InsertString(0, string);
	}
	void AddStringToReportsList(const CString& string) {
		m_OutputReportsList.AddString(string);
	}

protected:
	afx_msg int OnCreate(LPCREATESTRUCT createStructure);
	afx_msg void OnSize(UINT type, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
