#include "stdafx.h"
#include "DbDatabase.h"
#include "EoDlgPlotStyleEditLineweight.h"
#include "EoDlgPlotStyleTableEditor_FormViewPropertyPage.h"
#include "Ps/PlotStyles.h"
#include "ColorMapping.h"
#include "DynamicLinker.h"
#include "WindowsX.h"

void Dlg_OnClose(const HWND hwnd) noexcept {
	DestroyWindow(hwnd);
}

void Dlg_OnCommand(const HWND hwnd, const int id, HWND hwndCtl, const unsigned codeNotify) {
	switch (id) {
		case IDOK: {
			const auto nMaxCount {100};
			wchar_t sString[nMaxCount] {L"\0"};
			const auto hOwner {GetWindow(hwnd, GW_OWNER)};
			::GetWindowText(GetDlgItem(hwnd, IDC_PS_ADDPS_EDIT_PSNAME), sString, nMaxCount);
			const auto pPsDlg {static_cast<CPropertySheet*>(CWnd::FromHandle(hOwner))};
			auto ActivePage {static_cast<EoDlgPlotStyleEditor_FormViewPropertyPage*>(pPsDlg->GetActivePage())};
			ActivePage->AddNewPlotStyle(sString);
		}
		case IDCANCEL:
			EndDialog(hwnd, id);
			break;
		case IDC_PS_ADDPS_EDIT_PSNAME: {
			if (codeNotify == EN_CHANGE) {
				const auto nMaxCount {100};
				wchar_t sString[nMaxCount];
				const auto hOwner {GetWindow(hwnd, GW_OWNER)};
				::GetWindowText(GetDlgItem(hwnd, IDC_PS_ADDPS_EDIT_PSNAME), sString, nMaxCount);
				const auto pPsDlg {static_cast<CPropertySheet*>(CWnd::FromHandle(hOwner))};
				const auto ActivePage {static_cast<EoDlgPlotStyleEditor_FormViewPropertyPage*>(pPsDlg->GetActivePage())};
				const auto PlotStyleTable {ActivePage->GetPlotStyleTable()};
				const auto hDInfo {GetDlgItem(hwnd, IDC_PS_ADDPS_STATIC_DINFO)};
				const auto hSInfo {GetDlgItem(hwnd, IDC_PS_ADDPS_STATIC_SINFO)};
				const auto hOkBtn {GetDlgItem(hwnd, IDOK)};
				CString NewName {sString};
				NewName.MakeLower();
				if (NewName.IsEmpty()) {
					EnableWindow(hOkBtn, FALSE);
					return;
				}
				for (unsigned PlotStyleIndex = 0; PlotStyleIndex < PlotStyleTable->plotStyleSize(); PlotStyleIndex++) {
					auto PlotStyle {PlotStyleTable->plotStyleAt(static_cast<int>(PlotStyleIndex))};
					CString PlotStyleName = static_cast<const wchar_t*>(PlotStyle->localizedName());
					PlotStyleName.MakeLower();
					if (PlotStyleName == NewName) {
						CString sInfo;
						sInfo.Format(L"A style named <%s> already exists.", sString);
						SetWindowTextW(hDInfo, sInfo);
						ShowWindow(hDInfo, SW_SHOW);
						ShowWindow(hSInfo, SW_SHOW);
						EnableWindow(hOkBtn, FALSE);
						return;
					}
				}
				ShowWindow(hDInfo, SW_HIDE);
				ShowWindow(hSInfo, SW_HIDE);
				EnableWindow(hOkBtn, TRUE);
				break;
			}
		}
	}
}

BOOL Dlg_OnInit(const HWND hwnd, HWND hwndCtl, LPARAM lParam) {
	const auto hOwner {GetWindow(hwnd, GW_OWNER)};
	CPropertySheet* pPsDlg = static_cast<CPropertySheet*>(CWnd::FromHandle(hOwner));
	const EoDlgPlotStyleEditor_FormViewPropertyPage* pPg = static_cast<EoDlgPlotStyleEditor_FormViewPropertyPage*>(pPsDlg->GetActivePage());
	const auto PlotStyleTable {pPg->GetPlotStyleTable()};
	OdString sName;
	sName.format(L"Style %d", PlotStyleTable->plotStyleSize());
	SetWindowTextW(GetDlgItem(hwnd, IDC_PS_ADDPS_EDIT_PSNAME), sName);
	return TRUE;
}

int WINAPI Dlg_Proc(const HWND hwnd, const unsigned uMsg, const WPARAM wParam, const LPARAM lParam) {
	switch (uMsg) {
		case WM_CLOSE:
			return SetDlgMsgResult(hwnd, uMsg, HANDLE_WM_CLOSE(hwnd, wParam, lParam, Dlg_OnClose));
		case WM_COMMAND:
			return SetDlgMsgResult(hwnd, uMsg, HANDLE_WM_COMMAND(hwnd, wParam, lParam, Dlg_OnCommand));
		case WM_INITDIALOG:
			return SetDlgMsgResult(hwnd, uMsg, HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Dlg_OnInit));
	}
	return FALSE;
}

void CBitmapColorInfo::GetBitmapSizes(CBitmap& bitmap, int& width, int& height) {
	BITMAP Bitmap;
	bitmap.GetBitmap(&Bitmap);
	width = Bitmap.bmWidth;
	height = Bitmap.bmHeight;
}

DIBCOLOR* CBitmapColorInfo::GetBitmapPixels(CBitmap& bitmap, int& width, int& height) {
	CDC DeviceContext;
	DeviceContext.CreateCompatibleDC(nullptr);
	GetBitmapSizes(bitmap, width, height);
	BITMAPINFO BitmapInfo;
	BitmapInfo.bmiHeader.biSize = sizeof BitmapInfo.bmiHeader;
	BitmapInfo.bmiHeader.biWidth = width;
	BitmapInfo.bmiHeader.biHeight = -height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	BitmapInfo.bmiHeader.biSizeImage = static_cast<unsigned long>(height * width * 4);
	BitmapInfo.bmiHeader.biClrUsed = 0;
	BitmapInfo.bmiHeader.biClrImportant = 0;
	void* Bits {new char[static_cast<unsigned>(height * width * 4)]};
	GetDIBits(DeviceContext.m_hDC, static_cast<HBITMAP>(bitmap.m_hObject), 0, static_cast<unsigned>(height), Bits, &BitmapInfo, DIB_RGB_COLORS);
	return static_cast<DIBCOLOR*>(Bits);
}

