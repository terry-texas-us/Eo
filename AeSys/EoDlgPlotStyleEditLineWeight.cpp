#include "stdafx.h"

#include "EoDlgPlotStyleEditLineweight.h"
#include "EoDlgPlotStyleTableEditor_FormViewPropertyPage.h"
#include "Ge/GeIntArray.h"

typedef struct {
	CListCtrl* m_ListCtrl;
	int m_SubItem; // not used
	char m_Type; // not used
} EoListCtrlSortData;

static int CALLBACK EoLineweightCompareFunction(LPARAM item1, LPARAM item2, LPARAM sortData) {
	const int NewIndex1 = ((EoLineweightData*) item1)->m_NewIdx;
	const int NewIndex2 = ((EoLineweightData*) item2)->m_NewIdx;
	const CListCtrl* ListCtrl = ((EoListCtrlSortData*) sortData)->m_ListCtrl;

	double Value1 = _wtof(ListCtrl->GetItemText(NewIndex1, 0));
	double Value2 = _wtof(ListCtrl->GetItemText(NewIndex2, 0));

	return (Value1 > Value2);
}

IMPLEMENT_DYNAMIC(EoDlgPlotStyleEditLineweight, CDialog)

BEGIN_MESSAGE_MAP(EoDlgPlotStyleEditLineweight, CDialog)
	ON_BN_CLICKED(IDC_MILLIMETERS, OnRadioMillimetrs)
	ON_BN_CLICKED(IDC_INCHES, OnRadioInches)
	ON_BN_CLICKED(IDC_EDITLINEWEIGHT, OnButtonEditlineweight)
	ON_BN_CLICKED(IDC_SORTLINEWEIGHT, OnButtonSortlineweight)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_LINEWEIGHTS, OnEndlabeleditListLineweight)
END_MESSAGE_MAP()

EoDlgPlotStyleEditLineweight::EoDlgPlotStyleEditLineweight(CWnd* parent) 
	: CDialog(EoDlgPlotStyleEditLineweight::IDD, parent) {
	m_PlotStyleTable = NULL;
	m_InitialSelection = 0;
	m_LineweightData = 0;
}
BOOL EoDlgPlotStyleEditLineweight::DestroyWindow() {
	delete m_LineweightData;
	m_LineweightData = 0;
	return CDialog::DestroyWindow();
}
void EoDlgPlotStyleEditLineweight::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LINEWEIGHTS, m_LineweightsListCtrl);
	DDX_Control(pDX, IDC_MILLIMETERS, m_MillimetrsButton);
	DDX_Control(pDX, IDC_INCHES, m_InchesButton); 
}

