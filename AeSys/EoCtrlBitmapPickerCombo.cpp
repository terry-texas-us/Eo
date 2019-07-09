#include "stdafx.h"
#include "EoCtrlBitmapPickerCombo.h"

static void DrawBitmap(const CBitmap* bitmap, const CDC* deviceContext, const CPoint& point) {
	BITMAP Bitmap;
	const_cast<CBitmap*>(bitmap)->GetBitmap(&Bitmap);
	const int Width {Bitmap.bmWidth};
	const int Height {Bitmap.bmHeight};
	CDC MemoryDeviceContext;
	MemoryDeviceContext.CreateCompatibleDC(const_cast<CDC*>(deviceContext));
	const auto pBmp {MemoryDeviceContext.SelectObject(const_cast<CBitmap*>(bitmap))};
	const_cast<CDC*>(deviceContext)->BitBlt(point.x, point.y, Width, Height, &MemoryDeviceContext, 0, 0, SRCCOPY);
	MemoryDeviceContext.SelectObject(pBmp);
}

static void DrawBitmap(const CBitmap* bitmap, const CDC* deviceContext, const CRect& rect) {
	BITMAP Bitmap;
	const_cast<CBitmap*>(bitmap)->GetBitmap(&Bitmap);
	const int Width = Bitmap.bmWidth;
	const int Height = Bitmap.bmHeight;
	CPoint Point;
	Point.x = rect.left + (rect.right - rect.left) / 2 - Width / 2;
	Point.y = rect.top + (rect.bottom - rect.top) / 2 - Height / 2;
	DrawBitmap(bitmap, deviceContext, Point);
}

EoCtrlBitmapPickerCombo::EoCtrlBitmapPickerCombo()
	: CComboBox() {
}

int EoCtrlBitmapPickerCombo::AddBitmap(const CBitmap* bitmap, const wchar_t* string) {
	return InsertBitmap(GetCount(), bitmap, string);
}

int EoCtrlBitmapPickerCombo::InsertBitmap(const int index, const CBitmap* bitmap, const wchar_t* string) {
	const auto n {CComboBox::InsertString(index, string != nullptr ? string : L"")};
	if (bitmap == nullptr) { return n; }
	if (n != CB_ERR && n != CB_ERRSPACE) {
		SetItemData(n, reinterpret_cast<unsigned long>(bitmap));
		BITMAP Bitmap;
		const_cast<CBitmap*>(bitmap)->GetBitmap(&Bitmap);
		SetSize(Bitmap.bmWidth, Bitmap.bmHeight);
	}
	return n;
}

void EoCtrlBitmapPickerCombo::MeasureItem(const LPMEASUREITEMSTRUCT lpMIS) noexcept {
	lpMIS->itemWidth = static_cast<unsigned>(m_ItemWidth + 2);
	lpMIS->itemHeight = static_cast<unsigned>(m_ItemHeight + 2);
}

void EoCtrlBitmapPickerCombo::DrawItem(LPDRAWITEMSTRUCT drawItemStruct) {
	auto DeviceContext {CDC::FromHandle(drawItemStruct->hDC)};
	if (IsWindowEnabled() == 0) {
		CBrush DisabledBrush(RGB(192, 192, 192)); // light gray
		CPen DisabledPen(PS_SOLID, 1, RGB(192, 192, 192));
		const auto OldBrush {DeviceContext->SelectObject(&DisabledBrush)};
		const auto OldPen {DeviceContext->SelectObject(&DisabledPen)};
		OutputBitmap(drawItemStruct, false);
		DeviceContext->SelectObject(OldBrush);
		DeviceContext->SelectObject(OldPen);
		return;
	}
	if (drawItemStruct->itemState & ODS_SELECTED && drawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)) {
		CBrush HighlightBrush(GetSysColor(COLOR_HIGHLIGHT));
		CPen HighlightPen(PS_SOLID, 1, GetSysColor(COLOR_HIGHLIGHT));
		const auto OldBrush {DeviceContext->SelectObject(&HighlightBrush)};
		const auto OldPen {DeviceContext->SelectObject(&HighlightPen)};
		DeviceContext->Rectangle(&drawItemStruct->rcItem);
		DeviceContext->SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
		DeviceContext->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		OutputBitmap(drawItemStruct, true);
		DeviceContext->SelectObject(OldBrush);
		DeviceContext->SelectObject(OldPen);
	}
	if ((drawItemStruct->itemState & ODS_SELECTED) == 0U && drawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)) {
		CBrush WindowBrush(GetSysColor(COLOR_WINDOW));
		CPen WindowPen(PS_SOLID, 1, GetSysColor(COLOR_WINDOW));
		const auto OldBrush {DeviceContext->SelectObject(&WindowBrush)};
		const auto OldPen {DeviceContext->SelectObject(&WindowPen)};
		DeviceContext->Rectangle(&drawItemStruct->rcItem);
		DeviceContext->SetBkColor(GetSysColor(COLOR_WINDOW));
		DeviceContext->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		OutputBitmap(drawItemStruct, false);
		DeviceContext->SelectObject(OldBrush);
		DeviceContext->SelectObject(OldPen);
	}
	if (drawItemStruct->itemAction & ODA_FOCUS) { DeviceContext->DrawFocusRect(&drawItemStruct->rcItem); }
}

void EoCtrlBitmapPickerCombo::OutputBitmap(const LPDRAWITEMSTRUCT drawItemStruct, bool /*selected*/) {
	const auto Bitmap {reinterpret_cast<const CBitmap*>(drawItemStruct->itemData)};
	if (Bitmap != nullptr && Bitmap != reinterpret_cast<const CBitmap*>(0xffffffff)) {
		auto DeviceContext {CDC::FromHandle(drawItemStruct->hDC)};
		CString String;
		if (drawItemStruct->itemID != static_cast<unsigned>(-1)) { GetLBText(static_cast<int>(drawItemStruct->itemID), String); }
		if (String.IsEmpty()) {
			DrawBitmap(Bitmap, DeviceContext, drawItemStruct->rcItem);
		} else {
			CPoint Point;
			Point.x = drawItemStruct->rcItem.left + 2;
			Point.y = drawItemStruct->rcItem.top + (drawItemStruct->rcItem.bottom - drawItemStruct->rcItem.top) / 2 - m_ItemHeight / 2;
			DrawBitmap(Bitmap, DeviceContext, Point);
			CRect rcText(drawItemStruct->rcItem);
			rcText.DeflateRect(m_ItemWidth + 4, 0, 0, 0);
			DeviceContext->DrawTextW(String, rcText, DT_SINGLELINE | DT_VCENTER);
		}
	}
	if (Bitmap == nullptr) {
		auto DeviceContext {CDC::FromHandle(drawItemStruct->hDC)};
		CString String;
		if (drawItemStruct->itemID != static_cast<unsigned>(-1)) { GetLBText(static_cast<int>(drawItemStruct->itemID), String); }
		CRect TextRectangle(drawItemStruct->rcItem);
		DeviceContext->DrawTextW(String, TextRectangle, DT_SINGLELINE | DT_VCENTER);
	}
}

void EoCtrlBitmapPickerCombo::SetSize(const int width, const int height) {
	if (width > m_ItemWidth) { m_ItemWidth = width; }
	if (height > m_ItemHeight) { m_ItemHeight = height; }
	for (auto i = -1; i < GetCount(); i++) {
		SetItemHeight(i, static_cast<unsigned>(m_ItemHeight + 6));
	}
}
