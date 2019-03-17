#include "stdafx.h"

#include "EoDlgPlotStyleEditLineweight.h"
#include "EoDlgPlotStyleTableEditor_FormViewPropertyPage.h"
#include "Ps/PlotStyles.h"
#include "ColorMapping.h"
#include "DynamicLinker.h"

#include "WindowsX.h"

void Dlg_OnClose(HWND hwnd) {
	DestroyWindow(hwnd);
}
void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
	switch(id)
	{
	case IDOK:
		{
			const int nMaxCount = 100;
			wchar_t sString[nMaxCount];
			HWND hOwner = ::GetWindow(hwnd, GW_OWNER);
			::GetWindowText(::GetDlgItem(hwnd, IDC_PS_ADDPS_EDIT_PSNAME), sString, nMaxCount);
			CPropertySheet *pPsDlg = (CPropertySheet *)CWnd::FromHandle(hOwner);
			EoDlgPlotStyleEditor_FormViewPropertyPage *pPg = (EoDlgPlotStyleEditor_FormViewPropertyPage*)pPsDlg->GetActivePage();
			pPg->AddNewPlotStyle(sString);
		}
	case IDCANCEL:
		EndDialog(hwnd, id);
		break;
	case IDC_PS_ADDPS_EDIT_PSNAME:
		{
			if (codeNotify == EN_CHANGE) {
				const int nMaxCount = 100;
				wchar_t sString[nMaxCount];
				HWND hOwner = ::GetWindow(hwnd, GW_OWNER);
				::GetWindowText(::GetDlgItem(hwnd, IDC_PS_ADDPS_EDIT_PSNAME), sString, nMaxCount);
				CPropertySheet *pPsDlg = (CPropertySheet *)CWnd::FromHandle(hOwner);
				EoDlgPlotStyleEditor_FormViewPropertyPage *pPg = (EoDlgPlotStyleEditor_FormViewPropertyPage*)pPsDlg->GetActivePage();
				const OdPsPlotStyleTable *pPsTab = pPg->GetPlotStyleTable();
				OdPsPlotStylePtr pPs;
				HWND hDInfo = ::GetDlgItem(hwnd, IDC_PS_ADDPS_STATIC_DINFO);
				HWND hSInfo = ::GetDlgItem(hwnd, IDC_PS_ADDPS_STATIC_SINFO);
				HWND hOkBtn = ::GetDlgItem(hwnd, IDOK);
				CString newName = sString;
				newName.MakeLower();
				if (newName.IsEmpty()) {
					EnableWindow(hOkBtn, FALSE);
					return;
				}
				for (size_t PlotStyleIndex = 0; PlotStyleIndex < pPsTab->plotStyleSize(); PlotStyleIndex++) {
					pPs = pPsTab->plotStyleAt(PlotStyleIndex);
					CString name = (LPCWSTR)pPs->localizedName();
					name.MakeLower();
					if (name == newName) {
						CString sInfo;
						sInfo.Format(L"A style named <%s> already exists.", sString);
						::SetWindowText(hDInfo, sInfo);
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
BOOL Dlg_OnInit(HWND hwnd, HWND hwndCtl, LPARAM lParam) {
	HWND hOwner = ::GetWindow(hwnd, GW_OWNER);
	CPropertySheet *pPsDlg = (CPropertySheet *) CWnd::FromHandle(hOwner);
	EoDlgPlotStyleEditor_FormViewPropertyPage *pPg = (EoDlgPlotStyleEditor_FormViewPropertyPage*) pPsDlg->GetActivePage();
	const OdPsPlotStyleTable* PlotStyleTable = pPg->GetPlotStyleTable();
	OdString sName;
	sName.format(L"Style %d", PlotStyleTable->plotStyleSize());
	::SetWindowText(::GetDlgItem(hwnd, IDC_PS_ADDPS_EDIT_PSNAME), sName);
	return TRUE;
}
int WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CLOSE: 
		return SetDlgMsgResult(hwnd, uMsg, HANDLE_WM_CLOSE(hwnd, wParam, lParam, Dlg_OnClose));
	case WM_COMMAND: 
		return SetDlgMsgResult(hwnd, uMsg, HANDLE_WM_COMMAND(hwnd, wParam, lParam, Dlg_OnCommand));
	case WM_INITDIALOG: 
		return SetDlgMsgResult(hwnd, uMsg, HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Dlg_OnInit));
	}
	return (FALSE);
}
void CBitmapColorInfo::GetBitmapSizes(CBitmap &Bmp, int &W, int &H) {
	BITMAP bi;
	Bmp.GetBitmap(&bi);
	W = bi.bmWidth;
	H = bi.bmHeight;
}
DIBCOLOR *CBitmapColorInfo::GetBitmapPixels(CBitmap &Bmp, int &W, int &H) {
	CDC dcMem; 
	dcMem.CreateCompatibleDC(NULL);
	GetBitmapSizes(Bmp, W, H);
	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = W;
	bi.bmiHeader.biHeight = -H;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = H*W*4;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;
	void *buf = new char[H*W*4];
	GetDIBits(dcMem.m_hDC, (HBITMAP)Bmp.m_hObject, 0, H, buf, &bi, DIB_RGB_COLORS);
	return (DIBCOLOR*) buf;
}
void CBitmapColorInfo::SetBitmapPixels(CBitmap &Bmp, DIBCOLOR *pPixels) {
	CDC dcMem; 
	dcMem.CreateCompatibleDC(NULL);
	int W, H;
	GetBitmapSizes(Bmp, W, H);
	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = W;
	bi.bmiHeader.biHeight = -H;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = H*W*4;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;
	SetDIBits(dcMem.m_hDC, (HBITMAP) Bmp.m_hObject, 0, H, pPixels, &bi, DIB_RGB_COLORS); 
	delete pPixels;
}
CBitmap* CBitmapColorInfo::CloneBitmap(const CBitmap* pBmpSource, CBitmap* pBmpClone) {
	ASSERT(pBmpClone);
	ASSERT(pBmpSource);
	ASSERT(pBmpSource != pBmpClone);
	if (!pBmpClone && !pBmpSource && (pBmpSource == pBmpClone)) return NULL;

	BITMAP bmp; 
	DWORD dw;
	EoByte *pb;
	((CBitmap*)pBmpSource)->GetBitmap(&bmp); 

	CClientDC dc(NULL);
	CDC cdc;
	cdc.CreateCompatibleDC(&dc);
	pBmpClone->CreateCompatibleBitmap(&dc, bmp.bmWidth, bmp.bmHeight);

	dw = bmp.bmWidthBytes*bmp.bmHeight;
	pb = new EoByte[dw];
	dw = pBmpSource->GetBitmapBits(dw, pb); 
	pBmpClone->SetBitmapBits(dw, pb);
	delete[]pb;

	int W, H; 
	DIBCOLOR *buf = GetBitmapPixels(*(CBitmap*)pBmpSource, W, H);
	SetBitmapPixels(*pBmpClone, buf);

	return pBmpClone;
}
void CBitmapColorInfo::PaintBitmap(CBitmap &Bmp, COLORREF color) {
	int W, H; 
	DIBCOLOR *buf = GetBitmapPixels(Bmp, W, H);

	DIBCOLOR *pColor = buf;
	for (int y = H-1; y >= 0; y--)
		for (int x = 0; x < W; x++, pColor++)
			*pColor = DIBCOLOR(color);

	SetBitmapPixels(Bmp, buf);
}
const OdCmEntityColor CBitmapColorInfo::GetColor() {

	OdCmEntityColor color = OdCmEntityColor((OdUInt8)((m_color >> 16) & 0xFF), 
		(OdUInt8)((m_color >> 8) & 0xFF), (OdUInt8)(m_color & 0xFF));
	return color;
}
const bool CBitmapColorInfo::IsColor(COLORREF color, EoByte item) {
	color = (item << 24) + (GetRValue(color) << 16) + (GetGValue(color) << 8) + (GetBValue(color));
	return m_color == color;
}
CBitmapColorInfo::CBitmapColorInfo(const CBitmap *pBitmap, COLORREF color, EoByte cColorItem, int colorIndex) :
	m_iItem(cColorItem) {

		m_color = (m_iItem << 24) + (GetRValue(color) << 16) + (GetGValue(color) << 8) + (GetBValue(color));
		CloneBitmap(pBitmap, &m_bitmap);
		PaintBitmap(m_bitmap, color);
		if (colorIndex <= 0)
		{
			wcscpy(m_name, L"Custom Color");
		}
		else
		{
			OdString clrName;
			clrName.format(L"Color %d", colorIndex);
			wcscpy(m_name, (LPCWSTR) clrName);
		}
}
CBitmapColorInfo::CBitmapColorInfo(const CBitmap *pBitmap, COLORREF color, const wchar_t* name) :
	m_iItem(0xff) {
		m_color = (m_iItem << 24) + (GetRValue(color) << 16) + (GetGValue(color) << 8) + (GetBValue(color));
		CloneBitmap(pBitmap, &m_bitmap);
		PaintBitmap(m_bitmap, color);
		_tcsncpy(m_name, name, PS_COLOR_MAX_NAME);
}
CBitmapColorInfo::CBitmapColorInfo(LPCWSTR lpszResourceName, const wchar_t* name) :
	m_iItem(0xff) {
		HBITMAP hBmp;
		hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), 
			lpszResourceName, IMAGE_BITMAP, 13,13, LR_CREATEDIBSECTION ); 
		CBitmap *bitmap = CBitmap::FromHandle(hBmp);
		CloneBitmap(bitmap, &m_bitmap);
		_tcsncpy(m_name, name, PS_COLOR_MAX_NAME);
}
const int CPsListStyleData::getPublicArrayIndexByColor(COLORREF color) {

	for (UINT i = 0; i < m_pPublicBitmapList->size(); i++)
	{
		OdCmEntityColor cl = OdCmEntityColor(GetRValue(color), GetGValue(color), GetBValue(color));
		if ((*m_pPublicBitmapList)[i]->GetColor() == cl)
		{
			return i;
		}
	}
	return -1;

}
CPsListStyleData::CPsListStyleData(OdPsPlotStyle* pPs, OdBitmapColorInfoArray* pPublicBitmapList, const char item) : 
	m_pPlotStyles(pPs), m_pPublicBitmapList(pPublicBitmapList), m_pBitmapColorInfo(NULL) {
	if (!m_pPlotStyles && !m_pPublicBitmapList) return;
	OdPsPlotStyleData OdPsData;
	pPs->getData(OdPsData);
	OdCmEntityColor cL = OdPsData.color();

	OdUInt32 rgb;
	if (cL.isByACI())
		rgb = odcmLookupRGB(cL.colorIndex(), odcmAcadLightPalette());
	else
		rgb = RGB(cL.red(), cL.green(), cL.blue());
	m_iActiveListIndex = getPublicArrayIndexByColor(rgb);

	if (m_iActiveListIndex < 0) 
		m_pBitmapColorInfo = new CBitmapColorInfo(&(*m_pPublicBitmapList)[m_pPublicBitmapList->size()-1]->m_bitmap, rgb, item, cL.isByACI() ? cL.colorIndex() : -1); 
}
CPsListStyleData::~CPsListStyleData() {
	delete m_pBitmapColorInfo;
	m_pBitmapColorInfo = 0;
}
const bool CPsListStyleData::SetActiveListIndex(const int index, const bool bBmpInfo) {
	if (!m_pPlotStyles && !m_pPublicBitmapList) return false;
	if ((UINT)index >= m_pPublicBitmapList->size()-1) return false;
	if (index < 0) return false;
	m_iActiveListIndex = index;
	if (bBmpInfo) return true;
	delete m_pBitmapColorInfo;
	m_pBitmapColorInfo = 0;
	return true;
}
const bool CPsListStyleData::ReplaceBitmapColorInfo(COLORREF color, const int item) {
	if (!m_pPlotStyles && !m_pPublicBitmapList) return false;

	delete m_pBitmapColorInfo;
	m_pBitmapColorInfo = 0;

	m_iActiveListIndex = getPublicArrayIndexByColor(color);

	if (m_iActiveListIndex < 0) 
		m_pBitmapColorInfo = new CBitmapColorInfo(&(*m_pPublicBitmapList)[m_pPublicBitmapList->size() - 1]->m_bitmap, color, (EoByte)item);

	return true;

}
const OdCmEntityColor CPsListStyleData::GetColor() {
	if (m_iActiveListIndex < 0)
		return m_pBitmapColorInfo->GetColor();

	return (*m_pPublicBitmapList)[m_iActiveListIndex]->GetColor();

}