void EoDlgPlotStyleEditLineweight::OnOK() {
	// <tas="Is extra validation needed here?"</tas>
	OnButtonSortlineweight();
	m_PlotStyleTable->setDisplayCustomLineweightUnits(m_InchesButton.GetCheck() ? true : false);
	const size_t NumberOfPlotStyles = m_PlotStyleTable->plotStyleSize();
	OdPsPlotStyleData PlotStyleData;
	const int iLineweightQnt = m_PlotStyleTable->lineweightSize();
	for (size_t PlotStyleIndex = 0; PlotStyleIndex < NumberOfPlotStyles; PlotStyleIndex++ ) {
		OdPsPlotStylePtr PlotStyle = m_PlotStyleTable->plotStyleAt(PlotStyleIndex);
		PlotStyle->getData(PlotStyleData);
		const int lineweight = (int) PlotStyleData.lineweight() - 1;
		for (int j = 0; j < iLineweightQnt; j++) {
			if (m_LineweightData[j].m_OldIdx == lineweight) {
				if (m_LineweightData[j].m_OldIdx != m_LineweightData[j].m_NewIdx) {
					PlotStyleData.setLineweight(double(m_LineweightData[j].m_NewIdx) + 1.);
					PlotStyle->setData(PlotStyleData);
				}
				break;
			}
		}
	}
	OdGeDoubleArray Lineweights;
	const int NumberOfLineweights = m_LineweightsListCtrl.GetItemCount();
	Lineweights.resize(NumberOfLineweights);
	for (int LineweightIndex = 0; LineweightIndex < NumberOfLineweights;  LineweightIndex++) {
		EoLineweightData* LineweightDataItem = (EoLineweightData*) m_LineweightsListCtrl.GetItemData(LineweightIndex);
		Lineweights[LineweightIndex] = LineweightDataItem->m_Value;
	}
	m_PlotStyleTable->setLineweights(Lineweights);

	CDialog::OnOK();
}
void EoDlgPlotStyleEditLineweight::OnButtonSortlineweight() {
	EoListCtrlSortData SortData;
	SortData.m_ListCtrl = &m_LineweightsListCtrl;
	SortData.m_SubItem = 0;
	SortData.m_Type = 0;
	m_LineweightsListCtrl.SortItems(EoLineweightCompareFunction, (DWORD_PTR) &SortData);
	const int NumberOfLineweights = m_LineweightsListCtrl.GetItemCount();
	for (int LineWeightIndex = 0; LineWeightIndex < NumberOfLineweights; LineWeightIndex++) {
		EoLineweightData* LineweightDataItem = (EoLineweightData*) m_LineweightsListCtrl.GetItemData(LineWeightIndex);
		LineweightDataItem->m_NewIdx = LineWeightIndex; 
	}
}
void EoDlgPlotStyleEditLineweight::OnEndlabeleditListLineweight(NMHDR* pNMHDR, LRESULT* result) {
	LV_DISPINFO* DisplayInfo = (LV_DISPINFO*) pNMHDR;
	const LV_ITEM* pItem = &((LV_DISPINFO*) DisplayInfo)->item;
	if (pItem->mask & LVIF_TEXT) {
		CString str;
		str.Format(L"%.4f", _wtof(pItem->pszText));
		m_LineweightsListCtrl.SetItemText(pItem->iItem,  pItem->iSubItem, str);
		EoLineweightData* LineweightDataItem = (EoLineweightData*) m_LineweightsListCtrl.GetItemData(pItem->iItem);
		if (m_InchesButton.GetCheck()) {
			LineweightDataItem->m_Value = INCHTOMM(_wtof(pItem->pszText));
		}
		else {
			LineweightDataItem->m_Value = _wtof(pItem->pszText);
		}
	}
	*result = 0;
}
void EoDlgPlotStyleEditLineweight::OnButtonEditlineweight() {
	CRect rect;
	m_LineweightsListCtrl.GetWindowRect(&rect);
	ScreenToClient(&rect);
	m_LineweightsListCtrl.ModifyStyle(0, LVS_EDITLABELS);
	InvalidateRect(rect);
	UpdateData(FALSE);

	m_LineweightsListCtrl.SetFocus();
	m_LineweightsListCtrl.EditLabel(m_LineweightsListCtrl.GetSelectionMark());
} 
void EoDlgPlotStyleEditLineweight::SetInitialSelection(int selection) {
	m_InitialSelection = !selection ? selection : selection - 1;
}
void EoDlgPlotStyleEditLineweight::SetUnitIntoList(const bool isInchUnits) {
	CString Lineweight;
	const int NumberOfLineweights = m_LineweightsListCtrl.GetItemCount();
	for (int i = 0; i < NumberOfLineweights; i++) {
		const EoLineweightData* LineweightDataItem = (EoLineweightData*) m_LineweightsListCtrl.GetItemData(i);
		const double Value = LineweightDataItem->m_Value;
		Lineweight.Format(L"%.4f", isInchUnits ? MMTOINCH(Value) : Value);

		LV_ITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = (LPWSTR)(LPCWSTR) Lineweight;
		lvi.cchTextMax = Lineweight.GetLength();
		m_LineweightsListCtrl.SetItem(&lvi);
	}
}
void EoDlgPlotStyleEditLineweight::OnRadioMillimetrs() {
	SetUnitIntoList(false);
}
void EoDlgPlotStyleEditLineweight::OnRadioInches() {
	SetUnitIntoList(true);
}
const bool EoDlgPlotStyleEditLineweight::SetPlotStyleTable(OdPsPlotStyleTable* plotStyleTable) {
	if (!plotStyleTable) {
		return false;
	}
	m_PlotStyleTable = plotStyleTable;
	return true;
}
void EoDlgPlotStyleEditLineweight::InitializeLineweightsListCtrlImages() {
	VERIFY(m_ListCtrlImages.Create(IDB_PS_BITMAP_WHITE, 16, 3, RGB(255, 255, 255)));
	HBITMAP Bitmap = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PS_BITMAP_CHECK), IMAGE_BITMAP, 16, 16, LR_CREATEDIBSECTION); 
	m_ListCtrlImages.Add(CBitmap::FromHandle(Bitmap), RGB(255, 255, 255));
	m_LineweightsListCtrl.SetImageList(&m_ListCtrlImages, LVSIL_SMALL);
}
void EoDlgPlotStyleEditLineweight::InitializeListCtrl() {
	delete m_LineweightData;
	m_LineweightData = 0;

	LV_COLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_IMAGE;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 80;

	lvColumn.iSubItem = 0;
	lvColumn.pszText = L"Value";
	m_LineweightsListCtrl.InsertColumn(0, &lvColumn);
	
	lvColumn.iSubItem = 1;
	lvColumn.pszText = L"In Use";
	m_LineweightsListCtrl.InsertColumn(1, &lvColumn);
	
	OdGeIntArray useLineWeightIndex;
	OdPsPlotStylePtr pPs;
	OdPsPlotStyleData OdPsData;
	const size_t NumberOfPlotStyles = m_PlotStyleTable->plotStyleSize();
	for (size_t PlotStyleIndex = 0; PlotStyleIndex < NumberOfPlotStyles; PlotStyleIndex++) {
		pPs = m_PlotStyleTable->plotStyleAt(PlotStyleIndex);
		pPs->getData(OdPsData);
		const size_t value = (size_t) OdPsData.lineweight() - 1;
		size_t nIndex;
		if (!useLineWeightIndex.find(value, nIndex)) {
			useLineWeightIndex.push_back(value);
		}
	}
	m_LineweightData = new EoLineweightData[m_PlotStyleTable->lineweightSize()];
	const bool bInch = m_PlotStyleTable->isDisplayCustomLineweightUnits();
	for (size_t i = 0; i < m_PlotStyleTable->lineweightSize(); i++) {
		m_LineweightData[i].m_OldIdx = i;
		m_LineweightData[i].m_NewIdx = i;
		m_LineweightData[i].m_Value = m_PlotStyleTable->getLineweightAt(i);

		CString lineweight;
		lineweight.Format(L"%.4f", bInch ? MMTOINCH(m_LineweightData[i].m_Value) : m_LineweightData[i].m_Value);
		bool isUse = false;
		size_t nIndex;

		if (useLineWeightIndex.find(i, nIndex)) {
			isUse = true;
		}
		const size_t item = InsertLineweightAt(i, lineweight, isUse);
		m_LineweightsListCtrl.SetItemData(item, (DWORD) &m_LineweightData[i]);
	}
	m_LineweightsListCtrl.SetItemState(m_InitialSelection, LVIS_SELECTED, LVIS_SELECTED);
}
const int EoDlgPlotStyleEditLineweight::InsertLineweightAt(int index, const CString& lineweight, const bool isUse) {
	m_LineweightsListCtrl.LockWindowUpdate();	

	LV_ITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_STATE;
	lvItem.state = 0;
	lvItem.stateMask = 0;
	lvItem.iItem = index;
	lvItem.iSubItem = 0;

	lvItem.pszText = (LPWSTR)(LPCWSTR) lineweight;
	const int nItem = m_LineweightsListCtrl.InsertItem(&lvItem);

	LV_ITEM lvItem1;
	lvItem1.mask = LVIF_STATE | LVIF_IMAGE;
	lvItem1.state = 0;
	lvItem1.stateMask = 0;
	lvItem1.iImage = isUse ? 1 : 0;
	lvItem1.iItem = index;
	lvItem1.iSubItem = 1;
	m_LineweightsListCtrl.SetItem(&lvItem1);

	m_LineweightsListCtrl.UnlockWindowUpdate();	

	return nItem;
}
BOOL EoDlgPlotStyleEditLineweight::OnInitDialog() {
	CDialog::OnInitDialog();

	if (!m_PlotStyleTable) {
		return FALSE;
	}
	if (m_PlotStyleTable->isDisplayCustomLineweightUnits()) {
		m_InchesButton.SetCheck(1);
	}
	else {
		m_MillimetrsButton.SetCheck(1);
	}
	InitializeLineweightsListCtrlImages();
	m_LineweightsListCtrl.SetExtendedStyle(LVS_EX_SUBITEMIMAGES);
	InitializeListCtrl();

	return TRUE;
}
