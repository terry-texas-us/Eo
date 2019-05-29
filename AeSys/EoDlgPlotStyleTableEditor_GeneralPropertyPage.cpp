#include "stdafx.h"

#include "Shlwapi.h"
#include "EoDlgPlotStyleTableEditor_GeneralPropertyPage.h"

IMPLEMENT_DYNCREATE(EoDlgPlotStyleEditor_GeneralPropertyPage, CPropertyPage)

EoDlgPlotStyleEditor_GeneralPropertyPage::EoDlgPlotStyleEditor_GeneralPropertyPage() :
	CPropertyPage(EoDlgPlotStyleEditor_GeneralPropertyPage::IDD), m_pPlotStyleTable(0) {
}
EoDlgPlotStyleEditor_GeneralPropertyPage::~EoDlgPlotStyleEditor_GeneralPropertyPage() {
}
void EoDlgPlotStyleEditor_GeneralPropertyPage::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PS_GENERAL_EDIT_DESCRIPTION, m_editDescription);
	DDX_Control(pDX, IDC_PS_GENERAL_EDIT_SCALE_FACTOR, m_editScalefactor);
	DDX_Control(pDX, IDC_PS_GENERAL_CHECK_SCALE_FACTOR, m_checkScalefactor);
	DDX_Control(pDX, IDC_PS_GENERAL_STATIC_FILEPATH, m_staticFilepath);
	DDX_Control(pDX, IDC_PS_GENERAL_STATIC_FILE_NAME, m_staticFilename);
	DDX_Control(pDX, IDC_PS_GENERAL_STATIC_BITMAP, m_staticBitmap);
	DDX_Control(pDX, IDC_PS_GENERAL_STATIC_REGULAR, m_staticRegular);
}

BEGIN_MESSAGE_MAP(EoDlgPlotStyleEditor_GeneralPropertyPage, CPropertyPage)
	ON_EN_CHANGE(IDC_PS_GENERAL_EDIT_DESCRIPTION, OnChangeEditDescription)
	ON_BN_CLICKED(IDC_PS_GENERAL_CHECK_SCALE_FACTOR, OnCheckScalefactor)
	ON_EN_CHANGE(IDC_PS_GENERAL_EDIT_SCALE_FACTOR, OnEditScalefactor)

END_MESSAGE_MAP()

void DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, short xStart, short yStart, COLORREF cTransparentColor) noexcept {
	BITMAP bm;
	COLORREF cColor;
	HBITMAP bmAndBack, bmAndObject, bmAndMem, bmSave;
	HBITMAP bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
	HDC hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
	POINT ptSize;

	hdcTemp = CreateCompatibleDC(hdc);
	SelectObject(hdcTemp, hBitmap);

	GetObject(hBitmap, sizeof(BITMAP), (LPSTR) & bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;
	DPtoLP(hdcTemp, &ptSize, 1);

	hdcBack = CreateCompatibleDC(hdc);
	hdcObject = CreateCompatibleDC(hdc);
	hdcMem = CreateCompatibleDC(hdc);
	hdcSave = CreateCompatibleDC(hdc);

	bmAndBack = CreateBitmap(ptSize.x, ptSize.y, 1, 1, nullptr);

	bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, nullptr);

	bmAndMem = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
	bmSave = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

	bmBackOld = (HBITMAP) SelectObject(hdcBack, bmAndBack);
	bmObjectOld = (HBITMAP) SelectObject(hdcObject, bmAndObject);
	bmMemOld = (HBITMAP) SelectObject(hdcMem, bmAndMem);
	bmSaveOld = (HBITMAP) SelectObject(hdcSave, bmSave);

	SetMapMode(hdcTemp, GetMapMode(hdc));

	BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

	cColor = SetBkColor(hdcTemp, cTransparentColor);

	BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
		SRCCOPY);

	SetBkColor(hdcTemp, cColor);

	BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
		NOTSRCCOPY);

	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
		SRCCOPY);

	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

	BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

	BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
		SRCCOPY);

	BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(hdcBack, bmBackOld));
	DeleteObject(SelectObject(hdcObject, bmObjectOld));
	DeleteObject(SelectObject(hdcMem, bmMemOld));
	DeleteObject(SelectObject(hdcSave, bmSaveOld));

	DeleteDC(hdcMem);
	DeleteDC(hdcBack);
	DeleteDC(hdcObject);
	DeleteDC(hdcSave);
	DeleteDC(hdcTemp);
}

