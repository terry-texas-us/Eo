#pragma once
class EoCtrlBitmapPickerCombo final : public CComboBox {
	int m_ItemWidth {0};
	int m_ItemHeight {0};

	void OutputBitmap(LPDRAWITEMSTRUCT drawItemStruct, bool selected) const;

	void SetSize(int width, int height);

public:
	EoCtrlBitmapPickerCombo();

	int AddBitmap(const CBitmap* bitmap, const wchar_t* string = nullptr);

	int InsertBitmap(int index, const CBitmap* bitmap, const wchar_t* string = nullptr);

protected:
	void DrawItem(LPDRAWITEMSTRUCT drawItemStruct) override;

	void MeasureItem(LPMEASUREITEMSTRUCT lpMIS) noexcept override;

	int AddString(const wchar_t*) noexcept {
		return -1;
	} // hides non-virtual function of parent
	int InsertString(int, const wchar_t*) noexcept {
		return -1;
	} // hides non-virtual function of parent
};
