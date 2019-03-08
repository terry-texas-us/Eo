#pragma once

class EoCtrlBitmapPickerCombo : public CComboBox {
  int m_ItemWidth;
  int m_ItemHeight;

  void OutputBitmap(LPDRAWITEMSTRUCT drawItemStruct, bool selected);
  void SetSize(int width, int height);

public:
  EoCtrlBitmapPickerCombo();

  virtual ~EoCtrlBitmapPickerCombo() {}

  int AddBitmap(const CBitmap *bitmap, LPCWSTR string = NULL);
  int InsertBitmap(int nIndex, const CBitmap *bitmap, LPCWSTR string = NULL);

protected:
  virtual void DrawItem(LPDRAWITEMSTRUCT drawItemStruct);

  virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
  virtual int AddString(LPCWSTR ) { return -1; }
  virtual int InsertString(int , LPCWSTR ) { return -1; }
};