void CBitmapColorInfo::SetBitmapPixels(CBitmap& bitmap, DIBCOLOR* pixels) {
	CDC DeviceContext;
	DeviceContext.CreateCompatibleDC(nullptr);
	int Width;
	int Height;
	GetBitmapSizes(bitmap, Width, Height);
	BITMAPINFO BitmapInfo;
	BitmapInfo.bmiHeader.biSize = sizeof BitmapInfo.bmiHeader;
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = -Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	BitmapInfo.bmiHeader.biSizeImage = static_cast<unsigned long>(Height * Width * 4);
	BitmapInfo.bmiHeader.biClrUsed = 0;
	BitmapInfo.bmiHeader.biClrImportant = 0;
	SetDIBits(DeviceContext.m_hDC, static_cast<HBITMAP>(bitmap.m_hObject), 0, static_cast<unsigned>(Height), pixels, &BitmapInfo, DIB_RGB_COLORS);
	delete pixels;
}

CBitmap* CBitmapColorInfo::CloneBitmap(const CBitmap* sourceBitmap, CBitmap* clonedBitmap) {
	ASSERT(clonedBitmap);
	ASSERT(sourceBitmap);
	ASSERT(sourceBitmap != clonedBitmap);
	if (!clonedBitmap && !sourceBitmap && sourceBitmap == clonedBitmap) { return nullptr; }
	BITMAP Bitmap;
	const_cast<CBitmap*>(sourceBitmap)->GetBitmap(&Bitmap);
	CClientDC ClientDeviceContext(nullptr);
	CDC cdc;
	cdc.CreateCompatibleDC(&ClientDeviceContext);
	clonedBitmap->CreateCompatibleBitmap(&ClientDeviceContext, Bitmap.bmWidth, Bitmap.bmHeight);
	auto NumberOfBytes {gsl::narrow_cast<unsigned long>(Bitmap.bmWidthBytes * Bitmap.bmHeight)};
	const auto BitmapBuffer {new unsigned char[NumberOfBytes]};
	NumberOfBytes = sourceBitmap->GetBitmapBits(NumberOfBytes, BitmapBuffer);
	clonedBitmap->SetBitmapBits(NumberOfBytes, BitmapBuffer);
	delete[]BitmapBuffer;
	int Width;
	int Height;
	const auto buf {GetBitmapPixels(*const_cast<CBitmap*>(sourceBitmap), Width, Height)};
	SetBitmapPixels(*clonedBitmap, buf);
	return clonedBitmap;
}

void CBitmapColorInfo::PaintBitmap(CBitmap& bitmap, const COLORREF color) {
	int Width;
	int Height;
	const auto Bitmap {GetBitmapPixels(bitmap, Width, Height)};
	auto pColor {Bitmap};
	for (auto y = Height - 1; y >= 0; y--) {
		for (auto x = 0; x < Width; x++, pColor++) {
			*pColor = DIBCOLOR(color);
		}
	}
	SetBitmapPixels(bitmap, Bitmap);
}

const OdCmEntityColor CBitmapColorInfo::GetColor() {
	const auto EntityColor {OdCmEntityColor(static_cast<unsigned char>(m_color >> 16 & 0xFF), static_cast<unsigned char>(m_color >> 8 & 0xFF), static_cast<unsigned char>(m_color & 0xFF))};
	return EntityColor;
}

bool CBitmapColorInfo::IsColor(COLORREF color, const unsigned char item) noexcept {
	color = static_cast<unsigned long>((item << 24) + (GetRValue(color) << 16) + (GetGValue(color) << 8) + GetBValue(color));
	return m_color == color;
}

CBitmapColorInfo::CBitmapColorInfo(const CBitmap* bitmap, const COLORREF color, const unsigned char colorItem, const int colorIndex)
	: m_iItem(colorItem) {
	m_color = static_cast<unsigned long>((m_iItem << 24) + (GetRValue(color) << 16) + (GetGValue(color) << 8) + GetBValue(color));
	CloneBitmap(bitmap, &m_bitmap);
	PaintBitmap(m_bitmap, color);
	if (colorIndex <= 0) {
		wcscpy_s(m_name, PS_COLOR_MAX_NAME, L"Custom Color");
	} else {
		OdString ColorName;
		ColorName.format(L"Color %d", colorIndex);
		wcscpy_s(m_name, PS_COLOR_MAX_NAME, ColorName);
	}
}

CBitmapColorInfo::CBitmapColorInfo(const CBitmap* bitmap, const COLORREF color, const wchar_t* name)
	: m_iItem(0xff) {
	m_color = static_cast<unsigned long>((m_iItem << 24) + (GetRValue(color) << 16) + (GetGValue(color) << 8) + GetBValue(color));
	CloneBitmap(bitmap, &m_bitmap);
	PaintBitmap(m_bitmap, color);
	wcsncpy(m_name, name, PS_COLOR_MAX_NAME);
}

CBitmapColorInfo::CBitmapColorInfo(const wchar_t* resourceName, const wchar_t* name)
	: m_iItem(0xff)
	, m_color(0) {
	const auto BitmapHandle {static_cast<HBITMAP>(LoadImageW(AfxGetInstanceHandle(), resourceName, IMAGE_BITMAP, 13, 13, LR_CREATEDIBSECTION))};
	const auto Bitmap {CBitmap::FromHandle(BitmapHandle)};
	CloneBitmap(Bitmap, &m_bitmap);
	wcsncpy(m_name, name, PS_COLOR_MAX_NAME);
}

