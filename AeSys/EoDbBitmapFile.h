#pragma once

class EoDbBitmapFile : public CFile {
public:
	EoDbBitmapFile() noexcept {
	}
	EoDbBitmapFile(const CString& strPathName);

	~EoDbBitmapFile() = default;
	
	bool Load(const CString& strPathName, CBitmap& bm, CPalette& pal);
};