IMPLEMENT_DYNCREATE(EoDlgPlotStyleEditor_FormViewPropertyPage, CPropertyPage)

	EoDlgPlotStyleEditor_FormViewPropertyPage::EoDlgPlotStyleEditor_FormViewPropertyPage() : CPropertyPage(EoDlgPlotStyleEditor_FormViewPropertyPage::IDD) {
		m_pPlotStyleTable = 0;
		m_pPlotStyleActive = 0;
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
	int f;
	for (f = 0; f < m_listStyles.GetItemCount(); ++f) {
		CPsListStyleData* pPsListStyleData = (CPsListStyleData*)(m_listStyles.GetItemData(f));
		delete pPsListStyleData;
	}
	for (f = 0; f < (int) m_bitmapList.size(); ++f) {
		delete m_bitmapList[f];
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
	ON_CBN_SELENDOK (IDC_PS_FORMVIEW_COMBO_COLOR, OnSelendokComboColor)
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
	m_Adaptive.SelectString(-1,L"On");
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initGrayscaleComboBox() {
	m_Grayscale.AddString(L"On");
	m_Grayscale.AddString(L"Off");
	m_Grayscale.SelectString(-1,L"On");
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initDitherComboBox() {
	m_Dither.AddString(L"On");
	m_Dither.AddString(L"Off");
	m_Dither.SelectString(-1,L"On");
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initLinetypeComboBox() {

	for (int i = 0; i < 32; i++)
	{
		m_Linetype.AddString(StringLineType[i]);
	}
	m_Linetype.SetCurSel(31);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initLineweightComboBox() {
	m_Lineweight.AddString(L"Use object lineweight");
	const bool bInch = m_pPlotStyleTable->isDisplayCustomLineweightUnits();
	CString sUnits = bInch ? L"''" : L" mm";
	for (size_t i = 0; i < m_pPlotStyleTable->lineweightSize(); i++)
	{
		CString lineweight;
#pragma warning(suppress: 6284)
		lineweight.Format(L"%.4f%s", bInch ? MMTOINCH(m_pPlotStyleTable->getLineweightAt(i)) : m_pPlotStyleTable->getLineweightAt(i), sUnits);
		m_Lineweight.AddString(lineweight);
	}
	m_Lineweight.SetCurSel(0);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initLineendstyleComboBox() {
	for (int i = 0; i < 5; i++)
	{
		m_Lineendstyle.AddString(StringLineEndStyle[i]);
	}
	m_Lineendstyle.SetCurSel(0);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initLinejoinstyleComboBox() {
	for (int i = 0; i < 5; i++)
	{
		m_Linejoinstyle.AddString(StringLineJoinStyle[i]);
	}
	m_Linejoinstyle.SetCurSel(0);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initFillstyleComboBox() {
	for (int i = 0; i < 10; i++)
	{
		m_Fillstyle.AddString(StringFillStyle[i]);
	}
	m_Fillstyle.SetCurSel(0);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initColorComboBox() {
	int item = -1;

	for (size_t i = 0; i < m_bitmapList.size(); i++)
	{
		if (!i )
			item = m_Color.AddBitmap(NULL, m_bitmapList[i]->m_name);
		else
			item = m_Color.AddBitmap(&m_bitmapList[i]->m_bitmap, m_bitmapList[i]->m_name);
		m_bitmapList[i]->m_iItem = (EoByte)item;
	}
	m_Color.SetCurSel(0);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnItemchangedListStyles(NMHDR* pNMHDR, LRESULT* result) {
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (!pNMListView->uNewState) {
		*result = 0;
		return;
	}
	m_bEditChanging = true;

	CPsListStyleData *pPsListStyleData = (CPsListStyleData*)(m_listStyles.GetItemData(pNMListView->iItem));

	m_pPlotStyleActive = pPsListStyleData->GetOdPsPlotStyle();

	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);

	m_editDescription.SetWindowText(m_pPlotStyleActive->description());
	m_Dither.SelectString(-1, OdPsData.isDitherOn() ? L"On" : L"Off");
	m_Grayscale.SelectString(-1, OdPsData.isGrayScaleOn() ? L"On" : L"Off");
	m_spinPen.SetPos(OdPsData.physicalPenNumber());
	m_spinVirtpen.SetPos(OdPsData.virtualPenNumber());
	m_spinScreening.SetPos(OdPsData.screening());
	m_Adaptive.SelectString(-1,OdPsData.isAdaptiveLinetype() ? L"On" : L"Off");
	m_Linetype.SetCurSel(OdPsData.linetype());
	m_Lineweight.SetCurSel((OdUInt32)OdPsData.lineweight());
	m_Lineendstyle.SetCurSel(OdPsData.endStyle());
	m_Linejoinstyle.SetCurSel(OdPsData.joinStyle() < 5 ? OdPsData.joinStyle() : 4);
	m_Fillstyle.SetCurSel(OdPsData.fillStyle() - 64);

	deleteCustomColor();
	m_Color.SetCurSel(appendCustomColor(pNMListView->iItem));

	m_bEditChanging = false;

	if (!m_pPlotStyleTable->isAciTableAvailable()) {
		m_AddstyleButton.EnableWindow(TRUE);
		CWnd *pChildWnd = GetWindow(GW_CHILD);
		pChildWnd = pChildWnd->GetWindow(GW_HWNDFIRST);
		if (!pNMListView->iItem) {
			while (pChildWnd) {
				pChildWnd->EnableWindow(FALSE);
				pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
			}
			m_DelstyleButton.EnableWindow(FALSE);
			m_listStyles.EnableWindow(TRUE);
		}
		else {
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
	if (pVal == L"Automatic")
		pVal = L"0";
#pragma warning(suppress: 6031)
	_stscanf(pVal, L"%d", &num);
	if (num < 0 || num > PS_SPIN_MAX_PEN) {
		num = 0;
		m_spinScreening.SetPos(num);
	}
	OdPsData.setPhysicalPenNumber((OdInt16)num);
	m_pPlotStyleActive->setData(OdPsData);

	if (!m_spinScreening.GetPos()) {
		m_editScreening.SetWindowText(L"Automatic");
	}
	else {
		CString buffer;
		buffer.Format(L"%d", num);
		m_editScreening.SetWindowText(buffer);
	}
	m_bEditChanging = false;
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnChangeEditPen() {
	if (m_bEditChanging) return;
	m_bEditChanging = true;

	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	CString pVal;
	m_editPen.GetWindowText(pVal);
	int num;
	if (pVal == L"Automatic")
		pVal = L"0";
#pragma warning(suppress: 6031)
	_stscanf(pVal, L"%d", &num);
	if (num < 0 || num > PS_SPIN_MAX_PEN) {
		num = 0;
		m_spinPen.SetPos(num);
	}
	OdPsData.setPhysicalPenNumber((OdInt16)num);
	m_pPlotStyleActive->setData(OdPsData);

	if (!m_spinPen.GetPos()) {
		m_editPen.SetWindowText(L"Automatic");
	}
	else {
		wchar_t buffer[256];
		_itot(num, buffer, 10);
		m_editPen.SetWindowText(buffer);
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
	if (pVal == L"Automatic")
		pVal = L"0";
#pragma warning(suppress: 6031)
	_stscanf(pVal, L"%d", &num);
	if (num < 0 || num > PS_SPIN_MAX_VIRTPEN) {
		num = 0;
		m_spinVirtpen.SetPos(num);
	}
	OdPsData.setVirtualPenNumber((OdInt16)num);
	m_pPlotStyleActive->setData(OdPsData);

	if (!m_spinVirtpen.GetPos()) {
		m_editVirtpen.SetWindowText(L"Automatic");
	}
	else {
		wchar_t buffer[256];
		_itot(num, buffer, 10);
		m_editVirtpen.SetWindowText(buffer);
	}
	m_bEditChanging = false;
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnChangeEditDescription() {
	if (m_bEditChanging) return;
	m_bEditChanging = true;

	int iItem = m_listStyles.GetSelectionMark();
	if (iItem < 0) {
		return;
	}
	CPsListStyleData *pPsListStyleData = (CPsListStyleData*)(m_listStyles.GetItemData(iItem));
	OdPsPlotStyle* pPs = pPsListStyleData->GetOdPsPlotStyle();
	CString pVal;
	m_editDescription.GetWindowText(pVal);
	pPs->setDescription(OdString(pVal));

	m_bEditChanging = false;
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnUpdateEditDescription() {
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnDeltaposSpinPen(NMHDR* pNMHDR, LRESULT* result) {
	*result = 0;
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnItemchangingListStyles(NMHDR* pNMHDR, LRESULT* result) {
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if (!pNMListView->uNewState) {
		*result = 0;
		return;
	}
	int iLastItem = m_listStyles.GetSelectionMark();
	if (iLastItem < 0) *result = 0;

	int iItem = m_listStyles.GetSelectionMark();
	if (iItem < 0) {
		*result = 0;
		return;
	}
	CPsListStyleData *pPsListStyleData = (CPsListStyleData*)(m_listStyles.GetItemData(iItem));
	pPsListStyleData->SetActiveListIndex(m_Color.GetCurSel());

	*result = 0;
}
const bool EoDlgPlotStyleEditor_FormViewPropertyPage::SetPlotStyleTable(OdPsPlotStyleTable* pPlotStyleTable) {
	if (!pPlotStyleTable) return false;
	m_pPlotStyleTable = pPlotStyleTable;
	return true;
}
HICON EoDlgPlotStyleEditor_FormViewPropertyPage::initColorIcon(int width,int height, COLORREF color) {
	ICONINFO ii;
	ii.fIcon=TRUE;
	HDC hScreenDC=::GetDC(NULL);
	HDC hIconDC=CreateCompatibleDC(hScreenDC);
	HDC hMaskDC=CreateCompatibleDC(hScreenDC);

	ii.xHotspot=0;
	ii.yHotspot=0;	
	ii.hbmColor=CreateCompatibleBitmap(hScreenDC,width,height);	
	ii.hbmMask=CreateCompatibleBitmap(hMaskDC,width,height);

	::ReleaseDC(NULL,hScreenDC);

	HGDIOBJ hOldIconDC=::SelectObject(hIconDC,ii.hbmColor);
	HGDIOBJ hOldMaskDC=::SelectObject(hMaskDC,ii.hbmMask);	

	BitBlt(hIconDC,0,0,width,height,NULL,0,0,WHITENESS);
	BitBlt(hMaskDC,0,0,width,height,NULL,0,0,BLACKNESS);	

	RECT r={0,0,width,height};	
	HBRUSH hBR=CreateSolidBrush(color);
	FillRect(hIconDC,&r,hBR);
	DeleteObject(hBR);

	SelectObject(hIconDC,hOldIconDC);
	SelectObject(hMaskDC,hOldMaskDC);

	HICON hIcon=CreateIconIndirect(&ii);

	//Cleanup
	DeleteObject(ii.hbmColor);
	DeleteObject(ii.hbmMask);
	DeleteDC(hMaskDC);
	DeleteDC(hIconDC);

	return hIcon;
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initImageList() {
	m_imageList.Create(16, 16, ILC_COLORDDB/*ILC_COLOR32*/, 0, 0);

	size_t NumberOfPlotStyles = m_pPlotStyleTable->plotStyleSize();
	const ODCOLORREF* LightPalette = odcmAcadLightPalette();
	for (size_t PlotStyleIndex = 0; PlotStyleIndex < NumberOfPlotStyles; PlotStyleIndex++ ) {
		m_imageList.Add(initColorIcon(16, 16, LightPalette[PlotStyleIndex + 1])); 
	}
	if (m_pPlotStyleTable->isAciTableAvailable()) {
		m_listStyles.SetImageList(&m_imageList, LVSIL_SMALL);
	}
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initListCtrl() {
	LV_COLUMN lvColumn;
	::ZeroMemory(&lvColumn, sizeof(LV_COLUMN));
	lvColumn.mask = LVCF_FMT | LVCF_TEXT;
	lvColumn.fmt = LVCFMT_CENTER;
	m_listStyles.InsertColumn(1, &lvColumn);

	size_t NumberOfPlotStyles = m_pPlotStyleTable->plotStyleSize();
	for (size_t PlotStyleIndex = 0; PlotStyleIndex < NumberOfPlotStyles; PlotStyleIndex++ ) {
		insertItem(PlotStyleIndex);
	}
}
const int EoDlgPlotStyleEditor_FormViewPropertyPage::insertItem(int index) {
	m_listStyles.LockWindowUpdate();	// ***** lock window updates while filling list *****

	OdPsPlotStyle *pPs = (m_pPlotStyleTable->plotStyleAt(index)).get();

	LV_ITEM lvItem;
	::ZeroMemory(&lvItem, sizeof(LV_ITEM));
	lvItem.mask = m_pPlotStyleTable->isAciTableAvailable() ? LVIF_TEXT | LVIF_IMAGE | LVIF_STATE : LVIF_TEXT | LVIF_STATE;
	lvItem.state = 0;
	lvItem.stateMask = 0;
	if (m_pPlotStyleTable->isAciTableAvailable())
		lvItem.iImage = index;
	lvItem.iItem = index;
	lvItem.iSubItem = 0;

	OdString str = pPs->localizedName();
	lvItem.pszText = (LPWSTR)(LPCWSTR)str;

	int nItem = m_listStyles.InsertItem(&lvItem);

	CPsListStyleData *pPsListStyleData =
		new CPsListStyleData(pPs, &m_bitmapList, (char)nItem);

	m_listStyles.SetItemData(nItem, (LPARAM)pPsListStyleData);

	m_listStyles.UnlockWindowUpdate();	// ***** unlock window updates *****

	return nItem;
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::initBitmapList() {
	CBitmapColorInfo *pBitmapColorInfo = new CBitmapColorInfo(MAKEINTRESOURCE(IDB_SELECT_TRUE_COLOR), L"Select true color...");
	CBitmap *bitmapSrc = &(pBitmapColorInfo->m_bitmap); 

	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(255,255,255), L"Use object color"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(255,0,0), L"Red"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(255,255,0), L"Yellow"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(0,255,0), L"Green"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(0,255,255), L"Cyan"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(0,0,255), L"Blue"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(255,0,255), L"Magenta"));
	m_bitmapList.push_back(new CBitmapColorInfo(bitmapSrc, RGB(0,0,0), L"Black"));

	m_bitmapList.push_back(pBitmapColorInfo);
}
BOOL EoDlgPlotStyleEditor_FormViewPropertyPage::OnInitDialog() {
	CPropertyPage::OnInitDialog();

	if (!m_pPlotStyleTable) return FALSE;

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

	SetWindowLong(m_listStyles.m_hWnd, GWL_STYLE, WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_SMALLICON | LVS_SHOWSELALWAYS | LVS_SINGLESEL|LVS_AUTOARRANGE);

	ListView_SetItemState(m_listStyles.m_hWnd, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);


	if (m_pPlotStyleTable->isAciTableAvailable()) {
		m_AddstyleButton.EnableWindow(FALSE);
		m_DelstyleButton.EnableWindow(FALSE);
	}
	else {
		m_AddstyleButton.EnableWindow(TRUE);
		m_DelstyleButton.EnableWindow(FALSE);
	}
	return TRUE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelchangeComboColor() {
	// TODO: Add your control notification handler code here
	OdInt16 intColorPolicy = 1;
	int CurrentSelection = m_Color.GetCurSel();
	int cListStylesItem = m_listStyles.GetSelectionMark();
	CPsListStyleData *pPsListStyleData = (CPsListStyleData*)(m_listStyles.GetItemData(cListStylesItem));

	if (CurrentSelection == m_Color.GetCount() - 1) {
		CColorDialog dlgColor;
		if (dlgColor.DoModal() == IDOK) {
			deleteCustomColor();

			COLORREF color = dlgColor.GetColor();
			m_Color.SetCurSel(replaceCustomColor(color, cListStylesItem));
			intColorPolicy = 3;
		}
	}
	else {
		pPsListStyleData->SetActiveListIndex(CurrentSelection);
		if (CurrentSelection) intColorPolicy = 5;
	}
	OdCmEntityColor color = pPsListStyleData->GetColor();
	// m_pPlotStyleActive->setColorPolicy(intColorPolicy);

	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);

	OdPsData.setColor(color);
	m_pPlotStyleActive->setData(OdPsData);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboColor() {
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboDither() {
	int CurrentSelection = m_Dither.GetCurSel();
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setDitherOn(CurrentSelection == 0 ? true : false);
	m_pPlotStyleActive->setData(OdPsData);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboGrayScale() {
	int CurrentSelection = m_Grayscale.GetCurSel();
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setGrayScaleOn(CurrentSelection == 0 ? true : false);
	m_pPlotStyleActive->setData(OdPsData);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboAdaptive() {
	int CurrentSelection = m_Adaptive.GetCurSel();
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setAdaptiveLinetype(CurrentSelection == 0 ? true : false);
	m_pPlotStyleActive->setData(OdPsData);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboLineWeight() {
	int CurrentSelection = m_Lineweight.GetCurSel();
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setLineweight(CurrentSelection);
	m_pPlotStyleActive->setData(OdPsData);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboLineEndStyle() {
	int CurrentSelection = m_Lineendstyle.GetCurSel();
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setEndStyle(OdPs::LineEndStyle(CurrentSelection));
	m_pPlotStyleActive->setData(OdPsData);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboFillStyle() {
	int CurrentSelection = m_Fillstyle.GetCurSel();
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setFillStyle(OdPs::FillStyle(CurrentSelection + 64));
	m_pPlotStyleActive->setData(OdPsData);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboLineJoinStyle() {
	int CurrentSelection = m_Linejoinstyle.GetCurSel();
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setJoinStyle(OdPs::LineJoinStyle(CurrentSelection));
	m_pPlotStyleActive->setData(OdPsData);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSelendokComboLineType() {
	int CurrentSelection = m_Linetype.GetCurSel();
	OdPsPlotStyleData OdPsData;
	m_pPlotStyleActive->getData(OdPsData);
	OdPsData.setLinetype(OdPs::LineType(CurrentSelection));
	m_pPlotStyleActive->setData(OdPsData);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnDelBtnStyle() {
	// TODO: Add your control notification handler code here
	int iItem = m_listStyles.GetSelectionMark();

	m_pPlotStyleActive = m_pPlotStyleTable->delPlotStyle(m_pPlotStyleActive);
	CPsListStyleData* pPsListStyleData = (CPsListStyleData*)(m_listStyles.GetItemData(iItem));
	m_listStyles.DeleteItem(iItem);
	delete pPsListStyleData;
	m_listStyles.SetItemState(iItem-1, LVIS_SELECTED, LVIS_SELECTED);
	m_listStyles.SetSelectionMark(iItem-1);
	m_listStyles.SetFocus();
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::AddNewPlotStyle(LPCWSTR styleName) {
	m_pPlotStyleActive = m_pPlotStyleTable->addNewPlotStyle(styleName);
	int ItemIndex = m_pPlotStyleTable->plotStyleSize() - 1;
	insertItem(ItemIndex);
	m_listStyles.SetItemState(ItemIndex, LVIS_SELECTED, LVIS_SELECTED);
	m_listStyles.SetSelectionMark(ItemIndex);
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnAddBtnStyle() {
	DialogBox(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD_PS_DLG_ADDPS), m_hWnd, Dlg_Proc); 
	m_listStyles.SetFocus();
}
BOOL EoDlgPlotStyleEditor_FormViewPropertyPage::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags/*, BOOL bOpenFileDialog*/) {
	CString ext = fileName.Right(3);
	bool isCtb = m_pPlotStyleTable->isAciTableAvailable();

	CFileDialog dlgFile(FALSE);

	CString title = L"Save As";

	dlgFile.m_ofn.Flags |= lFlags;

	CString strFilter;
	CString strDefault;

	strFilter = isCtb 
		? L"Color-Dependent Style Table Files (*.ctb)"
		: L"Style Table Files (*.stb)";
	strFilter += (wchar_t)'\0'; // next string please
	strFilter += isCtb ? L"*.ctb" : L"*.stb";
	strFilter += (wchar_t)'\0'; // last string
	dlgFile.m_ofn.nMaxCustFilter++;
	dlgFile.m_ofn.nFilterIndex = 1;

	if (fileName.ReverseFind('.') != - 1) {
		fileName = fileName.Left(fileName.ReverseFind('.'));
	}
	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(MAX_PATH);

	LPARAM nResult = dlgFile.DoModal();
	if (nResult == IDOK ) {
		fileName = dlgFile.GetPathName();
		if (fileName.ReverseFind('.') == -1) {
			fileName += isCtb ? L".ctb" : L".stb";
		}
	}
	return nResult == IDOK;
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::SetFileBufPath(const OdString sFilePath) {
	m_sFileBufPath = sFilePath;
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnSaveBtn() {
	CString sPath = (LPCWSTR)m_sFileBufPath;

	if (!DoPromptFileName(sPath, AFX_IDS_SAVEFILE, OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST))
		return;	// don't even attempt to save


	OdStreamBufPtr pFileBuf;
	OdDbSystemServices* pSs = odSystemServices();
	try {
		pFileBuf = pSs->createFile(OdString((LPCWSTR)sPath), Oda::kFileWrite, Oda::kShareDenyWrite, Oda::kOpenAlways/*Oda::kCreateAlways*/);
		if (pFileBuf.get()) {
			OdPsPlotStyleServicesPtr pPSS = odrxDynamicLinker()->loadApp(ODPS_PLOTSTYLE_SERVICES_APPNAME);
			if (pPSS.get()) {
				pPSS->savePlotStyleTable(pFileBuf, m_pPlotStyleTable);
			}
		}
	}
	catch(...)
	{
		return;
	}
}
void EoDlgPlotStyleEditor_FormViewPropertyPage::OnLineweightBtn() {
	EoDlgPlotStyleEditLineweight PsEditLineweightDlg;
	PsEditLineweightDlg.SetPlotStyleTable(m_pPlotStyleTable);
	OdPsPlotStyleData OdPsData;
	int idx = m_Lineweight.GetCurSel();
	if (idx == CB_ERR) {
		m_pPlotStyleActive->getData(OdPsData);
		idx = (int)OdPsData.lineweight();
	}
	PsEditLineweightDlg.SetInitialSelection(m_Lineweight.GetCurSel());
	if (PsEditLineweightDlg.DoModal() == IDOK) {
		m_Lineweight.ResetContent();
		initLineweightComboBox();
		m_pPlotStyleActive->getData(OdPsData);
		m_Lineweight.SetCurSel((OdUInt32)OdPsData.lineweight());
	}
}
const int EoDlgPlotStyleEditor_FormViewPropertyPage::deleteCustomColor() {
	if (m_Color.GetCount() > PS_COMBO_COLOR_POSITION+1)
		m_Color.DeleteString(PS_COMBO_COLOR_POSITION);

	return 0;
}
const int EoDlgPlotStyleEditor_FormViewPropertyPage::appendCustomColor(const int item) {
	CPsListStyleData* pPsListStyleData = (CPsListStyleData*)(m_listStyles.GetItemData(item));
	CBitmapColorInfo* pBitmapColorInfo = pPsListStyleData->GetBitmapColorInfo();
	if (!pBitmapColorInfo) 
		return pPsListStyleData->GetActiveListIndex();

	return m_Color.InsertBitmap(PS_COMBO_COLOR_POSITION, &pBitmapColorInfo->m_bitmap, pBitmapColorInfo->m_name);

}
const int EoDlgPlotStyleEditor_FormViewPropertyPage::replaceCustomColor(COLORREF color, const int item) {
	CPsListStyleData* pPsListStyleData = (CPsListStyleData*)(m_listStyles.GetItemData(item));
	pPsListStyleData->ReplaceBitmapColorInfo(color, item);

	CBitmapColorInfo* pBitmapColorInfo = pPsListStyleData->GetBitmapColorInfo();
	if (!pBitmapColorInfo) 
		return pPsListStyleData->GetActiveListIndex();

	return m_Color.InsertBitmap(PS_COMBO_COLOR_POSITION, &pBitmapColorInfo->m_bitmap, pBitmapColorInfo->m_name);
}