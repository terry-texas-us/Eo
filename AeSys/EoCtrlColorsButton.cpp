#include "stdafx.h"
#include "AeSys.h"
#include "EoCtrlColorsButton.h"
gsl::span<COLORREF> EoCtrlColorsButton::m_Palette;
unsigned short EoCtrlColorsButton::m_CurrentIndex;
unsigned short EoCtrlColorsButton::m_SelectedIndex;
IMPLEMENT_DYNAMIC(EoCtrlColorsButton, CMFCButton)

BEGIN_MESSAGE_MAP(EoCtrlColorsButton, CMFCButton)
		ON_WM_GETDLGCODE()
		ON_WM_KEYDOWN()
		ON_WM_LBUTTONUP()
		ON_WM_MOUSEMOVE()
		ON_WM_PAINT()
		ON_WM_SETFOCUS()
END_MESSAGE_MAP()

void EoCtrlColorsButton::DrawCell(CDC* deviceContext, const unsigned short index, const COLORREF color) {
	if (deviceContext != nullptr && index != 0) {
		CRect CellRectangle;
		SubItemRectangleByIndex(index, CellRectangle);
		if (index == m_CurrentIndex || index == m_SelectedIndex) {
			CBrush FrameBrush;
			if (index == m_CurrentIndex) {
				FrameBrush.CreateSysColorBrush(COLOR_HIGHLIGHT);
			} else {
				FrameBrush.CreateSysColorBrush(COLOR_WINDOWFRAME);
			}
			deviceContext->FrameRect(&CellRectangle, &FrameBrush);
			CellRectangle.DeflateRect(1, 1);
			CBrush InnerFrameBrush;
			InnerFrameBrush.CreateSysColorBrush(COLOR_BTNHIGHLIGHT);
			deviceContext->FrameRect(&CellRectangle, &InnerFrameBrush);
			CellRectangle.DeflateRect(1, 1);
		}
		CBrush Brush(color);
		deviceContext->FillRect(&CellRectangle, &Brush);
	}
}

CSize EoCtrlColorsButton::SizeToContent(const BOOL calculateOnly) {
	CRect BeginRectangle;
	CRect EndRectangle;
	CRect UnionRectangle;
	SubItemRectangleByIndex(m_BeginIndex, BeginRectangle);
	SubItemRectangleByIndex(m_EndIndex, EndRectangle);
	UnionRectangle.UnionRect(BeginRectangle, EndRectangle);
	auto Size = UnionRectangle.Size();
	Size.cx += 2 * (m_CellSpacing.cx + m_Margins.cx);
	Size.cy += 2 * (m_CellSpacing.cy + m_Margins.cy);
	if (!calculateOnly) {
		CRect ClientRectangle;
		GetWindowRect(ClientRectangle);
		GetParent()->ScreenToClient(ClientRectangle);
		ClientRectangle.right = ClientRectangle.left + Size.cx;
		ClientRectangle.bottom = ClientRectangle.top + Size.cy;
		MoveWindow(ClientRectangle);
	}
	return Size;
}

void EoCtrlColorsButton::SubItemRectangleByIndex(const unsigned short index, CRect& rectangle) noexcept {
	rectangle.top = m_Margins.cx + m_CellSpacing.cy;
	rectangle.left = m_Margins.cy + m_CellSpacing.cx;
	switch (m_Layout) {
		case SimpleSingleRow:
			rectangle.left += (index - m_BeginIndex) * (m_CellSize.cx + m_CellSpacing.cx);
			break;
		case GridDown5RowsOddOnly:
			rectangle.top += (index - m_BeginIndex) % 10 / 2 * (m_CellSize.cy + m_CellSpacing.cy);
			rectangle.left += (index - m_BeginIndex) / 10 * (m_CellSize.cx + m_CellSpacing.cx);
			break;
		case GridUp5RowsEvenOnly:
			rectangle.top += (4 - (index - m_BeginIndex) % 10 / 2) * (m_CellSize.cy + m_CellSpacing.cy);
			rectangle.left += (index - m_BeginIndex) / 10 * (m_CellSize.cx + m_CellSpacing.cx);
	}
	rectangle.bottom = rectangle.top + m_CellSize.cy;
	rectangle.right = rectangle.left + m_CellSize.cx;
}

unsigned short EoCtrlColorsButton::SubItemByPoint(const CPoint& point) noexcept {
	CRect Rectangle;
	Rectangle.SetRectEmpty();
	switch (m_Layout) {
		case SimpleSingleRow:
			for (auto Index = m_BeginIndex; Index <= m_EndIndex; Index++) {
				SubItemRectangleByIndex(Index, Rectangle);
				if (Rectangle.PtInRect(point) == TRUE) { return Index; }
			}
			break;
		case GridDown5RowsOddOnly:
			for (auto Index = m_BeginIndex; Index <= m_EndIndex; Index++) {
				if (Index % 2 != 0) {
					SubItemRectangleByIndex(Index, Rectangle);
					if (Rectangle.PtInRect(point) == TRUE) { return Index; }
				}
			}
			break;
		case GridUp5RowsEvenOnly:
			for (auto Index = m_BeginIndex; Index <= m_EndIndex; Index++) {
				if (Index % 2 == 0) {
					SubItemRectangleByIndex(Index, Rectangle);
					if (Rectangle.PtInRect(point) == TRUE) { return Index; }
				}
			}
	}
	return 0;
}

