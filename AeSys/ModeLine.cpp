#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysView.h"

void AeSysView::ModeLineDisplay() {
	if (theApp.CurrentMode() == 0) {
		return;
	}
	m_OpHighlighted = 0;

	CString ModeInformation = theApp.LoadStringResource(UINT(theApp.CurrentMode()));

	CString ModeOp;

	CDC* DeviceContext = GetDC();

	for (int i = 0; i < 10; i++) {
		AfxExtractSubString(ModeOp, ModeInformation, i + 1, '\n');

		// Note: Using active view device context for sizing status bar panes
		const CSize size = DeviceContext->GetTextExtent(ModeOp);

		GetStatusBar().SetPaneInfo(::nStatusOp0 + i, ID_OP0 + i, SBPS_NORMAL, size.cx);
		GetStatusBar().SetPaneText(::nStatusOp0 + i, ModeOp);
		GetStatusBar().SetTipText(::nStatusOp0 + i, L"Mode Command Tip Text");
	}
	if (theApp.ModeInformationOverView()) {
		CFont* Font = (CFont*) DeviceContext->SelectStockObject(ANSI_VAR_FONT);
		const UINT nTextAlign = DeviceContext->SetTextAlign(TA_LEFT | TA_TOP);
		const COLORREF crText = DeviceContext->SetTextColor(AppGetTextCol());
		const COLORREF crBk = DeviceContext->SetBkColor(~AppGetTextCol() & 0x00ffffff);

		TEXTMETRIC tm;
		DeviceContext->GetTextMetrics(&tm);

		CRect rcClient;
		GetClientRect(&rcClient);

		const int iMaxChrs = (rcClient.Width() / 10) / tm.tmAveCharWidth;
		const int Width = iMaxChrs * tm.tmAveCharWidth;

		for (int i = 0; i < 10; i++) {
			ModeOp = GetStatusBar().GetPaneText(::nStatusOp0 + i);

			const CRect rc(i * Width, rcClient.bottom - tm.tmHeight, (i + 1) * Width, rcClient.bottom);

			DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, ModeOp, ModeOp.GetLength(), 0);
		}
		DeviceContext->SetBkColor(crBk);
		DeviceContext->SetTextColor(crText);
		DeviceContext->SetTextAlign(nTextAlign);
		DeviceContext->SelectObject(Font);
	}
	ReleaseDC(DeviceContext);
}
OdUInt16 AeSysView::ModeLineHighlightOp(OdUInt16 command) {
	ModeLineUnhighlightOp(m_OpHighlighted);

	m_OpHighlighted = command;

	if (command == 0) {
		return 0;
	}
	const int PaneIndex = ::nStatusOp0 + m_OpHighlighted - ID_OP0;

	GetStatusBar().SetPaneTextColor(PaneIndex, RGB(255, 0, 0));

	if (theApp.ModeInformationOverView()) {
		CString ModeOp = GetStatusBar().GetPaneText(PaneIndex);

		CDC* DeviceContext = GetDC();

		CFont* Font = (CFont*) DeviceContext->SelectStockObject(ANSI_VAR_FONT);
		const UINT TextAlign = DeviceContext->SetTextAlign(TA_LEFT | TA_TOP);
		const COLORREF crText = DeviceContext->SetTextColor(RGB(255, 0, 0));
		const COLORREF crBk = DeviceContext->SetBkColor(~AppGetTextCol() & 0x00ffffff);

		TEXTMETRIC tm;
		DeviceContext->GetTextMetrics(&tm);

		CRect rcClient;
		GetClientRect(&rcClient);

		const int iMaxChrs = (rcClient.Width() / 10) / tm.tmAveCharWidth;
		const int Width = iMaxChrs * tm.tmAveCharWidth;
		const int i = m_OpHighlighted - ID_OP0;

		const CRect rc(i * Width, rcClient.bottom - tm.tmHeight, (i + 1) * Width, rcClient.bottom);

		DeviceContext->ExtTextOutW(rc.left , rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, ModeOp, (UINT) ModeOp.GetLength(), 0);

		DeviceContext->SetBkColor(crBk);
		DeviceContext->SetTextColor(crText);
		DeviceContext->SetTextAlign(TextAlign);
		DeviceContext->SelectObject(Font);
		ReleaseDC(DeviceContext);
	}
	return (command);
}
void AeSysView::ModeLineUnhighlightOp(OdUInt16& command) {
	if (command == 0 || m_OpHighlighted == 0) {
		return;
	}
	const int PaneIndex = ::nStatusOp0 + m_OpHighlighted - ID_OP0;

	GetStatusBar().SetPaneTextColor(PaneIndex);

	if (theApp.ModeInformationOverView()) {
		CString ModeOp = GetStatusBar().GetPaneText(PaneIndex);

		CDC* DeviceContext = GetDC();

		CFont* Font = (CFont*) DeviceContext->SelectStockObject(ANSI_VAR_FONT);
		const UINT TextAlign = DeviceContext->SetTextAlign(TA_LEFT | TA_TOP);
		const COLORREF crText = DeviceContext->SetTextColor(AppGetTextCol());
		const COLORREF crBk = DeviceContext->SetBkColor(~AppGetTextCol() & 0x00ffffff);

		TEXTMETRIC tm;
		DeviceContext->GetTextMetrics(&tm);

		CRect rcClient;
		GetClientRect(&rcClient);

		const int iMaxChrs = (rcClient.Width() / 10) / tm.tmAveCharWidth;
		const int Width = iMaxChrs * tm.tmAveCharWidth;
		const int i = m_OpHighlighted - ID_OP0;

		const CRect rc(i * Width, rcClient.bottom - tm.tmHeight, (i + 1) * Width, rcClient.bottom);

		DeviceContext->ExtTextOutW(rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE, &rc, ModeOp, (UINT) ModeOp.GetLength(), 0);

		DeviceContext->SetBkColor(crBk);
		DeviceContext->SetTextColor(crText);
		DeviceContext->SetTextAlign(TextAlign);
		DeviceContext->SelectObject(Font);
		ReleaseDC(DeviceContext);
	}
	command = 0;
	m_OpHighlighted = 0;
}
