#include "stdafx.h"
#include "EoDbBitmapFile.h"

EoDbBitmapFile::EoDbBitmapFile(const CString& fileName) {
	CFileException e;
	if (CFile::Open(fileName, modeRead | shareDenyNone, &e) != 0) {
	}
}

bool EoDbBitmapFile::Load(const CString& fileName, CBitmap& bitmap, CPalette& palette) {
	const auto Bitmap {static_cast<HBITMAP>(LoadImageW(nullptr, fileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE))};
	if (Bitmap == nullptr) { return false; }
	bitmap.Attach(Bitmap);
	CClientDC ClientDeviceContext(nullptr);
	
	// Return now if device does not support palettes
	if ((ClientDeviceContext.GetDeviceCaps(RASTERCAPS) & RC_PALETTE) == 0) { return true; }
	DIBSECTION ds;
	bitmap.GetObjectW(sizeof(DIBSECTION), &ds);
	int NumberOfColors;
	if (ds.dsBmih.biClrUsed != 0) {
		NumberOfColors = static_cast<int>(ds.dsBmih.biClrUsed);
	} else {
		NumberOfColors = 1U << ds.dsBmih.biBitCount;
	}
	if (NumberOfColors > 256) { // Create a halftone palette
		palette.CreateHalftonePalette(&ClientDeviceContext);
	} else { // Create a custom palette from the DIB section's color table
		const auto RGBQuad {new RGBQUAD[static_cast<unsigned>(NumberOfColors)]};
		CDC dcMem;
		dcMem.CreateCompatibleDC(&ClientDeviceContext);
		auto Bitmap {dcMem.SelectObject(&bitmap)};
		GetDIBColorTable(static_cast<HDC>(dcMem), 0, static_cast<unsigned>(NumberOfColors), RGBQuad);
		dcMem.SelectObject(Bitmap);
		const auto Size {sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * (NumberOfColors - 1)};
		auto LogicalPalette {reinterpret_cast<LOGPALETTE*>(new unsigned char[Size])};
		LogicalPalette->palVersion = 0x300;
		LogicalPalette->palNumEntries = static_cast<unsigned short>(NumberOfColors);
		for (auto i = 0; i < NumberOfColors; i++) {
			LogicalPalette->palPalEntry[i].peRed = RGBQuad[i].rgbRed;
			LogicalPalette->palPalEntry[i].peGreen = RGBQuad[i].rgbGreen;
			LogicalPalette->palPalEntry[i].peBlue = RGBQuad[i].rgbBlue;
			LogicalPalette->palPalEntry[i].peFlags = 0;
		}
		palette.CreatePalette(LogicalPalette);
		delete[] LogicalPalette;
		delete [] RGBQuad;
	}
	Close();
	return true;
}