void EoCtrlColorsButton::OnDraw(CDC* deviceContext, const CRect& /*rectangle*/, unsigned /*state*/) {
	m_SelectedIndex = 0;
	for (auto Index = m_BeginIndex; Index <= m_EndIndex; Index++) {
		if (m_Layout == SimpleSingleRow) {
			DrawCell(deviceContext, Index, m_Palette.at(Index));
		} else if (m_Layout == GridDown5RowsOddOnly && Index % 2 != 0) {
			DrawCell(deviceContext, Index, m_Palette.at(Index));
		} else if (m_Layout == GridUp5RowsEvenOnly && Index % 2 == 0) {
			DrawCell(deviceContext, Index, m_Palette.at(Index));
		}
	}
}

unsigned EoCtrlColorsButton::OnGetDlgCode() noexcept {
	return DLGC_WANTARROWS;
}

void EoCtrlColorsButton::OnKeyDown(const unsigned keyCode, const unsigned repeatCount, const unsigned flags) {
	if (keyCode >= VK_LEFT && keyCode <= VK_DOWN) {
		const auto DeviceContext {GetDC()};
		m_SelectedIndex = 0;
		DrawCell(DeviceContext, m_SubItem, m_Palette.at(m_SubItem));
		if (m_Layout == SimpleSingleRow) {
			switch (keyCode) {
				case VK_RIGHT:
					m_SubItem++;
					break;
				case VK_LEFT:
					m_SubItem--;
					break;
			}
		} else if (m_Layout == GridDown5RowsOddOnly) {
			switch (keyCode) {
				case VK_DOWN:
					m_SubItem += 2;
					break;
				case VK_RIGHT:
					m_SubItem += 10;
					break;
				case VK_LEFT:
					m_SubItem -= 10;
					break;
				case VK_UP:
					m_SubItem -= 2;
					break;
			}
		} else if (m_Layout == GridUp5RowsEvenOnly) {
			switch (keyCode) {
				case VK_DOWN:
					m_SubItem -= 2;
					break;
				case VK_RIGHT:
					m_SubItem += 10;
					break;
				case VK_LEFT:
					m_SubItem -= 10;
					break;
				case VK_UP:
					m_SubItem += 2;
					break;
			}
		}
		m_SubItem = EoMax(m_BeginIndex, EoMin(m_EndIndex, m_SubItem));
		CRect CurrentSubItemRectangle;
		SubItemRectangleByIndex(m_SubItem, CurrentSubItemRectangle);
		m_SelectedIndex = m_SubItem;
		DrawCell(DeviceContext, m_SubItem, m_Palette.at(m_SubItem));
		ReleaseDC(DeviceContext);
		NMHDR NotifyStructure;
		NotifyStructure.hwndFrom = GetSafeHwnd();
		NotifyStructure.idFrom = 0;
		NotifyStructure.code = 0;
		::SendMessageW(GetParent()->GetSafeHwnd(), WM_NOTIFY, 0, LPARAM(&NotifyStructure));
	}
	CMFCButton::OnKeyDown(keyCode, repeatCount, flags);
}

void EoCtrlColorsButton::OnLButtonUp(const unsigned flags, const CPoint point) {
	const auto CurrentSubItem {SubItemByPoint(point)};
	if (CurrentSubItem != 0) { m_SubItem = CurrentSubItem; }
	CMFCButton::OnLButtonUp(flags, point);
}

void EoCtrlColorsButton::OnMouseMove(const unsigned flags, const CPoint point) {
	const auto DeviceContext {GetDC()};
	m_SelectedIndex = 0;
	DrawCell(DeviceContext, m_SubItem, m_Palette.at(m_SubItem));
	m_SubItem = SubItemByPoint(point);
	if (m_SubItem != 0) {
		m_SelectedIndex = m_SubItem;
		DrawCell(DeviceContext, m_SubItem, m_Palette.at(m_SubItem));
		NMHDR NotifyStructure;
		NotifyStructure.hwndFrom = GetSafeHwnd();
		NotifyStructure.idFrom = 0;
		NotifyStructure.code = 0;
		::SendMessageW(GetParent()->GetSafeHwnd(), WM_NOTIFY, 0, LPARAM(&NotifyStructure));
	}
	ReleaseDC(DeviceContext);
	CMFCButton::OnMouseMove(flags, point);
}

void EoCtrlColorsButton::OnPaint() {
	CMFCButton::OnPaint();
}

void EoCtrlColorsButton::OnSetFocus(CWnd* oldWindow) {
	CMFCButton::OnSetFocus(oldWindow);
	const auto DeviceContext {GetDC()};
	DrawCell(DeviceContext, m_SubItem, m_Palette.at(m_SubItem));
	m_SubItem = m_BeginIndex;
	CRect CurrentSubItemRectangle;
	SubItemRectangleByIndex(m_SubItem, CurrentSubItemRectangle);
	m_SelectedIndex = m_SubItem;
	DrawCell(DeviceContext, m_SubItem, m_Palette.at(m_SubItem));
	ReleaseDC(DeviceContext);
	NMHDR NotifyStructure;
	NotifyStructure.hwndFrom = GetSafeHwnd();
	NotifyStructure.idFrom = 0;
	NotifyStructure.code = 0;
	::SendMessageW(GetParent()->GetSafeHwnd(), WM_NOTIFY, 0, LPARAM(&NotifyStructure));
}
