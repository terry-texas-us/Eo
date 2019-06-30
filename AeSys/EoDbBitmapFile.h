#pragma once
class EoDbBitmapFile final : public CFile {
public:
	EoDbBitmapFile() = default;
	EoDbBitmapFile(const CString& fileName);
	~EoDbBitmapFile() = default;
	bool Load(const CString& fileName, CBitmap& bitmap, CPalette& palette);
};
