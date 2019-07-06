#include "stdafx.h"
#include "AeSys.h"
#include <FileDlgExt.h>
#include "EoPreviewDib.h"

void EoPreviewDib::SetPreviewFile(const wchar_t* fileName) {
	const CString FileName(fileName);
	m_odImage.header.clear();
	m_odImage.bmp.clear();
	m_odImage.wmf.clear();
	m_odImage.png.clear();
	if (FileName.GetLength() == 0) { return; }
	const auto Extension {FileName.Right(4)};
	if (Extension.CompareNoCase(L".dwg") == 0 || Extension.CompareNoCase(L".dxf") == 0) {
		auto FileStreamBuffer(theApp.createFile(static_cast<const wchar_t*>(FileName)));
		try {
			odDbGetPreviewBitmap(FileStreamBuffer, &m_odImage);
			m_odImage.convPngToBmp();
		} catch (...) {
		}
	}
}

CRect EoPreviewDib::Calc(int bmpWid, int bmpDep, int wndWid, int wndDep) noexcept {
	int d;
	int w;
	if (bmpDep > bmpWid) {
		d = __min(bmpDep, wndDep);
		w = bmpWid * d / bmpDep;
		if (w > wndWid) {
			d = d * wndWid / w;
			w = wndWid;
		}
	} else {
		w = __min(bmpWid, wndWid);
		d = bmpDep * w / bmpWid;
		if (d > wndDep) {
			w = w * wndDep / d;
			d = wndDep;
		}
	}
	const auto x {(wndWid - w) / 2};
	const auto y {(wndDep - d) / 2};
	return CRect(x, y, x + w, y + d);
}

// Placeable metafile data definitions
using OLDRECT = struct tagOLDRECT {
	short left;
	short top;
	short right;
	short bottom;
};

// Placeable metafile header
using ALDUSMFHEADER = struct {
	unsigned long key;
	unsigned short hmf;
	OLDRECT bbox;
	unsigned short inch;
	unsigned long reserved;
	unsigned short checksum;
};

constexpr auto gc_AldusKey = 0x9AC6CDD7;
constexpr unsigned long gc_AldusMetafileHeaderSize = 22;

void EoPreviewDib::DrawPreview(const HDC deviceContext, const int x, const int y, const int width, const int height) {
	CRect BitmapRectangle;
	if (m_odImage.hasBmp()) {
		const auto pHeader {reinterpret_cast<tagBITMAPINFOHEADER*>(m_odImage.bmp.begin())};
		BitmapRectangle = Calc(pHeader->biWidth, pHeader->biHeight, width, height);
		auto p = reinterpret_cast<unsigned char*>(pHeader);
		p += pHeader->biSize;
		switch (pHeader->biBitCount) {
			case 1:
				p += sizeof(RGBQUAD) * 2;
				break;
			case 4:
				p += sizeof(RGBQUAD) * 16;
				break;
			case 8:
				p += sizeof(RGBQUAD) * 256;
				break;
			default: ;
		}
		StretchDIBits(deviceContext, BitmapRectangle.left + x, BitmapRectangle.top + y, BitmapRectangle.Width(), BitmapRectangle.Height(), 0, 0, pHeader->biWidth, pHeader->biHeight, static_cast<const void*>(p), reinterpret_cast<CONST BITMAPINFO*>(pHeader), DIB_RGB_COLORS, SRCCOPY);
	} else if (m_odImage.hasWmf()) {
		CDC NewDeviceContext;
		ALDUSMFHEADER* AldusMfHeader {nullptr};
		unsigned long SeekPosition;
		NewDeviceContext.Attach(deviceContext);
		const auto IsAldus {*reinterpret_cast<unsigned long*>(m_odImage.wmf.begin())};
		if (IsAldus != gc_AldusKey) {
			SeekPosition = 0;
		} else {
			AldusMfHeader = reinterpret_cast<ALDUSMFHEADER*>(m_odImage.wmf.begin());
			SeekPosition = gc_AldusMetafileHeaderSize;
		}
		const auto p {static_cast<unsigned char*>(m_odImage.wmf.begin())};
		const auto MetaHeader {reinterpret_cast<METAHEADER*>(p + SeekPosition)};
		if (MetaHeader->mtType != 1 && MetaHeader->mtType != 2) { return; }
		const auto Size {MetaHeader->mtSize * 2};
		// Create the enhanced metafile
		const auto MetaFileHandle {SetWinMetaFileBits(Size, reinterpret_cast<const unsigned char*>(MetaHeader), nullptr, nullptr)};
		CSize InitialSize {0, 0};
		if (AldusMfHeader != nullptr) {
			InitialSize.cx = 254 * (AldusMfHeader->bbox.right - AldusMfHeader->bbox.left) / AldusMfHeader->inch;
			InitialSize.cy = 254 * (AldusMfHeader->bbox.bottom - AldusMfHeader->bbox.top) / AldusMfHeader->inch;
		}
		BitmapRectangle = Calc(InitialSize.cx, InitialSize.cy, width, height);
		BitmapRectangle.OffsetRect(x, y);
		NewDeviceContext.PlayMetaFile(MetaFileHandle, &BitmapRectangle);
	}
}
