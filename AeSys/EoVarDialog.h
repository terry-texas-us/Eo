#pragma once

#include "EoDialogResizeHelper.h"
#include "EoGripperScrollBar.h"

class EoVarDialog : public CDialog {
public:
	EoVarDialog(LPCWSTR lpszTemplateName, CWnd* pParentWnd = nullptr);
	EoVarDialog(unsigned templateId, CWnd* parent = nullptr);

protected:
	EoDialogResizeHelper m_resizeHelper;
	void initResizeHelper();

	BOOL OnInitDialog() override;
	void OnSize(unsigned type, int cx, int cy);

	DECLARE_MESSAGE_MAP()

private:
	void SetupGripper();
	void makeGripper();
	CPoint m_origSize;
	BOOL m_bInitialized;
	EoGripperScrollBar m_Grip;
};
