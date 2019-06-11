#include "stdafx.h"

#include "EoCtrlBitmapPickerCombo.h"

static void DrawBitmap(const CBitmap* bitmap, const CDC* deviceContext, const CPoint& point) {
	BITMAP Bitmap;
	((CBitmap*) bitmap)->GetBitmap(&Bitmap);
	const int Width = Bitmap.bmWidth;
	const int Height = Bitmap.bmHeight;
	CDC MemoryDeviceContext;
	MemoryDeviceContext.CreateCompatibleDC(( CDC*) deviceContext);
	CBitmap* pBmp = MemoryDeviceContext.SelectObject(( CBitmap*) bitmap);
	((CDC*) deviceContext)->BitBlt(point.x, point.y, Width, Height, &MemoryDeviceContext, 0, 0, SRCCOPY);
	MemoryDeviceContext.SelectObject(pBmp);
}

static void DrawBitmap(const CBitmap* bitmap, const CDC* deviceContext, const CRect& rect) {
	BITMAP Bitmap;
	((CBitmap*) bitmap)->GetBitmap(&Bitmap);
	const int Width = Bitmap.bmWidth;
	const int Height = Bitmap.bmHeight;
	CPoint Point;
	Point.x = rect.left + ((rect.right - rect.left) / 2) - (Width / 2);
	Point.y = rect.top + ((rect.bottom - rect.top) / 2) - (Height / 2);
	DrawBitmap(bitmap, deviceContext, Point);
}

EoCtrlBitmapPickerCombo::EoCtrlBitmapPickerCombo() :
	CComboBox(), m_ItemWidth(0), m_ItemHeight(0) {
}

int EoCtrlBitmapPickerCombo::AddBitmap(const CBitmap* bitmap, const wchar_t* string) {
	return InsertBitmap(GetCount(), bitmap, string);
}

int EoCtrlBitmapPickerCombo::InsertBitmap(int nIndex, const CBitmap* bitmap, const wchar_t* string) {
	const int n = CComboBox::InsertString(nIndex, string ? string : L"");

	if (!bitmap) { return n; }

	if (n != CB_ERR && n != CB_ERRSPACE) {
		SetItemData(n, (unsigned long) bitmap);
		BITMAP Bitmap;
		((CBitmap*) bitmap)->GetBitmap(&Bitmap);
		SetSize(Bitmap.bmWidth, Bitmap.bmHeight);
	}
	return n;
}

void EoCtrlBitmapPickerCombo::MeasureItem(LPMEASUREITEMSTRUCT lpMIS) noexcept {
	lpMIS->itemWidth = static_cast<unsigned>(m_ItemWidth + 2);
	lpMIS->itemHeight = static_cast<unsigned>(m_ItemHeight + 2);
}

void EoCtrlBitmapPickerCombo::DrawItem(LPDRAWITEMSTRUCT drawItemStruct) {
	auto DeviceContext {CDC::FromHandle(drawItemStruct->hDC)};

	if (!IsWindowEnabled()) {
		CBrush DisabledBrush(RGB(192, 192, 192)); // light gray
		CPen DisabledPen(PS_SOLID, 1, RGB(192, 192, 192));
		auto OldBrush {DeviceContext->SelectObject(&DisabledBrush)};
		auto OldPen {DeviceContext->SelectObject(&DisabledPen)};
		OutputBitmap(drawItemStruct, false);
		DeviceContext->SelectObject(OldBrush);
		DeviceContext->SelectObject(OldPen);
		return;
	}
	if ((drawItemStruct->itemState & ODS_SELECTED) && (drawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))) {
		CBrush HighlightBrush(::GetSysColor(COLOR_HIGHLIGHT));
		CPen HighlightPen(PS_SOLID, 1, ::GetSysColor(COLOR_HIGHLIGHT));
		auto OldBrush {DeviceContext->SelectObject(&HighlightBrush)};
		auto OldPen {DeviceContext->SelectObject(&HighlightPen)};
		DeviceContext->Rectangle(&drawItemStruct->rcItem);
		DeviceContext->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		DeviceContext->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		OutputBitmap(drawItemStruct, true);
		DeviceContext->SelectObject(OldBrush);
		DeviceContext->SelectObject(OldPen);
	}
	if (!(drawItemStruct->itemState & ODS_SELECTED) && (drawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))) {
		CBrush WindowBrush(::GetSysColor(COLOR_WINDOW));
		CPen WindowPen(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOW));
		auto OldBrush {DeviceContext->SelectObject(&WindowBrush)};
		auto OldPen {DeviceContext->SelectObject(&WindowPen)};
		DeviceContext->Rectangle(&drawItemStruct->rcItem);
		DeviceContext->SetBkColor(::GetSysColor(COLOR_WINDOW));
		DeviceContext->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		OutputBitmap(drawItemStruct, false);
		DeviceContext->SelectObject(OldBrush);
		DeviceContext->SelectObject(OldPen);
	}
	if (drawItemStruct->itemAction & ODA_FOCUS) { DeviceContext->DrawFocusRect(&drawItemStruct->rcItem); }
}

void EoCtrlBitmapPickerCombo::OutputBitmap(LPDRAWITEMSTRUCT drawItemStruct, bool selected) {
	const auto bitmap {(const CBitmap*) (drawItemStruct->itemData)};

	if (bitmap && bitmap != (const CBitmap*) (0xffffffff)) {
		auto DeviceContext {CDC::FromHandle(drawItemStruct->hDC)};
		CString string;

		if (drawItemStruct->itemID != -1) { GetLBText(static_cast<int>(drawItemStruct->itemID), string); }

		if (string.IsEmpty()) {
			DrawBitmap(bitmap, DeviceContext, drawItemStruct->rcItem);
		} else {
			CPoint point;
			point.x = drawItemStruct->rcItem.left + 2;
			point.y = drawItemStruct->rcItem.top + ((drawItemStruct->rcItem.bottom - drawItemStruct->rcItem.top) / 2) - (m_ItemHeight / 2);
			DrawBitmap(bitmap, DeviceContext, point);
			CRect rcText(drawItemStruct->rcItem);
			rcText.DeflateRect(m_ItemWidth + 4, 0, 0, 0);
			DeviceContext->DrawText(string, rcText, DT_SINGLELINE | DT_VCENTER);
		}
	}
	if (!bitmap) {
		auto DeviceContext {CDC::FromHandle(drawItemStruct->hDC)};
		CString string;

		if (drawItemStruct->itemID != -1) { GetLBText(static_cast<int>(drawItemStruct->itemID), string); }

		CPoint point;
		point.x = drawItemStruct->rcItem.left + 2;
		point.y = drawItemStruct->rcItem.top + ((drawItemStruct->rcItem.bottom - drawItemStruct->rcItem.top) / 2) - (m_ItemHeight / 2);
		CRect rcText(drawItemStruct->rcItem);
		DeviceContext->DrawText(string, rcText, DT_SINGLELINE | DT_VCENTER);
	}
}

void EoCtrlBitmapPickerCombo::SetSize(int width, int height) {
	if (width > m_ItemWidth) { m_ItemWidth = width; }

	if (height > m_ItemHeight) { m_ItemHeight = height; }

	for (int i = -1; i < GetCount(); i++) {
		SetItemHeight(i, static_cast<unsigned>(m_ItemHeight + 6));
	}
}
