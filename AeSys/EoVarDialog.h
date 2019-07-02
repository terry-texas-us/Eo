#pragma once
#include "EoDialogResizeHelper.h"
#include "EoGripperScrollBar.h"

class EoVarDialog : public CDialog {
public:
	EoVarDialog(const wchar_t* templateName, CWnd* parent = nullptr);

	EoVarDialog(unsigned templateId, CWnd* parent = nullptr);

protected:
	EoDialogResizeHelper m_resizeHelper;

	void initResizeHelper();

	BOOL OnInitDialog() override;

	void OnSize(unsigned type, int cx, int cy); // hides non-virtual function of parent
DECLARE_MESSAGE_MAP()

private:
	void SetupGripper();

	void MakeGripper();

	CPoint m_origSize;
	BOOL m_bInitialized {FALSE};
	EoGripperScrollBar m_Grip;
};
