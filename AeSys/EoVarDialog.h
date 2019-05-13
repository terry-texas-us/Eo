#pragma once

#include "EoDialogResizeHelper.h"
#include "EoGripperScrollBar.h"

class EoVarDialog : public CDialog {
public:
	EoVarDialog(LPCWSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	EoVarDialog(UINT nIDTemplate, CWnd* parent = NULL);

protected:
	EoDialogResizeHelper m_resizeHelper;
	void initResizeHelper();

	BOOL OnInitDialog() override;
	void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

private:
	void SetupGripper();
	void makeGripper();
	CPoint m_origSize;
	BOOL m_bInitialized;
	EoGripperScrollBar m_Grip;
};
