#pragma once

class EoCtrlBitmapPickerCombo : public CComboBox {
	int m_ItemWidth;
	int m_ItemHeight;

	void OutputBitmap(LPDRAWITEMSTRUCT drawItemStruct, bool selected);
	void SetSize(int width, int height);

public:
	EoCtrlBitmapPickerCombo();

	~EoCtrlBitmapPickerCombo() {}

	int AddBitmap(const CBitmap* bitmap, const wchar_t* string = NULL);
	int InsertBitmap(int nIndex, const CBitmap* bitmap, const wchar_t* string = NULL);

protected:
	void DrawItem(LPDRAWITEMSTRUCT drawItemStruct) override;

	void MeasureItem(LPMEASUREITEMSTRUCT lpMIS) noexcept override;
	int AddString(const wchar_t*) noexcept { return -1; } // hides non-virtual function of parent
	int InsertString(int, const wchar_t*) noexcept { return -1; } // hides non-virtual function of parent
};