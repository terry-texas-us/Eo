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
			const auto loadTime {static_cast<double>(m_pc1.QuadPart) / static_cast<double>(m_pc0.QuadPart)};
			OdString NewText;
			auto OperationName {operationName ? operationName : L"Operation"};
			NewText.format(L"%s Time: %.6f sec.", OperationName, loadTime);
			SetStatusPaneTextAt(wcscmp(L"Redraw", OperationName) == 0 ? nStatusProgress : nStatusInfo, NewText);
		}
	}

	void SetStatusPaneTextAt(int index, LPCWSTR newText);

	void SetStatusPaneTextColorAt(int index, COLORREF textColor = COLORREF(-1));

private:
	int m_CurrentProgress;
	bool m_InProgress;

public:
	void UpdateMDITabs(BOOL resetMDIChild);

public:
	BOOL PreCreateWindow(CREATESTRUCT& createStructure) override;
	BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL) override;
	~CMainFrame();
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
	static OdString StringByLineWeight(int lineWeight, bool lineWeightByIndex);

protected:  // control bar embedded members
	CMFCMenuBar m_MenuBar;
	CMFCToolBar m_StandardToolBar;
	CMFCStatusBar m_StatusBar;
	EoMfOutputDockablePane m_OutputPane;
	EoMfPropertiesDockablePane m_PropertiesPane;
	CMFCToolBarImages m_UserImages;

protected:
	int OnCreate(LPCREATESTRUCT createStructure);
	void OnDestroy();
	void OnWindowManager();
	void OnMdiTabbed();
	void OnUpdateMdiTabbed(CCmdUI* pCmdUI);
	void OnViewCustomize(void);
	void OnViewFullScreen(void);
	LRESULT OnToolbarContextMenu(WPARAM, LPARAM);
	void OnApplicationLook(UINT id);
	void OnUpdateApplicationLook(CCmdUI* pCmdUI);

	LRESULT OnGetTabToolTip(WPARAM wp, LPARAM lp);
	void OnTimer(UINT_PTR nIDEvent);
	LRESULT OnToolbarReset(WPARAM toolbarResourceId, LPARAM lparam);
	LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);

	DECLARE_MESSAGE_MAP()

	BOOL OnShowPopupMenu(CMFCPopupMenu* pMenuPopup) override;
	BOOL OnShowMDITabContextMenu(CPoint point, DWORD dwAllowedItems, BOOL bDrop) override;

	BOOL CreateDockingWindows();
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