int CPsListStyleData::getPublicArrayIndexByColor(const COLORREF color) {
	for (unsigned PublicBitmapIndex = 0; PublicBitmapIndex < m_pPublicBitmapList->size(); PublicBitmapIndex++) {
		const auto EntityColor {OdCmEntityColor(GetRValue(color), GetGValue(color), GetBValue(color))};
		if ((*m_pPublicBitmapList)[PublicBitmapIndex]->GetColor() == EntityColor) { return static_cast<int>(PublicBitmapIndex); }
	}
	return -1;
}

CPsListStyleData::CPsListStyleData(OdPsPlotStyle* plotStyle, OdBitmapColorInfoArray* publicBitmapList, const char item)
	: m_pPlotStyles(plotStyle)
	, m_pPublicBitmapList(publicBitmapList)
	, m_pBitmapColorInfo(nullptr)
	, m_iActiveListIndex(0) {
	if (!m_pPlotStyles && !m_pPublicBitmapList) { return; }
	OdPsPlotStyleData OdPsData;
	plotStyle->getData(OdPsData);
	const auto PlotStyleDataColor {OdPsData.color()};
	unsigned long PlotStyleDataRgb;
	if (PlotStyleDataColor.isByACI()) {
		PlotStyleDataRgb = odcmLookupRGB(PlotStyleDataColor.colorIndex(), odcmAcadLightPalette());
	} else {
		PlotStyleDataRgb = RGB(PlotStyleDataColor.red(), PlotStyleDataColor.green(), PlotStyleDataColor.blue());
	}
	m_iActiveListIndex = getPublicArrayIndexByColor(PlotStyleDataRgb);
	if (m_iActiveListIndex < 0) {
		m_pBitmapColorInfo = new CBitmapColorInfo(&(*m_pPublicBitmapList)[m_pPublicBitmapList->size() - 1]->m_bitmap, PlotStyleDataRgb, static_cast<unsigned char>(item), PlotStyleDataColor.isByACI() ? PlotStyleDataColor.colorIndex() : -1);
	}
}

CPsListStyleData::~CPsListStyleData() {
	delete m_pBitmapColorInfo;
	m_pBitmapColorInfo = nullptr;
}

bool CPsListStyleData::SetActiveListIndex(const int index, const bool bitmapInfo) {
	if (!m_pPlotStyles && !m_pPublicBitmapList) { return false; }
	if (static_cast<unsigned>(index) >= m_pPublicBitmapList->size() - 1) { return false; }
	if (index < 0) { return false; }
	m_iActiveListIndex = index;
	if (bitmapInfo) return true;
	delete m_pBitmapColorInfo;
	m_pBitmapColorInfo = nullptr;
	return true;
}

bool CPsListStyleData::ReplaceBitmapColorInfo(const COLORREF color, const int item) {
	if (!m_pPlotStyles && !m_pPublicBitmapList) { return false; }
	delete m_pBitmapColorInfo;
	m_pBitmapColorInfo = nullptr;
	m_iActiveListIndex = getPublicArrayIndexByColor(color);
	if (m_iActiveListIndex < 0) {
		m_pBitmapColorInfo = new CBitmapColorInfo(&(*m_pPublicBitmapList)[m_pPublicBitmapList->size() - 1]->m_bitmap, color, static_cast<unsigned char>(item));
	}
	return true;
}

const OdCmEntityColor CPsListStyleData::GetColor() {
	if (m_iActiveListIndex < 0) { return m_pBitmapColorInfo->GetColor(); }
	return (*m_pPublicBitmapList)[static_cast<unsigned>(m_iActiveListIndex)]->GetColor();
}

IMPLEMENT_DYNCREATE(EoDlgPlotStyleEditor_FormViewPropertyPage, CPropertyPage)

EoDlgPlotStyleEditor_FormViewPropertyPage::EoDlgPlotStyleEditor_FormViewPropertyPage()
	: CPropertyPage(IDD) {
	m_pPlotStyleTable = nullptr;
	m_pPlotStyleActive = nullptr;
	m_bEditChanging = false;
}

EoDlgPlotStyleEditor_FormViewPropertyPage::~EoDlgPlotStyleEditor_FormViewPropertyPage() {
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PS_FORMVIEW_COMBO_DITHER, m_Dither);
	DDX_Control(pDX, IDC_PS_FORMVIEW_COMBO_GRAYSCALE, m_Grayscale);
	DDX_Control(pDX, IDC_PS_FORMVIEW_COMBO_ADAPTIVE, m_Adaptive);
	DDX_Control(pDX, IDC_PS_FORMVIEW_COMBO_LINETYPE, m_Linetype);
	DDX_Control(pDX, IDC_PS_FORMVIEW_COMBO_LINEWEIGHT, m_Lineweight);
	DDX_Control(pDX, IDC_PS_FORMVIEW_COMBO_LINEENDSTYLE, m_Lineendstyle);
	DDX_Control(pDX, IDC_PS_FORMVIEW_COMBO_LINEJOINSTYLE, m_Linejoinstyle);
	DDX_Control(pDX, IDC_PS_FORMVIEW_COMBO_FILLSTYLE, m_Fillstyle);
	DDX_Control(pDX, IDC_PS_FORMVIEW_COMBO_COLOR, m_Color);
	DDX_Control(pDX, IDC_PS_FORMVIEW_EDIT_DESCRIPTION, m_editDescription);
	DDX_Control(pDX, IDC_PS_FORMVIEW_SPIN_PEN, m_spinPen);
	DDX_Control(pDX, IDC_PS_FORMVIEW_EDIT_PEN, m_editPen);
	DDX_Control(pDX, IDC_PS_FORMVIEW_SPIN_VIRTPEN, m_spinVirtpen);
	DDX_Control(pDX, IDC_PS_FORMVIEW_EDIT_VIRTPEN, m_editVirtpen);
	DDX_Control(pDX, IDC_PS_FORMVIEW_SPIN_SCREENING, m_spinScreening);
	DDX_Control(pDX, IDC_PS_FORMVIEW_EDIT_SCREENING, m_editScreening);
	DDX_Control(pDX, IDC_PS_FORMVIEW_LIST_STYLES, m_listStyles);
	DDX_Control(pDX, IDC_PS_FORMVIEW_BTN_ADDSTYLE, m_AddstyleButton);
	DDX_Control(pDX, IDC_PS_FORMVIEW_BTN_DELSTYLE, m_DelstyleButton);
	DDX_Control(pDX, IDC_PS_FORMVIEW_BTN_LINEWEIGHT, m_LineweightButton);
	DDX_Control(pDX, IDC_PS_FORMVIEW_BTN_SAVE, m_SaveButton);
	m_spinPen.SetBuddy(&m_editPen);
	m_spinPen.SetRange(0, PS_SPIN_MAX_PEN);
	m_spinVirtpen.SetBuddy(&m_editVirtpen);
	m_spinVirtpen.SetRange(0, PS_SPIN_MAX_VIRTPEN);
	m_spinScreening.SetBuddy(&m_editScreening);
	m_spinScreening.SetRange(0, PS_SPIN_MAX_SCREENING);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnDestroy() {
	for (auto ListStyleIndex = 0; ListStyleIndex < m_listStyles.GetItemCount(); ++ListStyleIndex) {
		const auto pPsListStyleData {reinterpret_cast<CPsListStyleData*>(m_listStyles.GetItemData(ListStyleIndex))};
		delete pPsListStyleData;
	}
	for (auto& Bitmap : m_bitmapList) {
		delete Bitmap;
	}
	CPropertyPage::OnDestroy();
}

