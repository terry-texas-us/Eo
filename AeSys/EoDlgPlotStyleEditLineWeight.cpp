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
	const auto NewIndex1 {reinterpret_cast<EoLineweightData*>(item1)->m_NewIdx};
	const auto NewIndex2 {reinterpret_cast<EoLineweightData*>(item2)->m_NewIdx};
	const CListCtrl* ListCtrl = reinterpret_cast<EoListCtrlSortData*>(sortData)->m_ListCtrl;
	auto Value1 {_wtof(ListCtrl->GetItemText(NewIndex1, 0))};
	auto Value2 {_wtof(ListCtrl->GetItemText(NewIndex2, 0))};

	return Value1 > Value2;
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
	: CDialog(IDD, parent) {
	m_PlotStyleTable = nullptr;
	m_InitialSelection = 0;
	m_LineweightData = nullptr;
}
BOOL EoDlgPlotStyleEditLineweight::DestroyWindow() {
	delete m_LineweightData;
	m_LineweightData = nullptr;
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
	const auto NumberOfPlotStyles {m_PlotStyleTable->plotStyleSize()};
	OdPsPlotStyleData PlotStyleData;
	const auto LineweightQnt {m_PlotStyleTable->lineweightSize()};
	
	for (unsigned PlotStyleIndex = 0; PlotStyleIndex < NumberOfPlotStyles; PlotStyleIndex++ ) {
		auto PlotStyle {m_PlotStyleTable->plotStyleAt(static_cast<int>(PlotStyleIndex))};
		PlotStyle->getData(PlotStyleData);
		const auto lineweight {static_cast<int>(PlotStyleData.lineweight()) - 1};
		for (unsigned j = 0; j < LineweightQnt; j++) {
			if (m_LineweightData[j].m_OldIdx == lineweight) {
				
				if (m_LineweightData[j].m_OldIdx != m_LineweightData[j].m_NewIdx) {
					PlotStyleData.setLineweight(static_cast<double>(m_LineweightData[j].m_NewIdx) + 1.0);
					PlotStyle->setData(PlotStyleData);
				}
				break;
			}
		}
	}
	OdGeDoubleArray Lineweights;
	const auto NumberOfLineweights {m_LineweightsListCtrl.GetItemCount()};
	Lineweights.resize(static_cast<unsigned>(NumberOfLineweights));
	for (auto LineweightIndex = 0; LineweightIndex < NumberOfLineweights;  LineweightIndex++) {
		auto LineweightDataItem {reinterpret_cast<EoLineweightData*>(m_LineweightsListCtrl.GetItemData(LineweightIndex))};
		Lineweights[static_cast<unsigned>(LineweightIndex)] = LineweightDataItem->m_Value;
	}
	m_PlotStyleTable->setLineweights(Lineweights);

	CDialog::OnOK();
}

void EoDlgPlotStyleEditLineweight::OnButtonSortlineweight() {
	EoListCtrlSortData SortData;
	SortData.m_ListCtrl = &m_LineweightsListCtrl;
	SortData.m_SubItem = 0;
	SortData.m_Type = 0;
	m_LineweightsListCtrl.SortItems(EoLineweightCompareFunction, reinterpret_cast<DWORD_PTR>(&SortData));
	const auto NumberOfLineweights {m_LineweightsListCtrl.GetItemCount()};
	for (auto LineWeightIndex = 0; LineWeightIndex < NumberOfLineweights; LineWeightIndex++) {
		EoLineweightData* LineweightDataItem = reinterpret_cast<EoLineweightData*>(m_LineweightsListCtrl.GetItemData(LineWeightIndex));
		LineweightDataItem->m_NewIdx = LineWeightIndex; 
	}
}

