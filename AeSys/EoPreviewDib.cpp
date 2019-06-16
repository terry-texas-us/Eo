#include "stdafx.h"

#include "AeSys.h"

#include "FileDlgExt.h"
#include "EoPreviewDib.h"

void EoPreviewDib::SetPreviewFile(const wchar_t* fileName) {
	CString FileName(fileName);

	m_odImage.header.clear();
	m_odImage.bmp.clear();
	m_odImage.wmf.clear();
	m_odImage.png.clear();

	if (!FileName.GetLength()) { return; }

	CString Extension {FileName.Right(4)};

	if (Extension.CompareNoCase(L".dwg") == 0 || Extension.CompareNoCase(L".dxf") == 0) {
		OdStreamBufPtr FileStreamBuffer(theApp.createFile((const wchar_t*)FileName));
		try {
			odDbGetPreviewBitmap(FileStreamBuffer, &m_odImage);
			m_odImage.convPngToBmp();
		}
		catch (...) {
			return;
		}
	}
}
CRect EoPreviewDib::Calc(int bmpWid, int bmpDep, int wndWid, int wndDep) noexcept {
	int d;
	int w;
	if (bmpDep > bmpWid) {
		d = __min(bmpDep, wndDep);
		w = (bmpWid * d) / bmpDep;
		if (w > wndWid) {
			d = (d * wndWid) / w;
			w = wndWid;
		}
	}
	else {
		w = __min(bmpWid, wndWid);
		d = (bmpDep * w) / bmpWid;
		if (d > wndDep) {
			w = (w * wndDep) / d;
			d = wndDep;
		}
	}
	const int x = (wndWid - w) / 2;
	const int y = (wndDep - d) / 2;
	return CRect(x, y, x + w, y + d);
}

// Placeable metafile data definitions
typedef struct tagOLDRECT {
	short left;
	short top;
	short right;
	short bottom;
} OLDRECT;

// Placeable metafile header
typedef struct {
	unsigned long key;
	unsigned short hmf;
	OLDRECT bbox;
	unsigned short inch;
	unsigned long reserved;
	unsigned short checksum;
} ALDUSMFHEADER;

#define METAFILE_VERSION 1
#define ALDUSKEY 0x9AC6CDD7
#define ALDUSMFHEADERSIZE 22  // Avoid sizeof is struct alignment > 1

void EoPreviewDib::DrawPreview(HDC dc, int X, int Y, int width, int height) {
	CRect cr;

	if (m_odImage.hasBmp()) {
		BITMAPINFOHEADER* pHeader;
		pHeader = (BITMAPINFOHEADER*)(m_odImage.bmp.begin());

		cr = Calc(pHeader->biWidth, pHeader->biHeight, width, height);

		auto p = (unsigned char*)pHeader;
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
		}
		StretchDIBits(dc, cr.left + X, cr.top + Y, cr.Width(), cr.Height(), 0, 0, pHeader->biWidth, pHeader->biHeight, (const void*)p, (CONST BITMAPINFO*) pHeader, DIB_RGB_COLORS, SRCCOPY);
	}
	else if (m_odImage.hasWmf()) {
		CDC newDC;
		unsigned long dwIsAldus {0};
		METAHEADER* mfHeader {nullptr};
		ALDUSMFHEADER* aldusMFHeader {nullptr};

		unsigned long dwSize {0};
		unsigned long seekpos {0};

		newDC.Attach(dc);
		dwIsAldus = *((unsigned long*)m_odImage.wmf.begin());

		if (dwIsAldus != ALDUSKEY) {
			seekpos = 0;
		} else {
			aldusMFHeader = (ALDUSMFHEADER*)m_odImage.wmf.begin();
			seekpos = ALDUSMFHEADERSIZE;
		}
		auto p = (unsigned char*)m_odImage.wmf.begin();
		mfHeader = (METAHEADER*)(p + seekpos);

		if ((mfHeader->mtType != 1) && (mfHeader->mtType != 2)) { return; }

		dwSize = mfHeader->mtSize * 2;
		// Create the enhanced metafile
		auto MetaFileHandle {::SetWinMetaFileBits(dwSize, (const unsigned char*) mfHeader, nullptr, nullptr)};

		CSize size {0, 0};

		if (aldusMFHeader) {
			size.cx = 254 * (aldusMFHeader->bbox.right - aldusMFHeader->bbox.left) / aldusMFHeader->inch;
			size.cy = 254 * (aldusMFHeader->bbox.bottom - aldusMFHeader->bbox.top) / aldusMFHeader->inch;
		}
		cr = Calc(size.cx, size.cy, width, height);
		cr.OffsetRect(X, Y);
		newDC.PlayMetaFile(MetaFileHandle, &cr);
	}
}
