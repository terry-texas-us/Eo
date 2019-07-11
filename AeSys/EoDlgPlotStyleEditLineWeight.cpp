#include "stdafx.h"
#include <Ge/GeIntArray.h>
#include "EoDlgPlotStyleEditLineweight.h"
#include "EoDlgPlotStyleTableEditor_FormViewPropertyPage.h"
using EoListCtrlSortData = struct {
	CListCtrl* listControl;
	int subItem; // not used
	char type; // not used
};

static int CALLBACK EoLineweightCompareFunction(const LPARAM item1, const LPARAM item2, const LPARAM sortData) {
	const auto NewIndex1 {reinterpret_cast<EoLineweightData*>(item1)->newIndex};
	const auto NewIndex2 {reinterpret_cast<EoLineweightData*>(item2)->newIndex};
	const CListCtrl* ListCtrl = reinterpret_cast<EoListCtrlSortData*>(sortData)->listControl;
	const auto Value1 {_wtof(ListCtrl->GetItemText(NewIndex1, 0))};
	const auto Value2 {_wtof(ListCtrl->GetItemText(NewIndex2, 0))};
	return static_cast<int>(Value1 > Value2);
}

IMPLEMENT_DYNAMIC(EoDlgPlotStyleEditLineweight, CDialog)
#pragma warning(push)
#pragma warning(disable : 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
BEGIN_MESSAGE_MAP(EoDlgPlotStyleEditLineweight, CDialog)
		ON_BN_CLICKED(IDC_MILLIMETERS, OnRadioMillimeters)
		ON_BN_CLICKED(IDC_INCHES, OnRadioInches)
		ON_BN_CLICKED(IDC_EDITLINEWEIGHT, OnButtonEditLineweight)
		ON_BN_CLICKED(IDC_SORTLINEWEIGHT, OnButtonSortLineweight)
		ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_LINEWEIGHTS, OnEndlabeleditListLineweight)
END_MESSAGE_MAP()
#pragma warning (pop)
EoDlgPlotStyleEditLineweight::EoDlgPlotStyleEditLineweight(CWnd* parent)
	: CDialog(IDD, parent) {
	plotStyleTable = nullptr;
	initialSelection = 0;
	lineweightData = nullptr;
}

BOOL EoDlgPlotStyleEditLineweight::DestroyWindow() {
	delete lineweightData;
	lineweightData = nullptr;
	return CDialog::DestroyWindow();
}

void EoDlgPlotStyleEditLineweight::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_LIST_LINEWEIGHTS, lineweightsListCtrl);
	DDX_Control(dataExchange, IDC_MILLIMETERS, millimetersButton);
	DDX_Control(dataExchange, IDC_INCHES, inchesButton);
}

void EoDlgPlotStyleEditLineweight::OnOK() {
	// <tas="Is extra validation needed here?"</tas>
	OnButtonSortLineweight();
	plotStyleTable->setDisplayCustomLineweightUnits(inchesButton.GetCheck() != 0 ? true : false);
	const auto NumberOfPlotStyles {plotStyleTable->plotStyleSize()};
	OdPsPlotStyleData PlotStyleData;
	const auto LineweightQnt {plotStyleTable->lineweightSize()};
	for (unsigned PlotStyleIndex = 0; PlotStyleIndex < NumberOfPlotStyles; PlotStyleIndex++) {
		auto PlotStyle {plotStyleTable->plotStyleAt(static_cast<int>(PlotStyleIndex))};
		PlotStyle->getData(PlotStyleData);
		const auto lineweight {static_cast<int>(PlotStyleData.lineweight()) - 1};
		for (unsigned j = 0; j < LineweightQnt; j++) {
			if (lineweightData[j].oldIndex == lineweight) {
				if (lineweightData[j].oldIndex != lineweightData[j].newIndex) {
					PlotStyleData.setLineweight(static_cast<double>(lineweightData[j].newIndex) + 1.0);
					PlotStyle->setData(PlotStyleData);
				}
				break;
			}
		}
	}
	OdGeDoubleArray Lineweights;
	const auto NumberOfLineweights {lineweightsListCtrl.GetItemCount()};
	Lineweights.resize(static_cast<unsigned>(NumberOfLineweights));
	for (auto LineweightIndex = 0; LineweightIndex < NumberOfLineweights; LineweightIndex++) {
		const auto LineweightDataItem {reinterpret_cast<EoLineweightData*>(lineweightsListCtrl.GetItemData(LineweightIndex))};
		Lineweights[static_cast<unsigned>(LineweightIndex)] = LineweightDataItem->value;
	}
	plotStyleTable->setLineweights(Lineweights);
	CDialog::OnOK();
}

