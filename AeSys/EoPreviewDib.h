#pragma once

#include "ThumbnailImage.h"

class EoPreviewDib : public CAbstractPreview {
public:
	void  SetPreviewFile(const wchar_t* fileName) override;
	void  DrawPreview(HDC deviceContext, int x, int y, int width, int height) override;
	CRect Calc(int bmpWid,int bmpDep,int wndWid,int wndDep) noexcept;

	OdThumbnailImage m_odImage;
};
