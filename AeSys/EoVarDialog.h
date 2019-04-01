#pragma once

#include "EoDialogResizeHelper.h"
#include "EoGripperScrollBar.h"

class EoVarDialog : public CDialog
{
	// Construction
public:
	EoVarDialog(LPCWSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	EoVarDialog(UINT nIDTemplate, CWnd* parent = NULL);

	// Implementation
protected:
	EoDialogResizeHelper m_resizeHelper;
	void initResizeHelper();

	// Generated message map functions
	//{{AFX_MSG(EoVarDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void SetupGripper();
	void makeGripper();
	CPoint m_origSize;
	BOOL m_bInitialized;
	EoGripperScrollBar m_Grip;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