const bool EoDlgPlotStyleEditor_GeneralPropertyPage::SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable) noexcept {
	if (!pPlotStyleTable) {
		return false;
	}
	m_pPlotStyleTable = pPlotStyleTable;
	return true;
}

void WinPathToDos(wchar_t* str) {
	CString pStr = str;
	CString sNewStr;
	int pos = 0;
	while (pos >= 0) {

		pos = pStr.Find('\\');
		CString s;
		if (pos < 0) {
			s = pStr;
		} else {
			s = pStr.Left(pos);
			if (s.GetLength() > 8) {
				s = s.Left(6);
				s.MakeUpper();
				s += L"~1";
		//        s += "\\";
			}
			s += L"\\";
			pStr = pStr.Right(pStr.GetLength() - pos - 1);
		}

		sNewStr += s;
	}

	wcscpy(str, sNewStr.GetBuffer(sNewStr.GetLength()));
}

BOOL EoDlgPlotStyleEditor_GeneralPropertyPage::OnInitDialog() {
	CPropertyPage::OnInitDialog();

	if (!m_pPlotStyleTable) return FALSE;

	OdString description = m_pPlotStyleTable->description();
	m_editDescription.SetWindowText(description);

	const bool check = m_pPlotStyleTable->isApplyScaleFactor();
	m_checkScalefactor.SetCheck(check);
	m_editScalefactor.EnableWindow(check);
	OdString sScaleFactor;
	sScaleFactor.format(L"%.1f", m_pPlotStyleTable->scaleFactor());
	m_editScalefactor.SetWindowText(sScaleFactor);


	HDC editDC = ::GetDC(m_staticFilepath.m_hWnd);
  //  CRect rect;
  //  m_staticFilepath.GetClientRect(&rect);
	wchar_t buffer[MAX_PATH];
	wcscpy(buffer, m_sFileBufPath);
	WinPathToDos(buffer);
	wchar_t* lpStr = buffer;
	PathCompactPath(editDC, lpStr, 630/*rect.right*/);
	m_staticFilepath.SetWindowText(lpStr);

	OdString sFileName = m_sFileBufPath.right(m_sFileBufPath.getLength() - m_sFileBufPath.reverseFind('\\') - 1);
	m_staticFilename.SetWindowText(sFileName);

	if (m_pPlotStyleTable->isAciTableAvailable())
		m_staticRegular.SetWindowText(L"Legacy (can be used to import old DWGs)");

	auto BitmapHandle {static_cast<HBITMAP>(::LoadImageW(AfxGetInstanceHandle(), MAKEINTRESOURCEW(m_pPlotStyleTable->isAciTableAvailable() ? IDB_PS_BITMAP_GENERAL_CTB : IDB_PS_BITMAP_GENERAL_STB), IMAGE_BITMAP, 32, 32, LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS))};

	CClientDC ClientDeviceContext(&m_staticBitmap);
	DrawTransparentBitmap(ClientDeviceContext.m_hDC, BitmapHandle, 0, 0, 0x00FFFFFF);
	m_staticBitmap.SetBitmap(BitmapHandle);

	return TRUE;

}



void EoDlgPlotStyleEditor_GeneralPropertyPage::SetFileBufPath(const OdString sFilePath) {
	m_sFileBufPath = sFilePath;
}

void EoDlgPlotStyleEditor_GeneralPropertyPage::OnChangeEditDescription() {
	CString pVal;
	m_editDescription.GetWindowText(pVal);
	m_pPlotStyleTable->setDescription(OdString(pVal));
}

void EoDlgPlotStyleEditor_GeneralPropertyPage::OnCheckScalefactor() {
	const int check = m_checkScalefactor.GetCheck();
	m_pPlotStyleTable->setApplyScaleFactor(check ? true : false);
	m_editScalefactor.EnableWindow(check);
}

void EoDlgPlotStyleEditor_GeneralPropertyPage::OnEditScalefactor() {
	CString pVal;
	m_editScalefactor.GetWindowText(pVal);
	double scaleFactor;
	_stscanf(pVal, L"%lf", &scaleFactor);
	if (scaleFactor <= 0 || scaleFactor > PS_EDIT_MAX_SCALEFACTOR) {
		scaleFactor = 0.01;
		m_editScalefactor.SetWindowText(L"0.01");
	}

  /*  char buffer[15];
	_gcvt(scaleFactor, 10, buffer);
	m_editScalefactor.SetWindowText(buffer);*/
	m_pPlotStyleTable->setScaleFactor(scaleFactor);
}
void EoDlgPlotStyleEditor_GeneralPropertyPage::OnOK() {
	CPropertyPage::OnOK();
}
