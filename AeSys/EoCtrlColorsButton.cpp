#include "stdafx.h"
#include "AeSysApp.h"

#include "EoCtrlColorsButton.h"

COLORREF* EoCtrlColorsButton::m_Palette;
EoUInt16 EoCtrlColorsButton::m_CurrentIndex;
EoUInt16 EoCtrlColorsButton::m_SelectedIndex;

IMPLEMENT_DYNAMIC(EoCtrlColorsButton, CMFCButton)

BEGIN_MESSAGE_MAP(EoCtrlColorsButton, CMFCButton)
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

EoCtrlColorsButton::EoCtrlColorsButton() {
	m_Layout = SimpleSingleRow;
	m_CellSize.cx = 8;
	m_CellSize.cy = 8;
	m_CellSpacing.cx = 1;
	m_CellSpacing.cy = 1;
	m_Margins.cx = 3;
	m_Margins.cy = 3;
	m_BeginIndex = 1;
	m_EndIndex = 1;

	m_SubItem = 0;
}
EoCtrlColorsButton::~EoCtrlColorsButton() {
}
void EoCtrlColorsButton::DrawCell(CDC* deviceContext, EoUInt16 index, COLORREF color) {
	if (index != 0) {
		CRect CellRectangle;
		SubItemRectangleByIndex(index, CellRectangle);

		if (index == m_CurrentIndex || index == m_SelectedIndex) {
			CBrush FrameBrush;
			if (index == m_CurrentIndex) {
				FrameBrush.CreateSysColorBrush(COLOR_HIGHLIGHT);
			}
			else {
				FrameBrush.CreateSysColorBrush(COLOR_WINDOWFRAME);
			}
			deviceContext->FrameRect(&CellRectangle, &FrameBrush);
			CellRectangle.DeflateRect(1, 1);
			CBrush InnerFrameBrush;
			InnerFrameBrush.CreateSysColorBrush(COLOR_BTNHIGHLIGHT);
			deviceContext->FrameRect(&CellRectangle, &InnerFrameBrush);
			CellRectangle.DeflateRect(1, 1);
		}
		CBrush Brush = CBrush(color);
		deviceContext->FillRect(&CellRectangle, &Brush);
	}
}
CSize EoCtrlColorsButton::SizeToContent(BOOL calculateOnly) {
	CRect BeginRectangle;
	CRect EndRectangle;
	CRect UnionRectangle;
	SubItemRectangleByIndex(m_BeginIndex, BeginRectangle);
	SubItemRectangleByIndex(m_EndIndex, EndRectangle);
	UnionRectangle.UnionRect(BeginRectangle, EndRectangle);
	CSize Size = UnionRectangle.Size();
	Size.cx += 2 * (m_CellSpacing.cx + m_Margins.cx);
	Size.cy += 2 * (m_CellSpacing.cy + m_Margins.cy);
	if (!calculateOnly) {
		CRect ClientRectangle;
		GetWindowRect(ClientRectangle);
		GetParent()->ScreenToClient(ClientRectangle);

		ClientRectangle.right  = ClientRectangle.left + Size.cx;
		ClientRectangle.bottom = ClientRectangle.top + Size.cy;

		MoveWindow(ClientRectangle);
	}
	return Size;
}
void EoCtrlColorsButton::SubItemRectangleByIndex(EoUInt16 index, CRect& rectangle) {
	rectangle.top = m_Margins.cx + m_CellSpacing.cy;
	rectangle.left = m_Margins.cy + m_CellSpacing.cx;

	switch (m_Layout) {
	case SimpleSingleRow:
		rectangle.left += (index - m_BeginIndex) * (m_CellSize.cx + m_CellSpacing.cx);
		break;
	case GridDown5RowsOddOnly:
		rectangle.top += (((index - m_BeginIndex) % 10) / 2) * (m_CellSize.cy + m_CellSpacing.cy);
		rectangle.left += ((index - m_BeginIndex) / 10) * (m_CellSize.cx + m_CellSpacing.cx);
		break;
	case GridUp5RowsEvenOnly:
		rectangle.top += (4 - ((index - m_BeginIndex) % 10) / 2) * (m_CellSize.cy + m_CellSpacing.cy);
		rectangle.left += ((index - m_BeginIndex) / 10) * (m_CellSize.cx + m_CellSpacing.cx);
	}
	rectangle.bottom = rectangle.top + m_CellSize.cy;
	rectangle.right = rectangle.left + m_CellSize.cx;
}
EoUInt16 EoCtrlColorsButton::SubItemByPoint(const CPoint& point) {
	CRect Rectangle;
	Rectangle.SetRectEmpty();

	switch (m_Layout) {
	case SimpleSingleRow:
		for (EoUInt16 Index = m_BeginIndex; Index <= m_EndIndex; Index++) {
			SubItemRectangleByIndex(Index, Rectangle);
			if (Rectangle.PtInRect(point) == TRUE) {
				return Index;
			}
		}
		break;
	case GridDown5RowsOddOnly:
		for (EoUInt16 Index = m_BeginIndex; Index <= m_EndIndex; Index++) {
			if ((Index % 2) != 0) {
				SubItemRectangleByIndex(Index, Rectangle);
				if (Rectangle.PtInRect(point) == TRUE) {
					return Index;
				}
			}
		}
		break;
	case GridUp5RowsEvenOnly:
		for (EoUInt16 Index = m_BeginIndex; Index <= m_EndIndex; Index++) {
			if ((Index % 2) == 0) {
				SubItemRectangleByIndex(Index, Rectangle);
				if (Rectangle.PtInRect(point) == TRUE) {
					return Index;
				}
			}
		}
	}
	return 0;
}
void EoCtrlColorsButton::OnDraw(CDC* deviceContext, const CRect& /*rectangle */, UINT /* state */) {
	m_SelectedIndex = 0;

	for (EoUInt16 Index = m_BeginIndex; Index <= m_EndIndex; Index++) {
		if (m_Layout == SimpleSingleRow) {
			DrawCell(deviceContext, Index, m_Palette[Index]);
		}
		else if (m_Layout == GridDown5RowsOddOnly && ((Index % 2) != 0)) {
			DrawCell(deviceContext, Index, m_Palette[Index]);
		}
		else if (m_Layout == GridUp5RowsEvenOnly && ((Index % 2) == 0)) {
			DrawCell(deviceContext, Index, m_Palette[Index]);
		}
	}
}
UINT EoCtrlColorsButton::OnGetDlgCode() {
	return DLGC_WANTARROWS;
}
void EoCtrlColorsButton::OnKeyDown(UINT keyCode, UINT repeatCount, UINT flags) {
	if (keyCode >= VK_LEFT && keyCode <= VK_DOWN) {
		CDC* DeviceContext = GetDC();
		m_SelectedIndex = 0;
		DrawCell(DeviceContext, m_SubItem, m_Palette[m_SubItem]);

		if (m_Layout == SimpleSingleRow) {
			switch (keyCode) {
			case VK_RIGHT:
				m_SubItem++;
				break;
			case VK_LEFT:
				m_SubItem--;
				break;
			}
		}
		else if (m_Layout == GridDown5RowsOddOnly) {
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
		}
		else if (m_Layout == GridUp5RowsEvenOnly) {
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
		DrawCell(DeviceContext, m_SubItem, m_Palette[m_SubItem]);
		ReleaseDC(DeviceContext);

		NMHDR NotifyStructure;
		NotifyStructure.hwndFrom = GetSafeHwnd();
		NotifyStructure.idFrom = 0;
		NotifyStructure.code = 0;

		::SendMessage(GetParent()->GetSafeHwnd(), WM_NOTIFY, 0, (LPARAM) &NotifyStructure);
	}
	CMFCButton::OnKeyDown(keyCode, repeatCount, flags);
}
void EoCtrlColorsButton::OnLButtonUp(UINT flags, CPoint point) {
	EoUInt16 CurrentSubItem = SubItemByPoint(point);
	if (CurrentSubItem != 0) {
		m_SubItem = CurrentSubItem;
	}
	CMFCButton::OnLButtonUp(flags, point);
}
void EoCtrlColorsButton::OnMouseMove(UINT flags, CPoint point) {
	CDC* DeviceContext = GetDC();
	m_SelectedIndex = 0;
	DrawCell(DeviceContext, m_SubItem, m_Palette[m_SubItem]);

	m_SubItem = SubItemByPoint(point);

	if (m_SubItem != 0) {
		m_SelectedIndex = m_SubItem;

		DrawCell(DeviceContext, m_SubItem, m_Palette[m_SubItem]);

		NMHDR NotifyStructure;
		NotifyStructure.hwndFrom = GetSafeHwnd();
		NotifyStructure.idFrom = 0;
		NotifyStructure.code = 0;

		::SendMessage(GetParent()->GetSafeHwnd(), WM_NOTIFY, 0, (LPARAM) &NotifyStructure);
	}
	ReleaseDC(DeviceContext);

	CMFCButton::OnMouseMove(flags, point);
}
void EoCtrlColorsButton::OnPaint() {
	CMFCButton::OnPaint();
}
void EoCtrlColorsButton::OnSetFocus(CWnd* oldWindow) {
	CMFCButton::OnSetFocus(oldWindow);

	CDC* DeviceContext = GetDC();
	DrawCell(DeviceContext, m_SubItem, m_Palette[m_SubItem]);

	m_SubItem = m_BeginIndex;
	CRect CurrentSubItemRectangle;
	SubItemRectangleByIndex(m_SubItem, CurrentSubItemRectangle);

	m_SelectedIndex = m_SubItem;
	DrawCell(DeviceContext, m_SubItem, m_Palette[m_SubItem]);
	ReleaseDC(DeviceContext);

	NMHDR NotifyStructure;
	NotifyStructure.hwndFrom = GetSafeHwnd();
	NotifyStructure.idFrom = 0;
	NotifyStructure.code = 0;

	::SendMessage(GetParent()->GetSafeHwnd(), WM_NOTIFY, 0, (LPARAM) &NotifyStructure);
}
