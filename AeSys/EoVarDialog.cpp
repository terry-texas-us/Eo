#include "stdafx.h"
#include "EoVarDialog.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
EoVarDialog::EoVarDialog(const wchar_t* templateName, CWnd* parent)
	: CDialog(templateName, parent) {}

EoVarDialog::EoVarDialog(unsigned templateId, CWnd* parent)
	: CDialog(templateId, parent) {}
#pragma warning(push)
#pragma warning(disable : 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
BEGIN_MESSAGE_MAP(EoVarDialog, CDialog)
		ON_WM_SIZE()
END_MESSAGE_MAP()
#pragma warning (pop)
BOOL EoVarDialog::OnInitDialog() {
	CDialog::OnInitDialog();
	m_bInitialized = TRUE;
	return TRUE;
}

void EoVarDialog::initResizeHelper() {
	CRect WindowRect;
	GetWindowRect(WindowRect);
	m_origSize = CPoint(WindowRect.Width(), WindowRect.Height());
	m_resizeHelper.Init(m_hWnd);
	MakeGripper();
}

void EoVarDialog::MakeGripper() {
	CRect InitialRect;
	GetClientRect(InitialRect);
	InitialRect.left = InitialRect.right - GetSystemMetrics(SM_CXHSCROLL);
	InitialRect.top = InitialRect.bottom - GetSystemMetrics(SM_CYVSCROLL);
	const unsigned long Style {WS_CHILD | SBS_SIZEBOX | SBS_SIZEBOXBOTTOMRIGHTALIGN | SBS_SIZEGRIP | WS_VISIBLE};
	m_Grip.Create(Style, InitialRect, this, AFX_IDW_SIZE_BOX);
}

void EoVarDialog::OnSize(const unsigned type, const int cx, const int cy) {
	CDialog::OnSize(type, cx, cy);
	m_resizeHelper.OnSize();
	if (m_bInitialized != 0) {
		SetupGripper();
	}
}

void EoVarDialog::SetupGripper() {
	WINDOWPLACEMENT WindowPlacement;
	GetWindowPlacement(&WindowPlacement);
	const auto Maximized {static_cast<const BOOL>(WindowPlacement.showCmd == SW_MAXIMIZE)};
	if (Maximized != 0) {
		m_Grip.ShowWindow(SW_HIDE);
	} else {
		m_Grip.ShowWindow(SW_SHOW);
	}
	if (Maximized == 0) {
		auto SizeBoxWindow {GetDlgItem(AFX_IDW_SIZE_BOX)};
		if (SizeBoxWindow != nullptr) {
			CRect WindowRect;
			SizeBoxWindow->GetWindowRect(&WindowRect);
			CRect ClientRect;
			GetClientRect(ClientRect);
			ClientRect.left = ClientRect.right - WindowRect.Width();
			ClientRect.top = ClientRect.bottom - WindowRect.Height();
			SizeBoxWindow->MoveWindow(&ClientRect);
		}
	}
}
