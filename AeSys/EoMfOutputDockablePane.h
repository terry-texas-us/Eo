#pragma once

class EoMfOutputListBox : public CListBox {
public:
    EoMfOutputListBox();

public:
    virtual ~EoMfOutputListBox();

protected:
    void OnContextMenu(CWnd* pWnd, CPoint point);
    void OnEditCopy() noexcept;
    void OnEditClear() noexcept;
    void OnViewOutput();
    
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
    int OnCreate(LPCREATESTRUCT createStructure);
    void OnSize(UINT type, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};
