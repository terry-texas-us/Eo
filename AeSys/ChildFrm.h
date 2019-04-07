#pragma once

class CChildFrame : public CMDIChildWndEx {
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

	virtual BOOL PreCreateWindow(CREATESTRUCT& createStructure) override;

	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const override;
	virtual void Dump(CDumpContext& dc) const override;
#endif

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void ActivateFrame(int nCmdShow = -1);
	virtual BOOL DestroyWindow();
	afx_msg void OnMDIActivate(BOOL activate, CWnd* activateWnd, CWnd* deactivateWnd);
	virtual void OnUpdateFrameMenu(BOOL bActive, CWnd* pActiveWnd, HMENU hMenuAlt) override;
};
