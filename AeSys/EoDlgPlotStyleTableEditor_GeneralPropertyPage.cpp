#include "stdafx.h"
#include <Shlwapi.h>
#include "EoDlgPlotStyleTableEditor_GeneralPropertyPage.h"
IMPLEMENT_DYNCREATE(EoDlgPlotStyleEditor_GeneralPropertyPage, CPropertyPage)

EoDlgPlotStyleEditor_GeneralPropertyPage::EoDlgPlotStyleEditor_GeneralPropertyPage()
	: CPropertyPage(IDD) {
}

void EoDlgPlotStyleEditor_GeneralPropertyPage::DoDataExchange(CDataExchange* dataExchange) {
	CPropertyPage::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_PS_GENERAL_EDIT_DESCRIPTION, m_editDescription);
	DDX_Control(dataExchange, IDC_PS_GENERAL_EDIT_SCALE_FACTOR, m_editScaleFactor);
	DDX_Control(dataExchange, IDC_PS_GENERAL_CHECK_SCALE_FACTOR, m_checkScaleFactor);
	DDX_Control(dataExchange, IDC_PS_GENERAL_STATIC_FILEPATH, m_staticFilepath);
	DDX_Control(dataExchange, IDC_PS_GENERAL_STATIC_FILE_NAME, m_staticFilename);
	DDX_Control(dataExchange, IDC_PS_GENERAL_STATIC_BITMAP, m_staticBitmap);
	DDX_Control(dataExchange, IDC_PS_GENERAL_STATIC_REGULAR, m_staticRegular);
}

BEGIN_MESSAGE_MAP(EoDlgPlotStyleEditor_GeneralPropertyPage, CPropertyPage)
		ON_EN_CHANGE(IDC_PS_GENERAL_EDIT_DESCRIPTION, OnChangeEditDescription)
		ON_BN_CLICKED(IDC_PS_GENERAL_CHECK_SCALE_FACTOR, OnCheckScaleFactor)
		ON_EN_CHANGE(IDC_PS_GENERAL_EDIT_SCALE_FACTOR, OnEditScaleFactor)
END_MESSAGE_MAP()

void DrawTransparentBitmap(const HDC hdc, const HBITMAP bitmap, const short xStart, const short yStart, const COLORREF transparentColor) noexcept {
	const auto hdcTemp {CreateCompatibleDC(hdc)};
	SelectObject(hdcTemp, bitmap);
	BITMAP bm;
	GetObjectW(bitmap, sizeof(BITMAP), reinterpret_cast<LPSTR>(& bm));
	POINT ptSize {bm.bmWidth, bm.bmHeight};
	DPtoLP(hdcTemp, &ptSize, 1);
	const auto hdcBack {CreateCompatibleDC(hdc)};
	const auto hdcObject {CreateCompatibleDC(hdc)};
	const auto hdcMem {CreateCompatibleDC(hdc)};
	const auto hdcSave {CreateCompatibleDC(hdc)};
	const auto bmAndBack {CreateBitmap(ptSize.x, ptSize.y, 1, 1, nullptr)};
	const auto bmAndObject {CreateBitmap(ptSize.x, ptSize.y, 1, 1, nullptr)};
	const auto bmAndMem {CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y)};
	const auto bmSave {CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y)};
	const auto bmBackOld {static_cast<HBITMAP>(SelectObject(hdcBack, bmAndBack))};
	const auto bmObjectOld {static_cast<HBITMAP>(SelectObject(hdcObject, bmAndObject))};
	const auto bmMemOld {static_cast<HBITMAP>(SelectObject(hdcMem, bmAndMem))};
	const auto bmSaveOld {static_cast<HBITMAP>(SelectObject(hdcSave, bmSave))};
	SetMapMode(hdcTemp, GetMapMode(hdc));
	BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);
	const auto BackgroundColor {SetBkColor(hdcTemp, transparentColor)};
	BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);
	SetBkColor(hdcTemp, BackgroundColor);
	BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY);
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart, SRCCOPY);
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);
	BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);
	BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY);
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

