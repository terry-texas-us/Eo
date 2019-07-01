#pragma once
class EoMfOutputListBox final : public CListBox {
protected:
	void OnContextMenu(CWnd* pWnd, CPoint point); // hides non-virtual function of parent
	void OnEditCopy() noexcept;

	void OnEditClear() noexcept;

	void OnViewOutput();

DECLARE_MESSAGE_MAP()
};

class EoMfOutputDockablePane final : public CDockablePane {
protected:
	CFont m_Font;
	CMFCTabCtrl m_wndTabs;
	EoMfOutputListBox m_OutputMessagesList;
	EoMfOutputListBox m_OutputReportsList;
public:
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
	int OnCreate(LPCREATESTRUCT createStructure); // hides non-virtual function of parent
	void OnSize(unsigned type, int cx, int cy); // hides non-virtual function of parent
DECLARE_MESSAGE_MAP()
};
