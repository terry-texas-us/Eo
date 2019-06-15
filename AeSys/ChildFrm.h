#pragma once

class CChildFrame : public CMDIChildWndEx {
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame() = default;

	BOOL PreCreateWindow(CREATESTRUCT& createStructure) override;

	~CChildFrame() = default;
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
	void OnUpdateFrameMenu(BOOL bActive, CWnd* pActiveWnd, HMENU hMenuAlt) override;

	void OnUpdateFrameMenu(HMENU hMenuAlt) override {} // CFrameWnd (to suppress C4266 warning)

	void OnMDIActivate(BOOL activate, CWnd* activateWnd, CWnd* deactivateWnd); // hides non-virtual function of parent
};