void EoDlgPlotStyleEditLineweight::OnButtonSortLineweight() {
	EoListCtrlSortData SortData;
	SortData.listControl = &lineweightsListCtrl;
	SortData.subItem = 0;
	SortData.type = 0;
	lineweightsListCtrl.SortItems(EoLineweightCompareFunction, reinterpret_cast<unsigned long>(&SortData));
	const auto NumberOfLineweights {lineweightsListCtrl.GetItemCount()};
	for (auto LineWeightIndex = 0; LineWeightIndex < NumberOfLineweights; LineWeightIndex++) {
		auto LineweightDataItem {reinterpret_cast<EoLineweightData*>(lineweightsListCtrl.GetItemData(LineWeightIndex))};
		LineweightDataItem->newIndex = LineWeightIndex;
	}
}

void EoDlgPlotStyleEditLineweight::OnEndlabeleditListLineweight(NMHDR* notifyStructure, LRESULT* result) {
	const auto DisplayInfo {reinterpret_cast<NMLVDISPINFOW*>(notifyStructure)};
	const auto ListViewItem {&static_cast<NMLVDISPINFOW*>(DisplayInfo)->item};
	if (ListViewItem->mask & LVIF_TEXT) {
		CString Text;
		Text.Format(L"%.4f", _wtof(ListViewItem->pszText));
		lineweightsListCtrl.SetItemText(ListViewItem->iItem, ListViewItem->iSubItem, Text);
		const auto LineweightDataItem = reinterpret_cast<EoLineweightData*>(lineweightsListCtrl.GetItemData(ListViewItem->iItem));
		if (inchesButton.GetCheck() != 0) {
			LineweightDataItem->value = InchesToMillimeters(_wtof(ListViewItem->pszText));
		} else {
			LineweightDataItem->value = _wtof(ListViewItem->pszText);
		}
	}
	*result = 0;
}

void EoDlgPlotStyleEditLineweight::OnButtonEditLineweight() {
	CRect rect;
	lineweightsListCtrl.GetWindowRect(&rect);
	ScreenToClient(&rect);
	lineweightsListCtrl.ModifyStyle(0, LVS_EDITLABELS);
	InvalidateRect(rect);
	UpdateData(FALSE);
	lineweightsListCtrl.SetFocus();
	lineweightsListCtrl.EditLabel(lineweightsListCtrl.GetSelectionMark());
}

void EoDlgPlotStyleEditLineweight::SetInitialSelection(const int selection) noexcept {
	initialSelection = static_cast<unsigned>(selection == 0 ? selection : selection - 1);
}

void EoDlgPlotStyleEditLineweight::SetUnitIntoList(const bool isInchUnits) {
	CString Lineweight;
	const auto NumberOfLineweights {lineweightsListCtrl.GetItemCount()};
	for (auto LineweightIndex = 0; LineweightIndex < NumberOfLineweights; LineweightIndex++) {
		const auto LineweightDataItem {reinterpret_cast<EoLineweightData*>(lineweightsListCtrl.GetItemData(LineweightIndex))};
		const auto Value {LineweightDataItem->value};
		Lineweight.Format(L"%.4f", isInchUnits ? MillimetersToInches(Value) : Value);
		LVITEMW lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = LineweightIndex;
		lvi.iSubItem = 0;
		lvi.pszText = Lineweight.GetBuffer(Lineweight.GetLength());
		lvi.cchTextMax = Lineweight.GetLength();
		lineweightsListCtrl.SetItem(&lvi);
		Lineweight.ReleaseBuffer();
	}
}

void EoDlgPlotStyleEditLineweight::OnRadioMillimeters() {
	SetUnitIntoList(false);
}

void EoDlgPlotStyleEditLineweight::OnRadioInches() {
	SetUnitIntoList(true);
}

bool EoDlgPlotStyleEditLineweight::SetPlotStyleTable(OdPsPlotStyleTable* plotStyleTable) noexcept {
	return plotStyleTable != nullptr;
}

