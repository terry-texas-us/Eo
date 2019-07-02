#pragma once
class EoDbBitmapFile final : public CFile {
public:
	EoDbBitmapFile(const CString& fileName);

	bool Load(const CString& fileName, CBitmap& bitmap, CPalette& palette);
};