BEGIN_MESSAGE_MAP(EoDlgPlotStyleEditor_FormViewPropertyPage, CPropertyPage)
		ON_BN_CLICKED(IDC_PS_FORMVIEW_BTN_LINEWEIGHT, OnLineweightBtn)
		ON_BN_CLICKED(IDC_PS_FORMVIEW_BTN_SAVE, OnSaveBtn)
		ON_BN_CLICKED(IDC_PS_FORMVIEW_BTN_DELSTYLE, OnDelBtnStyle)
		ON_BN_CLICKED(IDC_PS_FORMVIEW_BTN_ADDSTYLE, OnAddBtnStyle)
		ON_EN_UPDATE(IDC_PS_FORMVIEW_EDIT_DESCRIPTION, OnUpdateEditDescription)
		ON_EN_CHANGE(IDC_PS_FORMVIEW_EDIT_DESCRIPTION, OnChangeEditDescription)
		ON_EN_CHANGE(IDC_PS_FORMVIEW_EDIT_VIRTPEN, OnChangeEditVirtPen)
		ON_EN_CHANGE(IDC_PS_FORMVIEW_EDIT_PEN, OnChangeEditPen)
		ON_EN_CHANGE(IDC_PS_FORMVIEW_EDIT_SCREENING, OnChangeEditScreening)
		ON_NOTIFY(LVN_ITEMCHANGED, IDC_PS_FORMVIEW_LIST_STYLES, OnItemchangedListStyles)
		ON_NOTIFY(LVN_ITEMCHANGING, IDC_PS_FORMVIEW_LIST_STYLES, OnItemchangingListStyles)
		ON_NOTIFY(UDN_DELTAPOS, IDC_PS_FORMVIEW_SPIN_PEN, OnDeltaposSpinPen)
		ON_CBN_SELCHANGE(IDC_PS_FORMVIEW_COMBO_COLOR, OnSelchangeComboColor)
		ON_CBN_SELENDOK(IDC_PS_FORMVIEW_COMBO_COLOR, OnSelendokComboColor)
		ON_CBN_SELCHANGE(IDC_PS_FORMVIEW_COMBO_DITHER, OnSelendokComboDither)
		ON_CBN_SELCHANGE(IDC_PS_FORMVIEW_COMBO_GRAYSCALE, OnSelendokComboGrayScale)
		ON_CBN_SELCHANGE(IDC_PS_FORMVIEW_COMBO_LINETYPE, OnSelendokComboLineType)
		ON_CBN_SELCHANGE(IDC_PS_FORMVIEW_COMBO_ADAPTIVE, OnSelendokComboAdaptive)
		ON_CBN_SELCHANGE(IDC_PS_FORMVIEW_COMBO_LINEWEIGHT, OnSelendokComboLineWeight)
		ON_CBN_SELCHANGE(IDC_PS_FORMVIEW_COMBO_LINEENDSTYLE, OnSelendokComboLineEndStyle)
		ON_CBN_SELCHANGE(IDC_PS_FORMVIEW_COMBO_LINEJOINSTYLE, OnSelendokComboLineJoinStyle)
		ON_CBN_SELCHANGE(IDC_PS_FORMVIEW_COMBO_FILLSTYLE, OnSelendokComboFillStyle)
		ON_WM_DESTROY()
END_MESSAGE_MAP()