void EoDlgPlotStyleEditLineweight::InitializeLineweightsListCtrlImages() {
	VERIFY(listCtrlImages.Create(IDB_PS_BITMAP_WHITE, 16, 3, RGB(255, 255, 255)));
	const auto Bitmap {static_cast<HBITMAP>(LoadImageW(AfxGetInstanceHandle(), MAKEINTRESOURCEW(IDB_PS_BITMAP_CHECK), IMAGE_BITMAP, 16, 16, LR_CREATEDIBSECTION))};
	listCtrlImages.Add(CBitmap::FromHandle(Bitmap), RGB(255, 255, 255));
	lineweightsListCtrl.SetImageList(&listCtrlImages, LVSIL_SMALL);
}

void EoDlgPlotStyleEditLineweight::InitializeListCtrl() {
	delete lineweightData;
	lineweightData = nullptr;
	lineweightsListCtrl.InsertColumn(0, L"Value", LVCFMT_LEFT, 80, 0);
	lineweightsListCtrl.InsertColumn(1, L"In Use", LVCFMT_LEFT, 80, 0);
	OdGeIntArray useLineWeightIndex;
	OdPsPlotStyleData OdPsData;
	const auto NumberOfPlotStyles {plotStyleTable->plotStyleSize()};
	for (unsigned PlotStyleIndex = 0; PlotStyleIndex < NumberOfPlotStyles; PlotStyleIndex++) {
		auto PlotStyle {plotStyleTable->plotStyleAt(gsl::narrow_cast<int>(PlotStyleIndex))};
		PlotStyle->getData(OdPsData);
		const auto value {static_cast<int>(OdPsData.lineweight()) - 1};
		unsigned nIndex;
		if (!useLineWeightIndex.find(value, nIndex)) { useLineWeightIndex.push_back(value); }
	}
	lineweightData = new EoLineweightData[plotStyleTable->lineweightSize()];
	const auto Inch {plotStyleTable->isDisplayCustomLineweightUnits()};
	for (unsigned i = 0; i < plotStyleTable->lineweightSize(); i++) {
		lineweightData[i].oldIndex = static_cast<int>(i);
		lineweightData[i].newIndex = static_cast<int>(i);
		lineweightData[i].value = plotStyleTable->getLineweightAt(i);
		OdString lineweight;
		lineweight.format(L"%.4f", Inch ? MillimetersToInches(lineweightData[i].value) : lineweightData[i].value);
		auto IsUse {false};
		unsigned nIndex {0};
		if (useLineWeightIndex.find(static_cast<int>(i), nIndex)) { IsUse = true; }
		const auto item {InsertLineweightAt(static_cast<int>(i), lineweight, IsUse)};
		lineweightsListCtrl.SetItemData(item, reinterpret_cast<unsigned long>(&lineweightData[i]));
	}
	lineweightsListCtrl.SetItemState(static_cast<int>(initialSelection), LVIS_SELECTED, LVIS_SELECTED);
}

int EoDlgPlotStyleEditLineweight::InsertLineweightAt(const int index, const OdString& lineweight, const bool isUse) {
	lineweightsListCtrl.LockWindowUpdate();
	LVITEMW lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_STATE;
	lvItem.state = 0;
	lvItem.stateMask = 0;
	lvItem.iItem = index;
	lvItem.iSubItem = 0;
	lvItem.pszText = (wchar_t*)lineweight.c_str();
	const auto nItem {lineweightsListCtrl.InsertItem(&lvItem)};
	LVITEMW lvItem1;
	lvItem1.mask = LVIF_STATE | LVIF_IMAGE;
	lvItem1.state = 0;
	lvItem1.stateMask = 0;
	lvItem1.iImage = isUse ? 1 : 0;
	lvItem1.iItem = index;
	lvItem1.iSubItem = 1;
	lineweightsListCtrl.SetItem(&lvItem1);
	lineweightsListCtrl.UnlockWindowUpdate();
	return nItem;
}

BOOL EoDlgPlotStyleEditLineweight::OnInitDialog() {
	CDialog::OnInitDialog();
	if (plotStyleTable == nullptr) {
		return FALSE;
	}
	if (plotStyleTable->isDisplayCustomLineweightUnits()) {
		inchesButton.SetCheck(1);
	} else {
		millimetersButton.SetCheck(1);
	}
	InitializeLineweightsListCtrlImages();
	lineweightsListCtrl.SetExtendedStyle(LVS_EX_SUBITEMIMAGES);
	InitializeListCtrl();
	return TRUE;
}
