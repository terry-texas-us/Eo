#pragma once

class EoDbBitmapFile : public CFile {
public:
	EoDbBitmapFile() {
	}
	EoDbBitmapFile(const CString& strPathName);

	~EoDbBitmapFile() {
	}
	bool Load(const CString& strPathName, CBitmap& bm, CPalette& pal);
};