void EoDlgPlotStyleEditor_FormViewPropertyPage::initAdaptiveComboBox() {
	m_Adaptive.AddString(L"On");
	m_Adaptive.AddString(L"Off");
	m_Adaptive.SelectString(-1, L"On");
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initGrayscaleComboBox() {
	m_Grayscale.AddString(L"On");
	m_Grayscale.AddString(L"Off");
	m_Grayscale.SelectString(-1, L"On");
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initDitherComboBox() {
	m_Dither.AddString(L"On");
	m_Dither.AddString(L"Off");
	m_Dither.SelectString(-1, L"On");
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initLinetypeComboBox() {
	for (auto& LineType : g_PlotStylesLineTypes) {
		m_Linetype.AddString(LineType);
	}
	m_Linetype.SetCurSel(31);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initLineweightComboBox() {
	m_Lineweight.AddString(L"Use object lineweight");
	const auto bInch {m_pPlotStyleTable->isDisplayCustomLineweightUnits()};
	const OdString sUnits = bInch ? L"''" : L" mm";
	for (unsigned i = 0; i < m_pPlotStyleTable->lineweightSize(); i++) {
		CString lineweight;
		lineweight.Format(L"%.4f%s", bInch ? MMTOINCH(m_pPlotStyleTable->getLineweightAt(i)) : m_pPlotStyleTable->getLineweightAt(i), static_cast<const wchar_t*>(sUnits));
		m_Lineweight.AddString(lineweight);
	}
	m_Lineweight.SetCurSel(0);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initLineendstyleComboBox() {
	for (auto& LineEndStyle : g_PlotStylesLineEndStyles) {
		m_Lineendstyle.AddString(LineEndStyle);
	}
	m_Lineendstyle.SetCurSel(0);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initLinejoinstyleComboBox() {
	for (auto& LineJoinStyles : g_PlotStylesLineJoinStyles) {
		m_Linejoinstyle.AddString(LineJoinStyles);
	}
	m_Linejoinstyle.SetCurSel(0);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initFillstyleComboBox() {
	for (auto& FillStyle : g_PlotStylesFillStyles) {
		m_Fillstyle.AddString(FillStyle);
	}
	m_Fillstyle.SetCurSel(0);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initColorComboBox() {
	int Item;
	for (unsigned BitmapIndex = 0; BitmapIndex < m_bitmapList.size(); BitmapIndex++) {
		if (!BitmapIndex) {
			Item = m_Color.AddBitmap(nullptr, m_bitmapList[BitmapIndex]->m_name);
		} else {
			Item = m_Color.AddBitmap(&m_bitmapList[BitmapIndex]->m_bitmap, m_bitmapList[BitmapIndex]->m_name);
		}
		m_bitmapList[BitmapIndex]->m_iItem = static_cast<unsigned char>(Item);
	}
	m_Color.SetCurSel(0);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnItemchangedListStyles(NMHDR* notifyStructure, LRESULT* result) {
	const NM_LISTVIEW* pNMListView = reinterpret_cast<NM_LISTVIEW*>(notifyStructure);
	if (!pNMListView->uNewState) {
		*result = 0;
		return;
	}
	m_bEditChanging = true;
	CPsListStyleData* pPsListStyleData = reinterpret_cast<CPsListStyleData*>(m_listStyles.GetItemData(pNMListView->iItem));
	m_pPlotStyleActive = pPsListStyleData->GetOdPsPlotStyle();
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	m_editDescription.SetWindowTextW(m_pPlotStyleActive->description());
	m_Dither.SelectString(-1, OdPsData.isDitherOn() ? L"On" : L"Off");
	m_Grayscale.SelectString(-1, OdPsData.isGrayScaleOn() ? L"On" : L"Off");
	m_spinPen.SetPos(OdPsData.physicalPenNumber());
	m_spinVirtpen.SetPos(OdPsData.virtualPenNumber());
	m_spinScreening.SetPos(OdPsData.screening());
	m_Adaptive.SelectString(-1, OdPsData.isAdaptiveLinetype() ? L"On" : L"Off");
	m_Linetype.SetCurSel(OdPsData.linetype());
	m_Lineweight.SetCurSel(static_cast<int>(static_cast<unsigned long>(OdPsData.lineweight())));
	m_Lineendstyle.SetCurSel(OdPsData.endStyle());
	m_Linejoinstyle.SetCurSel(OdPsData.joinStyle() < 5 ? OdPsData.joinStyle() : 4);
	m_Fillstyle.SetCurSel(OdPsData.fillStyle() - 64);
	deleteCustomColor();
	m_Color.SetCurSel(appendCustomColor(pNMListView->iItem));
	m_bEditChanging = false;
	if (!m_pPlotStyleTable->isAciTableAvailable()) {
		m_AddstyleButton.EnableWindow(TRUE);
		auto pChildWnd {GetWindow(GW_CHILD)};
		pChildWnd = pChildWnd->GetWindow(GW_HWNDFIRST);
		if (!pNMListView->iItem) {
			while (pChildWnd) {
				pChildWnd->EnableWindow(FALSE);
				pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
			}
			m_DelstyleButton.EnableWindow(FALSE);
			m_listStyles.EnableWindow(TRUE);
		} else {
			while (pChildWnd) {
				pChildWnd->EnableWindow(TRUE);
				pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
			}
		}
		m_LineweightButton.EnableWindow(TRUE);
		m_SaveButton.EnableWindow(TRUE);
		m_AddstyleButton.EnableWindow(TRUE);
	}
	*result = 0;
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnChangeEditScreening() {
	if (m_bEditChanging) return;
	m_bEditChanging = true;
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	CString pVal;
	m_editScreening.GetWindowText(pVal);
	int num;
	if (pVal == L"Automatic") { pVal = L"0"; }
	_stscanf(pVal, L"%d", &num);
	if (num < 0 || num > PS_SPIN_MAX_PEN) {
		num = 0;
		m_spinScreening.SetPos(num);
	}
	OdPsData.setPhysicalPenNumber(static_cast<short>(num));
	m_pPlotStyleActive->setData(OdPsData);
	if (!m_spinScreening.GetPos()) {
		m_editScreening.SetWindowTextW(L"Automatic");
	} else {
		CString buffer;
		buffer.Format(L"%d", num);
		m_editScreening.SetWindowTextW(buffer);
	}
	m_bEditChanging = false;
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnChangeEditPen() {
	if (m_bEditChanging) { return; }
	m_bEditChanging = true;
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	CString pVal;
	m_editPen.GetWindowText(pVal);
	int num;
	if (pVal == L"Automatic") { pVal = L"0"; }
	_stscanf(pVal, L"%d", &num);
	if (num < 0 || num > PS_SPIN_MAX_PEN) {
		num = 0;
		m_spinPen.SetPos(num);
	}
	OdPsData.setPhysicalPenNumber(static_cast<short>(num));
	m_pPlotStyleActive->setData(OdPsData);
	if (!m_spinPen.GetPos()) {
		m_editPen.SetWindowTextW(L"Automatic");
	} else {
		wchar_t buffer[256];
		_itot(num, buffer, 10);
		m_editPen.SetWindowTextW(buffer);
	}
	m_bEditChanging = false;
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnChangeEditVirtPen() {
	if (m_bEditChanging) return;
	m_bEditChanging = true;
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	CString pVal;
	m_editVirtpen.GetWindowText(pVal);
	int num;
	if (pVal == L"Automatic") { pVal = L"0"; }
	_stscanf(pVal, L"%d", &num);
	if (num < 0 || num > PS_SPIN_MAX_VIRTPEN) {
		num = 0;
		m_spinVirtpen.SetPos(num);
	}
	OdPsData.setVirtualPenNumber(static_cast<short>(num));
	m_pPlotStyleActive->setData(OdPsData);
	if (!m_spinVirtpen.GetPos()) {
		m_editVirtpen.SetWindowTextW(L"Automatic");
	} else {
		wchar_t buffer[256];
		_itot(num, buffer, 10);
		m_editVirtpen.SetWindowTextW(buffer);
	}
	m_bEditChanging = false;
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnChangeEditDescription() {
	if (m_bEditChanging) { return; }
	m_bEditChanging = true;
	const auto iItem {m_listStyles.GetSelectionMark()};
	if (iItem < 0) { return; }
	CPsListStyleData* pPsListStyleData = reinterpret_cast<CPsListStyleData*>(m_listStyles.GetItemData(iItem));
	auto PlotStyle {pPsListStyleData->GetOdPsPlotStyle()};
	CString pVal;
	m_editDescription.GetWindowText(pVal);
	PlotStyle->setDescription(OdString(pVal));
	m_bEditChanging = false;
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnUpdateEditDescription() noexcept {
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnDeltaposSpinPen(NMHDR* notifyStructure, LRESULT* result) noexcept {
	*result = 0;
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnItemchangingListStyles(NMHDR* notifyStructure, LRESULT* result) {
	const NM_LISTVIEW* pNMListView = reinterpret_cast<NM_LISTVIEW*>(notifyStructure);
	// TODO: Add your control notification handler code here
	if (!pNMListView->uNewState) {
		*result = 0;
		return;
	}
	const auto iLastItem {m_listStyles.GetSelectionMark()};
	if (iLastItem < 0) * result = 0;
	const auto iItem {m_listStyles.GetSelectionMark()};
	if (iItem < 0) {
		*result = 0;
		return;
	}
	CPsListStyleData* pPsListStyleData = reinterpret_cast<CPsListStyleData*>(m_listStyles.GetItemData(iItem));
	pPsListStyleData->SetActiveListIndex(m_Color.GetCurSel());
	*result = 0;
}

bool EoDlgPlotStyleEditor_FormViewPropertyPage::SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable) noexcept {
	if (!pPlotStyleTable) { return false; }
	m_pPlotStyleTable = pPlotStyleTable;
	return true;
}

HICON EoDlgPlotStyleEditor_FormViewPropertyPage::initColorIcon(const int width, const int height, const COLORREF color) noexcept {
	ICONINFO ii;
	ii.fIcon = TRUE;
	const auto hScreenDC {::GetDC(nullptr)};
	const auto hIconDC {CreateCompatibleDC(hScreenDC)};
	const auto hMaskDC {CreateCompatibleDC(hScreenDC)};
	ii.xHotspot = 0;
	ii.yHotspot = 0;
	ii.hbmColor = CreateCompatibleBitmap(hScreenDC, width, height);
	ii.hbmMask = CreateCompatibleBitmap(hMaskDC, width, height);
	::ReleaseDC(nullptr, hScreenDC);
	const auto hOldIconDC {SelectObject(hIconDC, ii.hbmColor)};
	const auto hOldMaskDC {SelectObject(hMaskDC, ii.hbmMask)};
	BitBlt(hIconDC, 0, 0, width, height, nullptr, 0, 0, WHITENESS);
	BitBlt(hMaskDC, 0, 0, width, height, nullptr, 0, 0, BLACKNESS);
	RECT r = {0, 0, width, height};
	const auto SolidBrush {CreateSolidBrush(color)};
	FillRect(hIconDC, &r, SolidBrush);
	DeleteObject(SolidBrush);
	SelectObject(hIconDC, hOldIconDC);
	SelectObject(hMaskDC, hOldMaskDC);
	const auto Icon {CreateIconIndirect(&ii)};

	//Cleanup
	DeleteObject(ii.hbmColor);
	DeleteObject(ii.hbmMask);
	DeleteDC(hMaskDC);
	DeleteDC(hIconDC);
	return Icon;
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initImageList() {
	m_imageList.Create(16, 16, ILC_COLORDDB/*ILC_COLOR32*/, 0, 0);
	const auto NumberOfPlotStyles = m_pPlotStyleTable->plotStyleSize();
	const auto LightPalette {odcmAcadLightPalette()};
	for (unsigned PlotStyleIndex = 0; PlotStyleIndex < NumberOfPlotStyles; PlotStyleIndex++) {
		m_imageList.Add(initColorIcon(16, 16, LightPalette[PlotStyleIndex + 1]));
	}
	if (m_pPlotStyleTable->isAciTableAvailable()) {
		m_listStyles.SetImageList(&m_imageList, LVSIL_SMALL);
	}
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initListCtrl() {
	LVCOLUMNW lvColumn;
	::ZeroMemory(&lvColumn, sizeof(LVCOLUMNW));
	lvColumn.mask = LVCF_FMT | LVCF_TEXT;
	lvColumn.fmt = LVCFMT_CENTER;
	m_listStyles.InsertColumn(1, &lvColumn);
	const auto NumberOfPlotStyles = static_cast<int>(m_pPlotStyleTable->plotStyleSize());
	for (auto PlotStyleIndex = 0; PlotStyleIndex < NumberOfPlotStyles; PlotStyleIndex++) {
		insertItem(PlotStyleIndex);
	}
}

const int EoDlgPlotStyleEditor_FormViewPropertyPage::insertItem(const int index) {
	m_listStyles.LockWindowUpdate(); // ***** lock window updates while filling list *****
	const auto PlotStyle {m_pPlotStyleTable->plotStyleAt(index).get()};
	LVITEMW lvItem;
	::ZeroMemory(&lvItem, sizeof(LVITEMW));
	lvItem.mask = static_cast<unsigned>(m_pPlotStyleTable->isAciTableAvailable() ? LVIF_TEXT | LVIF_IMAGE | LVIF_STATE : LVIF_TEXT | LVIF_STATE);
	lvItem.state = 0;
	lvItem.stateMask = 0;
	if (m_pPlotStyleTable->isAciTableAvailable()) { lvItem.iImage = index; }
	lvItem.iItem = index;
	lvItem.iSubItem = 0;
	const auto str = PlotStyle->localizedName();
	lvItem.pszText = const_cast<wchar_t*>(static_cast<const wchar_t*>(str));
	const auto nItem {m_listStyles.InsertItem(&lvItem)};
	const auto pPsListStyleData {new CPsListStyleData(PlotStyle, &m_bitmapList, static_cast<char>(nItem))};
	m_listStyles.SetItemData(nItem, reinterpret_cast<LPARAM>(pPsListStyleData));
	m_listStyles.UnlockWindowUpdate();
	return nItem;
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::initBitmapList() {
	CBitmapColorInfo* pBitmapColorInfo = new CBitmapColorInfo(MAKEINTRESOURCEW(IDB_SELECT_TRUE_COLOR), L"Select true color...");
	const CBitmap* bitmapSrc = &pBitmapColorInfo->m_bitmap;
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(255, 255, 255), L"Use object color"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(255, 0, 0), L"Red"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(255, 255, 0), L"Yellow"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(0, 255, 0), L"Green"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(0, 255, 255), L"Cyan"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(0, 0, 255), L"Blue"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(255, 0, 255), L"Magenta"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(0, 0, 0), L"Black"));
	m_bitmapList.push_back(pBitmapColorInfo);
}

BOOL EoDlgPlotStyleEditor_FormViewPropertyPage::OnInitDialog() {
	CPropertyPage::OnInitDialog();
	if (!m_pPlotStyleTable) { return FALSE; }
	initBitmapList();
	initImageList();
	initListCtrl();
	initGrayscaleComboBox();
	initDitherComboBox();
	initAdaptiveComboBox();
	initLinetypeComboBox();
	initLineweightComboBox();
	initLineendstyleComboBox();
	initLinejoinstyleComboBox();
	initFillstyleComboBox();
	initColorComboBox();
	SetWindowLong(m_listStyles.m_hWnd, GWL_STYLE, WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_SMALLICON | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_AUTOARRANGE);
	ListView_SetItemState(m_listStyles.m_hWnd, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
	if (m_pPlotStyleTable->isAciTableAvailable()) {
		m_AddstyleButton.EnableWindow(FALSE);
		m_DelstyleButton.EnableWindow(FALSE);
	} else {
		m_AddstyleButton.EnableWindow(TRUE);
		m_DelstyleButton.EnableWindow(FALSE);
	}
	return TRUE;	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelchangeComboColor() {
	// TODO: Add your control notification handler code here
	short intColorPolicy;
	const auto CurrentSelection {m_Color.GetCurSel()};
	const auto ListStylesItem {m_listStyles.GetSelectionMark()};
	auto pPsListStyleData {reinterpret_cast<CPsListStyleData*>(m_listStyles.GetItemData(ListStylesItem))};
	if (CurrentSelection == m_Color.GetCount() - 1) {
		CColorDialog dlgColor;
		if (dlgColor.DoModal() == IDOK) {
			deleteCustomColor();
			const auto color {dlgColor.GetColor()};
			m_Color.SetCurSel(replaceCustomColor(color, ListStylesItem));
			intColorPolicy = 3;
		}
	} else {
		pPsListStyleData->SetActiveListIndex(CurrentSelection);
		if (CurrentSelection) intColorPolicy = 5;
	}
	const auto color {pPsListStyleData->GetColor()};
	// m_pPlotStyleActive->setColorPolicy(intColorPolicy);
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setColor(color);
	m_pPlotStyleActive->setData(OdPsData);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboColor() noexcept {
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboDither() {
	const auto CurrentSelection {m_Dither.GetCurSel()};
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setDitherOn(CurrentSelection == 0 ? true : false);
	m_pPlotStyleActive->setData(OdPsData);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboGrayScale() {
	const auto CurrentSelection {m_Grayscale.GetCurSel()};
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setGrayScaleOn(CurrentSelection == 0 ? true : false);
	m_pPlotStyleActive->setData(OdPsData);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboAdaptive() {
	const auto CurrentSelection {m_Adaptive.GetCurSel()};
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setAdaptiveLinetype(CurrentSelection == 0 ? true : false);
	m_pPlotStyleActive->setData(OdPsData);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboLineWeight() {
	const auto CurrentSelection {m_Lineweight.GetCurSel()};
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setLineweight(CurrentSelection);
	m_pPlotStyleActive->setData(OdPsData);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboLineEndStyle() {
	const auto CurrentSelection {m_Lineendstyle.GetCurSel()};
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setEndStyle(OdPs::LineEndStyle(CurrentSelection));
	m_pPlotStyleActive->setData(OdPsData);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboFillStyle() {
	const auto CurrentSelection {m_Fillstyle.GetCurSel()};
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setFillStyle(OdPs::FillStyle(CurrentSelection + 64));
	m_pPlotStyleActive->setData(OdPsData);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboLineJoinStyle() {
	const auto CurrentSelection {m_Linejoinstyle.GetCurSel()};
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setJoinStyle(OdPs::LineJoinStyle(CurrentSelection));
	m_pPlotStyleActive->setData(OdPsData);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboLineType() {
	const auto CurrentSelection {m_Linetype.GetCurSel()};
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setLinetype(OdPs::LineType(CurrentSelection));
	m_pPlotStyleActive->setData(OdPsData);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnDelBtnStyle() {
	// TODO: Add your control notification handler code here
	const auto Item {m_listStyles.GetSelectionMark()};
	m_pPlotStyleActive = m_pPlotStyleTable->delPlotStyle(m_pPlotStyleActive);
	CPsListStyleData* pPsListStyleData = reinterpret_cast<CPsListStyleData*>(m_listStyles.GetItemData(Item));
	m_listStyles.DeleteItem(Item);
	delete pPsListStyleData;
	m_listStyles.SetItemState(Item - 1, LVIS_SELECTED, LVIS_SELECTED);
	m_listStyles.SetSelectionMark(Item - 1);
	m_listStyles.SetFocus();
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::AddNewPlotStyle(const wchar_t* styleName) {
	m_pPlotStyleActive = m_pPlotStyleTable->addNewPlotStyle(styleName);
	const auto ItemIndex {static_cast<int>(m_pPlotStyleTable->plotStyleSize()) - 1};
	insertItem(ItemIndex);
	m_listStyles.SetItemState(ItemIndex, LVIS_SELECTED, LVIS_SELECTED);
	m_listStyles.SetSelectionMark(ItemIndex);
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnAddBtnStyle() {
	DialogBox(AfxGetInstanceHandle(), MAKEINTRESOURCEW(IDD_PS_DLG_ADDPS), m_hWnd, Dlg_Proc);
	m_listStyles.SetFocus();
}

BOOL EoDlgPlotStyleEditor_FormViewPropertyPage::DoPromptFileName(CString& fileName, unsigned nIDSTitle, unsigned long flags) {
	auto ext {fileName.Right(3)};
	const auto isCtb {m_pPlotStyleTable->isAciTableAvailable()};
	CFileDialog FileDialog(FALSE);
	CString title {L"Save As"};
	FileDialog.m_ofn.Flags |= flags;
	CString strFilter;
	CString strDefault;
	strFilter = isCtb ? L"Color-Dependent Style Table Files (*.ctb)" : L"Style Table Files (*.stb)";
	strFilter += static_cast<wchar_t>('\0'); // next string please
	strFilter += isCtb ? L"*.ctb" : L"*.stb";
	strFilter += static_cast<wchar_t>('\0'); // last string
	FileDialog.m_ofn.nMaxCustFilter++;
	FileDialog.m_ofn.nFilterIndex = 1;
	if (fileName.ReverseFind('.') != -1) { fileName = fileName.Left(fileName.ReverseFind('.')); }
	FileDialog.m_ofn.lpstrFilter = strFilter;
	FileDialog.m_ofn.lpstrTitle = title;
	FileDialog.m_ofn.lpstrFile = fileName.GetBuffer(MAX_PATH);
	const auto nResult {FileDialog.DoModal()};
	fileName.ReleaseBuffer();
	if (nResult == IDOK) {
		fileName = FileDialog.GetPathName();
		if (fileName.ReverseFind('.') == -1) { fileName += isCtb ? L".ctb" : L".stb"; }
	}
	return nResult == IDOK;
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::SetFileBufPath(OdString filePath) {
	m_sFileBufPath = filePath;
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSaveBtn() {
	CString sPath = static_cast<const wchar_t*>(m_sFileBufPath);
	if (!DoPromptFileName(sPath, AFX_IDS_SAVEFILE, OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST)) { return; } // don't even attempt to save
	OdStreamBufPtr pFileBuf;
	auto SystemServices {odSystemServices()};
	try {
		pFileBuf = SystemServices->createFile(static_cast<const wchar_t*>(sPath), Oda::kFileWrite, Oda::kShareDenyWrite, Oda::kOpenAlways /*Oda::kCreateAlways*/);
		if (pFileBuf.get()) {
			OdPsPlotStyleServicesPtr pPSS = odrxDynamicLinker()->loadApp(ODPS_PLOTSTYLE_SERVICES_APPNAME);
			if (pPSS.get()) {
				pPSS->savePlotStyleTable(pFileBuf, m_pPlotStyleTable);
			}
		}
	} catch (...) {
	}
}

void EoDlgPlotStyleEditor_FormViewPropertyPage::OnLineweightBtn() {
	Expects(m_pPlotStyleActive);
	EoDlgPlotStyleEditLineweight PsEditLineweightDlg;
	PsEditLineweightDlg.SetPlotStyleTable(m_pPlotStyleTable);
	OdPsPlotStyleData OdPsData;
	auto idx {m_Lineweight.GetCurSel()};
	if (idx == CB_ERR) {
		m_pPlotStyleActive->getData(OdPsData);
		idx = static_cast<int>(OdPsData.lineweight());
	}
	PsEditLineweightDlg.SetInitialSelection(m_Lineweight.GetCurSel());
	if (PsEditLineweightDlg.DoModal() == IDOK) {
		m_Lineweight.ResetContent();
		initLineweightComboBox();
		m_pPlotStyleActive->getData(OdPsData);
		m_Lineweight.SetCurSel(static_cast<int>(OdPsData.lineweight()));
	}
}

int EoDlgPlotStyleEditor_FormViewPropertyPage::deleteCustomColor() {
	if (m_Color.GetCount() > PS_COMBO_COLOR_POSITION + 1) { m_Color.DeleteString(PS_COMBO_COLOR_POSITION); }
	return 0;
}

int EoDlgPlotStyleEditor_FormViewPropertyPage::appendCustomColor(const int item) {
	const auto pPsListStyleData {reinterpret_cast<CPsListStyleData*>(m_listStyles.GetItemData(item))};
	const auto pBitmapColorInfo {pPsListStyleData->GetBitmapColorInfo()};
	if (!pBitmapColorInfo) { return pPsListStyleData->GetActiveListIndex(); }
	return m_Color.InsertBitmap(PS_COMBO_COLOR_POSITION, &pBitmapColorInfo->m_bitmap, pBitmapColorInfo->m_name);
}

int EoDlgPlotStyleEditor_FormViewPropertyPage::replaceCustomColor(const COLORREF color, const int item) {
	auto pPsListStyleData {reinterpret_cast<CPsListStyleData*>(m_listStyles.GetItemData(item))};
	pPsListStyleData->ReplaceBitmapColorInfo(color, item);
	const auto pBitmapColorInfo {pPsListStyleData->GetBitmapColorInfo()};
	if (!pBitmapColorInfo) { return pPsListStyleData->GetActiveListIndex(); }
	return m_Color.InsertBitmap(PS_COMBO_COLOR_POSITION, &pBitmapColorInfo->m_bitmap, pBitmapColorInfo->m_name);
}
