#pragma once

class CChildFrame : public CMDIChildWndEx {
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame() noexcept;

	BOOL PreCreateWindow(CREATESTRUCT& createStructure) override;

	~CChildFrame();
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	void ActivateFrame(int nCmdShow = -1) override;
	BOOL DestroyWindow() override;
	void OnMDIActivate(BOOL activate, CWnd* activateWnd, CWnd* deactivateWnd);
	void OnUpdateFrameMenu(BOOL bActive, CWnd* pActiveWnd, HMENU hMenuAlt) override;
};