bool EoDlgPlotStyleEditor_GeneralPropertyPage::SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable) noexcept {
	if (pPlotStyleTable == nullptr) { return false; }
	m_pPlotStyleTable = pPlotStyleTable;
	return true;
}

void WinPathToDos(wchar_t* str) {
	CString pStr = str;
	CString sNewStr;
	auto pos {0};
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
	sNewStr.ReleaseBuffer();
}

BOOL EoDlgPlotStyleEditor_GeneralPropertyPage::OnInitDialog() {
	CPropertyPage::OnInitDialog();
	if (m_pPlotStyleTable == nullptr) { return FALSE; }
	const auto description {m_pPlotStyleTable->description()};
	m_editDescription.SetWindowTextW(description);
	const auto check {m_pPlotStyleTable->isApplyScaleFactor()};
	m_checkScaleFactor.SetCheck(static_cast<int>(check));
	m_editScaleFactor.EnableWindow(static_cast<BOOL>(check));
	OdString sScaleFactor;
	sScaleFactor.format(L"%.1f", m_pPlotStyleTable->scaleFactor());
	m_editScaleFactor.SetWindowTextW(sScaleFactor);
	const auto editDC {::GetDC(m_staticFilepath.m_hWnd)};
	//  CRect rect;
	//  m_staticFilepath.GetClientRect(&rect);
	wchar_t buffer[MAX_PATH];
	wcscpy(buffer, m_sFileBufPath);
	WinPathToDos(buffer);
	const auto lpStr {buffer};
	PathCompactPathW(editDC, lpStr, 630/*rect.right*/);
	m_staticFilepath.SetWindowTextW(lpStr);
	const auto sFileName {m_sFileBufPath.right(m_sFileBufPath.getLength() - m_sFileBufPath.reverseFind('\\') - 1)};
	m_staticFilename.SetWindowTextW(sFileName);
	if (m_pPlotStyleTable->isAciTableAvailable()) { m_staticRegular.SetWindowTextW(L"Legacy (can be used to import old DWGs)"); }
	const auto BitmapHandle {
	static_cast<HBITMAP>(LoadImageW(AfxGetInstanceHandle(), MAKEINTRESOURCEW(m_pPlotStyleTable->isAciTableAvailable() ? IDB_PS_BITMAP_GENERAL_CTB : IDB_PS_BITMAP_GENERAL_STB), IMAGE_BITMAP, 32, 32, LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS))
	};
	const CClientDC ClientDeviceContext(&m_staticBitmap);
	DrawTransparentBitmap(ClientDeviceContext.m_hDC, BitmapHandle, 0, 0, 0x00FFFFFF);
	m_staticBitmap.SetBitmap(BitmapHandle);
	return TRUE;
}

void EoDlgPlotStyleEditor_GeneralPropertyPage::SetFileBufPath(OdString filePath) {
	m_sFileBufPath = filePath;
}

void EoDlgPlotStyleEditor_GeneralPropertyPage::OnChangeEditDescription() {
	CString pVal;
	m_editDescription.GetWindowText(pVal);
	m_pPlotStyleTable->setDescription(OdString(pVal));
}

void EoDlgPlotStyleEditor_GeneralPropertyPage::OnCheckScaleFactor() {
	const auto Check {m_checkScaleFactor.GetCheck()};
	m_pPlotStyleTable->setApplyScaleFactor(Check != 0);
	m_editScaleFactor.EnableWindow(Check);
}

void EoDlgPlotStyleEditor_GeneralPropertyPage::OnEditScaleFactor() {
	CString String;
	m_editScaleFactor.GetWindowTextW(String);
	double ScaleFactor;
	swscanf(String, L"%lf", &ScaleFactor);
	if (ScaleFactor <= 0.0 || ScaleFactor > gc_PlotStyleEditMaxScaleFactor) {
		ScaleFactor = 0.01;
		m_editScaleFactor.SetWindowTextW(L"0.01");
	}
	m_pPlotStyleTable->setScaleFactor(ScaleFactor);
}

void EoDlgPlotStyleEditor_GeneralPropertyPage::OnOK() {
	CPropertyPage::OnOK();
}