void EoDlgPlotStyleEditLineweight::OnEndlabeleditListLineweight(NMHDR* notifyStructure, LRESULT* result) {
	auto DisplayInfo {reinterpret_cast<NMLVDISPINFOW*>(notifyStructure)};
	const auto ListViewItem {&static_cast<NMLVDISPINFOW*>(DisplayInfo)->item};

	if (ListViewItem->mask & LVIF_TEXT) {
		CString Text;
		Text.Format(L"%.4f", _wtof(ListViewItem->pszText));
		m_LineweightsListCtrl.SetItemText(ListViewItem->iItem, ListViewItem->iSubItem, Text);
		auto LineweightDataItem = reinterpret_cast<EoLineweightData*>(m_LineweightsListCtrl.GetItemData(ListViewItem->iItem));

		if (m_InchesButton.GetCheck()) {
			LineweightDataItem->m_Value = INCHTOMM(_wtof(ListViewItem->pszText));
		} else {
			LineweightDataItem->m_Value = _wtof(ListViewItem->pszText);
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

void EoDlgPlotStyleEditLineweight::SetInitialSelection(int selection) noexcept {
	m_InitialSelection = static_cast<unsigned>(!selection ? selection : selection - 1);
}

void EoDlgPlotStyleEditLineweight::SetUnitIntoList(bool isInchUnits) {
	CString Lineweight;
	const auto NumberOfLineweights {m_LineweightsListCtrl.GetItemCount()};

	for (auto LineweightIndex = 0; LineweightIndex < NumberOfLineweights; LineweightIndex++) {
		const auto LineweightDataItem {reinterpret_cast<EoLineweightData*>(m_LineweightsListCtrl.GetItemData(LineweightIndex))};
		const auto Value {LineweightDataItem->m_Value};

		Lineweight.Format(L"%.4f", isInchUnits ? MMTOINCH(Value) : Value);

		LVITEMW lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = LineweightIndex;
		lvi.iSubItem = 0;
		lvi.pszText = Lineweight.GetBuffer(Lineweight.GetLength());
		lvi.cchTextMax = Lineweight.GetLength();
		m_LineweightsListCtrl.SetItem(&lvi);
		Lineweight.ReleaseBuffer();
	}
}

void EoDlgPlotStyleEditLineweight::OnRadioMillimetrs() {
	SetUnitIntoList(false);
}

void EoDlgPlotStyleEditLineweight::OnRadioInches() {
	SetUnitIntoList(true);
}

bool EoDlgPlotStyleEditLineweight::SetPlotStyleTable(OdPsPlotStyleTable* plotStyleTable) noexcept {
	
	if (!plotStyleTable) { return false; }
	
	m_PlotStyleTable = plotStyleTable;
	return true;
}

void EoDlgPlotStyleEditLineweight::InitializeLineweightsListCtrlImages() {
	VERIFY(m_ListCtrlImages.Create(IDB_PS_BITMAP_WHITE, 16, 3, RGB(255, 255, 255)));
	auto Bitmap {static_cast<HBITMAP>(LoadImageW(AfxGetInstanceHandle(), MAKEINTRESOURCEW(IDB_PS_BITMAP_CHECK), IMAGE_BITMAP, 16, 16, LR_CREATEDIBSECTION))};
	m_ListCtrlImages.Add(CBitmap::FromHandle(Bitmap), RGB(255, 255, 255));
	m_LineweightsListCtrl.SetImageList(&m_ListCtrlImages, LVSIL_SMALL);
}

void EoDlgPlotStyleEditLineweight::InitializeListCtrl() {
	delete m_LineweightData;
	m_LineweightData = nullptr;

	m_LineweightsListCtrl.InsertColumn(0, L"Value", LVCFMT_LEFT, 80, 0);
	m_LineweightsListCtrl.InsertColumn(1, L"In Use", LVCFMT_LEFT, 80, 0);
	
	OdGeIntArray useLineWeightIndex;
	OdPsPlotStyleData OdPsData;
	const auto NumberOfPlotStyles {m_PlotStyleTable->plotStyleSize()};

	for (unsigned PlotStyleIndex = 0; PlotStyleIndex < NumberOfPlotStyles; PlotStyleIndex++) {
		auto PlotStyle {m_PlotStyleTable->plotStyleAt(gsl::narrow_cast<int>(PlotStyleIndex))};
		PlotStyle->getData(OdPsData);
		const auto value {static_cast<int>(OdPsData.lineweight()) - 1};
		unsigned nIndex;

		if (!useLineWeightIndex.find(value, nIndex)) { useLineWeightIndex.push_back(value); }
	}
	m_LineweightData = new EoLineweightData[m_PlotStyleTable->lineweightSize()];
	const auto Inch {m_PlotStyleTable->isDisplayCustomLineweightUnits()};
	
	for (unsigned i = 0; i < m_PlotStyleTable->lineweightSize(); i++) {
		m_LineweightData[i].m_OldIdx = static_cast<int>(i);
		m_LineweightData[i].m_NewIdx = static_cast<int>(i);
		m_LineweightData[i].m_Value = m_PlotStyleTable->getLineweightAt(i);

		OdString lineweight;
		lineweight.format(L"%.4f", Inch ? MMTOINCH(m_LineweightData[i].m_Value) : m_LineweightData[i].m_Value);
		auto IsUse {false};
		unsigned nIndex {0};

		if (useLineWeightIndex.find(static_cast<int>(i), nIndex)) { IsUse = true; }

		const auto item {InsertLineweightAt(static_cast<int>(i), lineweight, IsUse)};
		m_LineweightsListCtrl.SetItemData(item, reinterpret_cast<unsigned long>(&m_LineweightData[i]));
	}
	m_LineweightsListCtrl.SetItemState(static_cast<int>(m_InitialSelection), LVIS_SELECTED, LVIS_SELECTED);
}

int EoDlgPlotStyleEditLineweight::InsertLineweightAt(int index, const OdString& lineweight, bool isUse) {
	m_LineweightsListCtrl.LockWindowUpdate();	

	LVITEMW lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_STATE;
	lvItem.state = 0;
	lvItem.stateMask = 0;
	lvItem.iItem = index;
	lvItem.iSubItem = 0;
	lvItem.pszText = (wchar_t*)lineweight.c_str();

	const auto nItem {m_LineweightsListCtrl.InsertItem(&lvItem)};

	LVITEMW lvItem1;
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
	} else {
		m_MillimetrsButton.SetCheck(1);
	}
	InitializeLineweightsListCtrlImages();
	m_LineweightsListCtrl.SetExtendedStyle(LVS_EX_SUBITEMIMAGES);
	InitializeListCtrl();

	return TRUE;
}
