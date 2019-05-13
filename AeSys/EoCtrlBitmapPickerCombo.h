#pragma once

class EoCtrlBitmapPickerCombo : public CComboBox {
	int m_ItemWidth;
	int m_ItemHeight;

	void OutputBitmap(LPDRAWITEMSTRUCT drawItemStruct, bool selected);
	void SetSize(int width, int height);

public:
	EoCtrlBitmapPickerCombo();

	virtual ~EoCtrlBitmapPickerCombo() {}

	int AddBitmap(const CBitmap* bitmap, LPCWSTR string = NULL);
	int InsertBitmap(int nIndex, const CBitmap* bitmap, LPCWSTR string = NULL);

protected:
	void DrawItem(LPDRAWITEMSTRUCT drawItemStruct) override;

	void MeasureItem(LPMEASUREITEMSTRUCT lpMIS) noexcept override;
	virtual int AddString(LPCWSTR) noexcept { return -1; }
	virtual int InsertString(int, LPCWSTR) noexcept { return -1; }
};