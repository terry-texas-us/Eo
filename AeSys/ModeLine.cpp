#include "stdafx.h"
#include "AeSys.h"
#include "AeSysView.h"

void AeSysView::ModeLineDisplay() {
	if (theApp.CurrentMode() == 0) {
		return;
	}
	m_OpHighlighted = 0;
	const auto ModeInformation {AeSys::LoadStringResource(theApp.CurrentMode())};
	CString ModeOp;
	const gsl::not_null<CDC*> (DeviceContext) {GetDC()};
	for (auto ModeIndex = 0; ModeIndex < 10; ModeIndex++) {
		AfxExtractSubString(ModeOp, ModeInformation, ModeIndex + 1, '\n');

		// <tas="Using active view device context for sizing status bar panes."/>
		const auto Size {DeviceContext->GetTextExtent(ModeOp)};
		GetStatusBar().SetPaneInfo(gc_StatusOp0 + ModeIndex, static_cast<unsigned>(ID_OP0 + ModeIndex), SBPS_NORMAL, Size.cx);
		GetStatusBar().SetPaneText(gc_StatusOp0 + ModeIndex, ModeOp);
		GetStatusBar().SetTipText(gc_StatusOp0 + ModeIndex, L"Mode Command Tip Text");
	}
	if (theApp.ModeInformationOverView()) {
		const auto Font {dynamic_cast<CFont*>(DeviceContext->SelectStockObject(SYSTEM_FONT))};
		const auto TextAlign {DeviceContext->SetTextAlign(TA_LEFT | TA_TOP)};
		const auto TextColor {DeviceContext->SetTextColor(AppGetTextCol())};
		const auto BackgroundColor {DeviceContext->SetBkColor(~AppGetTextCol() & 0x00ffffff)};
		TEXTMETRIC TextMetrics;
		DeviceContext->GetTextMetricsW(&TextMetrics);
		CRect ClientRectangle;
		GetClientRect(&ClientRectangle);
		const int Width = ClientRectangle.Width() / 10 / TextMetrics.tmAveCharWidth * TextMetrics.tmAveCharWidth;
		for (auto ModeIndex = 0; ModeIndex < 10; ModeIndex++) {
			ModeOp = GetStatusBar().GetPaneText(gc_StatusOp0 + ModeIndex);
			const CRect Rectangle(ModeIndex * Width, ClientRectangle.bottom - TextMetrics.tmHeight, (ModeIndex + 1) * Width, ClientRectangle.bottom);
			DeviceContext->ExtTextOutW(Rectangle.left, Rectangle.top, ETO_CLIPPED | ETO_OPAQUE, &Rectangle, ModeOp, static_cast<unsigned>(ModeOp.GetLength()), nullptr);
		}
		DeviceContext->SetBkColor(BackgroundColor);
		DeviceContext->SetTextColor(TextColor);
		DeviceContext->SetTextAlign(TextAlign);
		DeviceContext->SelectObject(Font);
	}
	ReleaseDC(DeviceContext);
}

unsigned short AeSysView::ModeLineHighlightOp(const unsigned short command) {
	ModeLineUnhighlightOp(m_OpHighlighted);
	m_OpHighlighted = command;
	if (command == 0) {
		return 0;
	}
	const auto PaneIndex {gc_StatusOp0 + m_OpHighlighted - ID_OP0};
	GetStatusBar().SetPaneTextColor(PaneIndex, RGB(255, 0, 0));
	if (theApp.ModeInformationOverView()) {
		const auto ModeOp {GetStatusBar().GetPaneText(PaneIndex)};
		const gsl::not_null<CDC*> (DeviceContext) {GetDC()};
		const auto Font {dynamic_cast<CFont*>(DeviceContext->SelectStockObject(SYSTEM_FONT))};
		const auto TextAlign {DeviceContext->SetTextAlign(TA_LEFT | TA_TOP)};
		const auto TextColor {DeviceContext->SetTextColor(RGB(255, 0, 0))};
		const auto BackgroundColor {DeviceContext->SetBkColor(~AppGetTextCol() & 0x00ffffff)};
		TEXTMETRIC TextMetrics;
		DeviceContext->GetTextMetricsW(&TextMetrics);
		CRect ClientRectangle;
		GetClientRect(&ClientRectangle);
		const int Width = ClientRectangle.Width() / 10 / TextMetrics.tmAveCharWidth * TextMetrics.tmAveCharWidth;
		const auto i {m_OpHighlighted - ID_OP0};
		const CRect Rectangle {i * Width, ClientRectangle.bottom - TextMetrics.tmHeight, (i + 1) * Width, ClientRectangle.bottom};
		DeviceContext->ExtTextOutW(Rectangle.left, Rectangle.top, ETO_CLIPPED | ETO_OPAQUE, &Rectangle, ModeOp, static_cast<unsigned>(ModeOp.GetLength()), nullptr);
		DeviceContext->SetBkColor(BackgroundColor);
		DeviceContext->SetTextColor(TextColor);
		DeviceContext->SetTextAlign(TextAlign);
		DeviceContext->SelectObject(Font);
		ReleaseDC(DeviceContext);
	}
	return command;
}

void AeSysView::ModeLineUnhighlightOp(unsigned short& command) {
	if (command == 0 || m_OpHighlighted == 0) {
		return;
	}
	const auto PaneIndex {gc_StatusOp0 + m_OpHighlighted - ID_OP0};
	GetStatusBar().SetPaneTextColor(PaneIndex);
	if (theApp.ModeInformationOverView()) {
		const auto ModeOp {GetStatusBar().GetPaneText(PaneIndex)};
		const gsl::not_null<CDC*> (DeviceContext) {GetDC()};
		const auto Font {dynamic_cast<CFont*>(DeviceContext->SelectStockObject(SYSTEM_FONT))};
		const auto TextAlign {DeviceContext->SetTextAlign(TA_LEFT | TA_TOP)};
		const auto TextColor {DeviceContext->SetTextColor(AppGetTextCol())};
		const auto BackgroundColor {DeviceContext->SetBkColor(~AppGetTextCol() & 0x00ffffff)};
		TEXTMETRIC TextMetrics;
		DeviceContext->GetTextMetricsW(&TextMetrics);
		CRect ClientRectangle;
		GetClientRect(&ClientRectangle);
		const int Width = ClientRectangle.Width() / 10 / TextMetrics.tmAveCharWidth * TextMetrics.tmAveCharWidth;
		const auto i {m_OpHighlighted - ID_OP0};
		const CRect Rectangle {i * Width, ClientRectangle.bottom - TextMetrics.tmHeight, (i + 1) * Width, ClientRectangle.bottom};
		DeviceContext->ExtTextOutW(Rectangle.left, Rectangle.top, ETO_CLIPPED | ETO_OPAQUE, &Rectangle, ModeOp, static_cast<unsigned>(ModeOp.GetLength()), nullptr);
		DeviceContext->SetBkColor(BackgroundColor);
		DeviceContext->SetTextColor(TextColor);
		DeviceContext->SetTextAlign(TextAlign);
		DeviceContext->SelectObject(Font);
		ReleaseDC(DeviceContext);
	}
	command = 0;
	m_OpHighlighted = 0;
}
