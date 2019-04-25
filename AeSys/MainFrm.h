#pragma once

#include "EoMfOutputDockablePane.h"
#include "EoMfPropertiesDockablePane.h"

const int nStatusIcon = 0;
const int nStatusInfo = 1;
const int nStatusProgress = 2;
const int nStatusOp0 = 3;

class CMainFrame : public CMDIFrameWndEx {
	LARGE_INTEGER m_pc0;
	LARGE_INTEGER m_pc1;

	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

	inline void StartTimer() noexcept {
		QueryPerformanceCounter(&m_pc0);
	}
	inline void StopTimer(LPCWSTR operationName = NULL) {
		QueryPerformanceCounter(&m_pc1);
		m_pc1.QuadPart -= m_pc0.QuadPart;
		if (QueryPerformanceFrequency(&m_pc0)) {
			const double loadTime = ((double) m_pc1.QuadPart) / ((double) m_pc0.QuadPart);
			OdString NewText;
            OdString OperationName {operationName ? operationName : L"Operation"};
			NewText.format(L"%s Time: %.6f sec.", OperationName, loadTime);
			SetStatusPaneTextAt(wcscmp(L"Redraw", OperationName) == 0 ? nStatusProgress : nStatusInfo, NewText);
		}
	}

	void SetStatusPaneTextAt(int index, LPCWSTR newText);

	void SetStatusPaneTextColorAt(int index, COLORREF textColor = COLORREF(- 1));

private:
	UINT m_ApplicationLook;
	int m_CurrentProgress;
	bool m_InProgress;

public:
	void UpdateMDITabs(BOOL resetMDIChild);

public:
	BOOL PreCreateWindow(CREATESTRUCT& createStructure) override;
	BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL) override;
	virtual ~CMainFrame();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

public:
	static void DrawColorBox(CDC& deviceContext, const RECT& itemRectangle, const OdCmColor& color);
	static void DrawLineWeight(CDC& deviceContext, const RECT& itemRectangle, const OdDb::LineWeight lineWeight);
	static void DrawPlotStyle(CDC& deviceContext, const RECT& itemRectangle, const CString& textOut, const OdDbDatabasePtr& database);
	static CMFCToolBarComboBoxButton* GetFindCombo(void);
	static HTREEITEM InsertTreeViewControlItem(HWND tree, HTREEITEM parent, LPWSTR text, LPCVOID object) noexcept;
	static OdDb::LineWeight LineWeightByIndex(char lineWeight) noexcept;
	static CString StringByLineWeight(int lineWeight, bool lineWeightByIndex);

protected:  // control bar embedded members
	CMFCMenuBar m_MenuBar;
	CMFCToolBar m_StandardToolBar;
	CMFCStatusBar m_StatusBar;
	EoMfOutputDockablePane m_OutputPane;
	EoMfPropertiesDockablePane m_PropertiesPane;
	CMFCToolBarImages m_UserImages;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT createStructure);
	afx_msg void OnDestroy();
	afx_msg void OnWindowManager();
	afx_msg void OnMdiTabbed();
	afx_msg void OnUpdateMdiTabbed(CCmdUI* pCmdUI);
	afx_msg void OnViewCustomize(void);
	afx_msg void OnViewFullScreen(void);
	afx_msg LRESULT OnToolbarContextMenu(WPARAM,LPARAM);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);

	afx_msg LRESULT OnGetTabToolTip(WPARAM wp, LPARAM lp);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnToolbarReset(WPARAM toolbarResourceId, LPARAM lparam);
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);

	DECLARE_MESSAGE_MAP()

	BOOL OnShowPopupMenu(CMFCPopupMenu* pMenuPopup) override;
	BOOL OnShowMDITabContextMenu(CPoint point, DWORD dwAllowedItems, BOOL bDrop) override;

	/// <summary></summary>
	/// <remarks>
	// CBRS_FLOAT_MULTI allows panes to float together in a single window. By default, panes only float individually.
	/// </remarks>
	BOOL CreateDockablePanes();
	void SetDockablePanesIcons(bool highColorMode);
	void ShowAnnotationScalesPopupMenu(CMFCPopupMenu* popupMenu);
	void ShowRegisteredCommandsPopupMenu(CMFCPopupMenu* popupMenu);

public:
	void OnStartProgress(void);

	CMFCStatusBar& GetStatusBar(void) noexcept {
		return m_StatusBar;
	}
	EoMfOutputDockablePane& GetOutputPane(void) noexcept {
		return m_OutputPane;
	}
	EoMfPropertiesDockablePane& GetPropertiesPane(void) noexcept {
		return m_PropertiesPane;
	}
};