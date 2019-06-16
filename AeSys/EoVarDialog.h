#pragma once

#include "EoDialogResizeHelper.h"
#include "EoGripperScrollBar.h"

class EoVarDialog : public CDialog {
public:
	EoVarDialog(const wchar_t* templateName, CWnd* parentWindow = nullptr);
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
	BOOL m_bInitialized {FALSE};
	EoGripperScrollBar m_Grip;
};
