#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoDb.h"
#include "EoDlgFileManage.h"
#include "EoDlgLineWeight.h"
#include "EoDlgSetupColor.h"
#include "EoDlgSetupLinetype.h"

#include "Preview.h"

/// EoDlgFileManage dialog

IMPLEMENT_DYNAMIC(EoDlgFileManage, CDialog)

BEGIN_MESSAGE_MAP(EoDlgFileManage, CDialog)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_FUSE, &EoDlgFileManage::OnBnClickedFuse)
	ON_BN_CLICKED(IDC_MELT, &EoDlgFileManage::OnBnClickedMelt)
	ON_BN_CLICKED(IDC_NEWLAYER, &EoDlgFileManage::OnBnClickedNewlayer)
	ON_BN_CLICKED(IDC_SETCURRENT, &EoDlgFileManage::OnBnClickedSetcurrent)
	ON_LBN_SELCHANGE(IDC_BLOCKS_LIST, &EoDlgFileManage::OnLbnSelchangeBlocksList)
	ON_NOTIFY(NM_CLICK, IDC_LAYERS_LIST_CONTROL, &EoDlgFileManage::OnNMClickLayersListControl)
	ON_NOTIFY(NM_DBLCLK, IDC_LAYERS_LIST_CONTROL, &EoDlgFileManage::OnNMDblclkLayersListControl)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LAYERS_LIST_CONTROL, &EoDlgFileManage::OnItemchangedLayersListControl)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LAYERS_LIST_CONTROL, &EoDlgFileManage::OnLvnEndlabeleditLayersListControl)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LAYERS_LIST_CONTROL, &EoDlgFileManage::OnLvnBeginlabeleditLayersListControl)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LAYERS_LIST_CONTROL, &EoDlgFileManage::OnLvnKeydownLayersListControl)
END_MESSAGE_MAP()

EoDlgFileManage::EoDlgFileManage(CWnd* parent)
	: CDialog(EoDlgFileManage::IDD, parent)
	, m_Document(nullptr)
	, m_ClickToColumnStatus(false)
	, m_Description(0)
	, m_NumberOfColumns(0)
	, m_PreviewWindowHandle(0) {
}

EoDlgFileManage::EoDlgFileManage(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent)
	: CDialog(EoDlgFileManage::IDD, parent)
	, m_Document(document)
	, m_Database(database)
	, m_ClickToColumnStatus(false)
	, m_Description(0)
	, m_NumberOfColumns(0)
	, m_PreviewWindowHandle(0) {
}

EoDlgFileManage::~EoDlgFileManage() {
}

void EoDlgFileManage::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LAYERS_LIST_CONTROL, m_LayersList);
	DDX_Control(pDX, IDC_BLOCKS_LIST, m_BlocksList);
	DDX_Control(pDX, IDC_GROUPS, m_Groups);
}
void EoDlgFileManage::DrawItem(CDC& deviceContext, int itemID, int labelIndex, const RECT& itemRectangle) {
	const EoDbLayer* Layer = (EoDbLayer*) m_LayersList.GetItemData(itemID);
	OdDbLayerTableRecordPtr LayerTableRecord(Layer->TableRecord());

	OdString ItemName;
	switch (labelIndex) {
		case Status:
			if (Layer->IsCurrent()) {
				m_StateImages.Draw(&deviceContext, 8, ((CRect&) itemRectangle).TopLeft(), ILD_TRANSPARENT);
			} else if (LayerTableRecord->isInUse()) {
				m_StateImages.Draw(&deviceContext, 9, ((CRect&) itemRectangle).TopLeft(), ILD_TRANSPARENT);
			} else if (!Layer->IsInternal()) { // <tas="Unconventional usage of status image to flag Tracing files"</tas>
				m_StateImages.Draw(&deviceContext, 10, ((CRect&) itemRectangle).TopLeft(), ILD_TRANSPARENT);
			}
			break;
		case Name:
			ItemName = Layer->Name();
			deviceContext.ExtTextOutW(itemRectangle.left + 6, itemRectangle.top + 1, ETO_CLIPPED, &itemRectangle, ItemName, ItemName.getLength(), nullptr);
			break;
		case On:
			m_StateImages.Draw(&deviceContext, Layer->IsOff() ? 3 : 2, ((CRect&) itemRectangle).TopLeft(), ILD_TRANSPARENT);
			break;
		case Freeze:
			m_StateImages.Draw(&deviceContext, LayerTableRecord->isFrozen() ? 4 : 5, ((CRect&) itemRectangle).TopLeft(), ILD_TRANSPARENT);
			break;
		case Lock:
			m_StateImages.Draw(&deviceContext, LayerTableRecord->isLocked() ? 0 : 1, ((CRect&) itemRectangle).TopLeft(), ILD_TRANSPARENT);
			break;
		case Color:
			CMainFrame::DrawColorBox(deviceContext, itemRectangle, LayerTableRecord->color());
			break;
		case Linetype:
			ItemName = OdDbSymUtil::getSymbolName(LayerTableRecord->linetypeObjectId());
			deviceContext.ExtTextOutW(itemRectangle.left + 6, itemRectangle.top + 1, ETO_CLIPPED, &itemRectangle, ItemName, ItemName.getLength(), nullptr);
			break;
		case Lineweight:
			CMainFrame::DrawLineWeight(deviceContext, itemRectangle, LayerTableRecord->lineWeight());
			break;
		case PlotStyle:
			ItemName = LayerTableRecord->plotStyleName();
			CMainFrame::DrawPlotStyle(deviceContext, itemRectangle, (LPCWSTR) ItemName, m_Database);
			break;
		case Plot:
			m_StateImages.Draw(&deviceContext, LayerTableRecord->isPlottable() ? 6 : 7, ((CRect&) itemRectangle).TopLeft(), ILD_TRANSPARENT);
			break;
		case VpFreeze:
			if (labelIndex != m_Description) {
				OdDbViewportPtr Viewport = OdDbViewport::cast(LayerTableRecord->database()->activeViewportId().safeOpenObject());
				if (Viewport.get()) {
	//				m_StateImages.Draw(&deviceContext, Viewport->isLayerFrozenInViewport(ItemData) ? 4 : 5, ((CRect&) itemRectangle).TopLeft(), ILD_TRANSPARENT);
				}
			} else {
				ItemName = LayerTableRecord->description();
				deviceContext.ExtTextOutW(itemRectangle.left + 6, itemRectangle.top + 1, ETO_CLIPPED, &itemRectangle, ItemName, ItemName.getLength(), nullptr);
			}
			break;
		case VpColor:
			CMainFrame::DrawColorBox(deviceContext, itemRectangle, LayerTableRecord->color(m_ActiveViewport));
			break;
		case VpLinetype:
			ItemName = OdDbSymUtil::getSymbolName(LayerTableRecord->linetypeObjectId(m_ActiveViewport));
			deviceContext.ExtTextOutW(itemRectangle.left + 6, itemRectangle.top + 1, ETO_CLIPPED, &itemRectangle, ItemName, ItemName.getLength(), nullptr);
			break;
		case VpLineweight:
			CMainFrame::DrawLineWeight(deviceContext, itemRectangle, LayerTableRecord->lineWeight(m_ActiveViewport));
			break;
		case VpPlotStyle:
			ItemName = (LPCWSTR) LayerTableRecord->plotStyleName(m_ActiveViewport);
			CMainFrame::DrawPlotStyle(deviceContext, itemRectangle, (LPCWSTR) ItemName, m_Database);
			break;
	}
}
void EoDlgFileManage::OnBnClickedFuse() {
	const int SelectionMark = m_LayersList.GetSelectionMark();
	if (SelectionMark > -1) {
		const EoDbLayer* Layer = (EoDbLayer*) m_LayersList.GetItemData(SelectionMark);
		OdString Name(Layer->Name());
		if (Layer->IsInternal()) {
			theApp.AddStringToMessageList(L"Selection <%s> already an internal layer.\n", Name);
		} else {
			m_Document->TracingFuse(Name);
			m_LayersList.SetItemText(SelectionMark, 0, Name);
			theApp.AddStringToMessageList(IDS_MSG_TRACING_CONVERTED_TO_LAYER, Layer->Name());
		}
	}
}
void EoDlgFileManage::OnBnClickedMelt() {
	const int SelectionMark = m_LayersList.GetSelectionMark();
	if (SelectionMark > -1) {
		const EoDbLayer* Layer = (EoDbLayer*) m_LayersList.GetItemData(SelectionMark);
		OdString Name(Layer->Name());
		if (!Layer->IsInternal()) {
			theApp.AddStringToMessageList(L"Selection <%s> already a tracing.\n", Name);
		} else {
			if (m_Document->LayerMelt(Name)) {
				m_LayersList.SetItemText(SelectionMark, 0, Name);
				theApp.AddStringToMessageList(IDS_MSG_LAYER_CONVERTED_TO_TRACING, Layer->Name());
			}
		}
	}
}
void EoDlgFileManage::OnBnClickedNewlayer() {
	OdDbLayerTablePtr Layers = m_Document->LayerTable(OdDb::kForWrite);

	OdString Name;
	int Suffix = 1;
	do {
		Name.format(L"Layer%d", Suffix++);
	} while (Layers->has(Name));

	OdDbLayerTableRecordPtr LayerTableRecord = OdDbLayerTableRecord::createObject();
	EoDbLayer* Layer = new EoDbLayer(LayerTableRecord);
	LayerTableRecord->setName(Name);
	m_Document->AddLayerTo(Layers, Layer);

	const int ItemCount = m_LayersList.GetItemCount();
	m_LayersList.InsertItem(ItemCount, Name);
	m_LayersList.SetItemData(ItemCount, DWORD_PTR(m_Document->GetLayerAt(Name)));
}
void EoDlgFileManage::OnBnClickedSetcurrent() {
	const int SelectionMark = m_LayersList.GetSelectionMark();
	if (SelectionMark > -1) {
		const EoDbLayer* Layer = (EoDbLayer*) m_LayersList.GetItemData(SelectionMark);
		OdDbLayerTableRecordPtr LayerTableRecord(Layer->TableRecord());
		LayerTableRecord->upgradeOpen();
		if (!LayerTableRecord.isNull()) {
			OdDbLayerTableRecordPtr PreviousLayer = m_Database->getCLAYER().safeOpenObject();
			OdString PreviousLayerName = PreviousLayer->getName();
			m_Document->GetLayerAt(PreviousLayerName)->MakeActive();
			theApp.AddStringToMessageList(L"Status of layer <%s> has changed to active\n", PreviousLayerName);
			m_Database->setCLAYER(LayerTableRecord->objectId());
			theApp.AddStringToMessageList(L"Layer <%s> is now the current working layer\n", LayerTableRecord->getName());
			m_Document->SetCurrentLayer(LayerTableRecord);
			UpdateCurrentLayerInfoField();
			m_LayersList.RedrawWindow();
		}
		LayerTableRecord->downgradeOpen();
	}
}

void EoDlgFileManage::OnDrawItem(int controlIdentifier, LPDRAWITEMSTRUCT drawItemStruct) {
	if (controlIdentifier == IDC_LAYERS_LIST_CONTROL) {
		switch (drawItemStruct->itemAction) {
			case ODA_DRAWENTIRE:
			{
				CRect rcItem(drawItemStruct->rcItem);
				CDC DeviceContext;
				const COLORREF BackgroundColor {::GetSysColor((drawItemStruct->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)};
				DeviceContext.Attach(drawItemStruct->hDC);
				CBrush BackgroundBrush(BackgroundColor);
				DeviceContext.FillRect(rcItem, &BackgroundBrush);

				if (drawItemStruct->itemState & ODS_FOCUS) { DeviceContext.DrawFocusRect(rcItem); }

				const int ItemID {narrow_cast<int>(drawItemStruct->itemID)};

				if (ItemID != -1) { // The text color is stored as the item data.
					const COLORREF TextColor {(drawItemStruct->itemState & ODS_SELECTED) ? ::GetSysColor(COLOR_HIGHLIGHTTEXT) : ::GetSysColor(COLOR_WINDOWTEXT)};
					DeviceContext.SetBkColor(BackgroundColor);
					DeviceContext.SetTextColor(TextColor);
					for (int labelIndex = 0; labelIndex < m_NumberOfColumns; ++labelIndex) {
						m_LayersList.GetSubItemRect(ItemID, labelIndex, LVIR_LABEL, rcItem);
						DrawItem(DeviceContext, ItemID, labelIndex, rcItem);
					}
				}
				DeviceContext.Detach();
			}
			break;

			case ODA_SELECT:
				::InvertRect(drawItemStruct->hDC, &(drawItemStruct->rcItem));
				break;

			case ODA_FOCUS:
				//::DrawFocusRect(drawItemStruct->hDC, &(drawItemStruct->rcItem));
				break;
		}
		return;
	}
	CDialog::OnDrawItem(controlIdentifier, drawItemStruct);
}

BOOL EoDlgFileManage::OnInitDialog() {
	CDialog::OnInitDialog();

	CString CaptionText;
	GetWindowTextW(CaptionText);
	SetWindowTextW(CaptionText + L" - " + m_Document->GetPathName());

	m_PreviewWindowHandle = GetDlgItem(IDC_LAYER_PREVIEW)->GetSafeHwnd();

	m_LayersList.DeleteAllItems();
	m_LayersList.InsertColumn(Status, L"Status", LVCFMT_LEFT, 32);
	m_LayersList.InsertColumn(Name, L"Name", LVCFMT_LEFT, 96);
	m_LayersList.InsertColumn(On, L"On", LVCFMT_LEFT, 32);
	m_LayersList.InsertColumn(Freeze, L"Freeze in all VP", LVCFMT_LEFT, 32);
	m_LayersList.InsertColumn(Lock, L"Lock", LVCFMT_LEFT, 32);
	m_LayersList.InsertColumn(Color, L"Color", LVCFMT_LEFT, 96);
	m_LayersList.InsertColumn(Linetype, L"Linetype", LVCFMT_LEFT, 96);
	m_LayersList.InsertColumn(Lineweight, L"Lineweight", LVCFMT_LEFT, 96);
	m_LayersList.InsertColumn(PlotStyle, L"Plot Style", LVCFMT_LEFT, 64);
	m_NumberOfColumns = m_LayersList.InsertColumn(Plot, L"Plot", LVCFMT_LEFT, 32);

	if (!m_Database->getTILEMODE()) { // Layout (not Model) tab is active
		m_ActiveViewport = m_Database->activeViewportId();
		m_LayersList.InsertColumn(VpFreeze, L"VP Freeze", LVCFMT_LEFT, 32);
		m_LayersList.InsertColumn(VpColor, L"VP Color", LVCFMT_LEFT, 96);
		m_LayersList.InsertColumn(VpLinetype, L"VP Linetype", LVCFMT_LEFT, 96);
		m_LayersList.InsertColumn(VpLineweight, L"VP Lineweight", LVCFMT_LEFT, 96);
		m_NumberOfColumns = m_LayersList.InsertColumn(VpPlotStyle, L"Plot Style", LVCFMT_LEFT, 64);
	}
	m_Description = m_LayersList.InsertColumn(++m_NumberOfColumns, L"Description", LVCFMT_LEFT, 96);
	m_NumberOfColumns++;

	OdDbLayerTablePtr Layers = m_Document->LayerTable(OdDb::kForRead);
	for (int LayerIndex = 0; LayerIndex < m_Document->GetLayerTableSize(); LayerIndex++) {
		const auto Layer {m_Document->GetLayerAt(LayerIndex)};

		m_LayersList.InsertItem(LayerIndex, Layer->Name());
		m_LayersList.SetItemData(LayerIndex, DWORD_PTR(Layer));
	}
	UpdateCurrentLayerInfoField();

	m_BlocksList.SetHorizontalExtent(512);

	CString BlockName;
	EoDbBlock* Block;

	auto Position {m_Document->GetFirstBlockPosition()};
	while (Position != nullptr) {
		m_Document->GetNextBlock(Position, BlockName, Block);

		if (!Block->IsAnonymous()) {
			const int ItemIndex = m_BlocksList.AddString(BlockName);
			m_BlocksList.SetItemData(ItemIndex, DWORD_PTR(Block));
		}
	}
	CBitmap Bitmap;
	Bitmap.LoadBitmap(IDB_LAYER_STATES_HC);
	m_StateImages.Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 1);
	m_StateImages.Add(&Bitmap, RGB(0, 0, 128));

	WndProcPreviewClear(m_PreviewWindowHandle);

	return TRUE;
}
void EoDlgFileManage::OnItemchangedLayersListControl(NMHDR* pNMHDR, LRESULT* result) {
	const LPNMLISTVIEW ListViewNotificationMessage = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if ((ListViewNotificationMessage->uNewState & LVIS_FOCUSED) == LVFIS_FOCUSED) {
		const int Item = ListViewNotificationMessage->iItem;
		EoDbLayer* Layer = (EoDbLayer*) m_LayersList.GetItemData(Item);

		CString NumberOfGroups;
		NumberOfGroups.Format(L"%-4i", Layer->GetCount());
		m_Groups.SetWindowTextW(NumberOfGroups);

		EoDbPrimitive::SetLayerColorIndex(Layer->ColorIndex());
		EoDbPrimitive::SetLayerLinetypeIndex(Layer->LinetypeIndex());

		_WndProcPreviewUpdate(m_PreviewWindowHandle, m_Document->GetLayerAt(Layer->Name()));
	}
	*result = 0;
}
void EoDlgFileManage::OnLbnSelchangeBlocksList() {
	const int CurrentSelection = m_BlocksList.GetCurSel();
	if (CurrentSelection != LB_ERR) {
		if (m_BlocksList.GetTextLen(CurrentSelection) != LB_ERR) {
			CString BlockName;
			m_BlocksList.GetText(CurrentSelection, BlockName);

			EoDbBlock* Block = (EoDbBlock*) m_BlocksList.GetItemData(CurrentSelection);

			m_Groups.SetDlgItemInt(IDC_GROUPS, Block->GetCount(), FALSE);
			WndProcPreviewUpdate(m_PreviewWindowHandle, Block);
		}
	}
}
void EoDlgFileManage::OnNMClickLayersListControl(NMHDR* pNMHDR, LRESULT* result) {
	const LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	const int Item = pNMItemActivate->iItem;
	const int SubItem = pNMItemActivate->iSubItem;

	EoDbLayer* Layer = (EoDbLayer*) m_LayersList.GetItemData(Item);
	OdDbLayerTableRecordPtr LayerTableRecord(Layer->TableRecord());

	m_ClickToColumnStatus = false;
	switch (SubItem) {
		case Status:
			m_ClickToColumnStatus = true;
			break;
		case Name:
			break;
		case On:
			if (Layer->IsCurrent()) {
				theApp.WarningMessageBox(IDS_MSG_LAYER_NO_HIDDEN, LayerTableRecord->getName());
			} else {
				Layer->SetIsOff(!Layer->IsOff());
			}
			break;
		case Freeze:
			Layer->SetIsFrozen(!LayerTableRecord->isFrozen());
			break;
		case Lock:
			if (Layer->IsCurrent()) {
				theApp.WarningMessageBox(IDS_MSG_LAYER_NO_STATIC, LayerTableRecord->getName());
			} else {
				Layer->SetIsLocked(!Layer->IsLocked());
			}
			break;
		case Color:
		{
			EoDlgSetupColor Dialog;
			Dialog.m_ColorIndex = LayerTableRecord->colorIndex();
			if (Dialog.DoModal() == IDOK) {
				Layer->SetColorIndex(Dialog.m_ColorIndex);
			}
			break;
		}
		case Linetype:
		{
			OdDbLinetypeTablePtr Linetypes = m_Database->getLinetypeTableId().safeOpenObject();
			EoDlgSetupLinetype Dialog(Linetypes);
			if (Dialog.DoModal() == IDOK) {
				Layer->SetLinetype(Dialog.m_Linetype->objectId());
			}
			break;
		}
		case Lineweight:
		{
			EoDlgLineWeight Dialog(LayerTableRecord->lineWeight());
			if (Dialog.DoModal() == IDOK) {
				LayerTableRecord->upgradeOpen();
				LayerTableRecord->setLineWeight(Dialog.m_LineWeight);
				LayerTableRecord->downgradeOpen();
			}
			break;
		}
		case PlotStyle:
			break;
		case Plot:
			LayerTableRecord->upgradeOpen();
			LayerTableRecord->setIsPlottable(!LayerTableRecord->isPlottable());
			LayerTableRecord->downgradeOpen();
			break;
		case VpFreeze:
		case Descr:
			if (SubItem != m_Description) {
				OdDbViewportPtr pVp = OdDbViewport::cast(LayerTableRecord->database()->activeViewportId().safeOpenObject(OdDb::kForWrite));
	//			if (pVp.get()) {
	//				OdDbObjectIdArray ids(1);
	//				ids.append(ItemData);
	//				if (pVp->isLayerFrozenInViewport(ItemData)) {
	//					pVp->thawLayersInViewport(ids);
	//				}
	//				else {
	//					pVp->freezeLayersInViewport(ids);
	//				}
	//			}
			} else {
			}
			break;
		case VpColor:
			break;
		case VpLinetype:
			break;
		case VpLineweight:
		{
			EoDlgLineWeight dlg(LayerTableRecord->lineWeight());
			if (IDOK == dlg.DoModal()) {
				LayerTableRecord->upgradeOpen();
				LayerTableRecord->setLineWeight(dlg.m_LineWeight, m_ActiveViewport);
				LayerTableRecord->downgradeOpen();
			}
			break;
		}
		case VpPlotStyle:
			break;
	}
	m_LayersList.Invalidate();

	*result = 0;
}
void EoDlgFileManage::OnNMDblclkLayersListControl(NMHDR* pNMHDR, LRESULT* result) {
	if (m_ClickToColumnStatus) {
		OnBnClickedSetcurrent();
	}
	*result = 0;
}
void EoDlgFileManage::UpdateCurrentLayerInfoField() {
	OdString LayerName = OdDbSymUtil::getSymbolName(m_Database->getCLAYER());
	GetDlgItem(IDC_STATIC_CURRENT_LAYER)->SetWindowTextW(L"Current Layer: " + LayerName);
}
void EoDlgFileManage::OnLvnBeginlabeleditLayersListControl(LPNMHDR pNMHDR, LRESULT * result) {
	const NMLVDISPINFO* ListViewNotificationDisplayInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	const LVITEM Item = ListViewNotificationDisplayInfo->item;
	const EoDbLayer* Layer = (EoDbLayer*) m_LayersList.GetItemData(Item.iItem);
	OdDbLayerTableRecordPtr LayerTableRecord(Layer->TableRecord());
	// <tas="Layer0 should be culled here instead of the EndlabeleditLayers."</tas>
	result = 0;
}
void EoDlgFileManage::OnLvnEndlabeleditLayersListControl(LPNMHDR pNMHDR, LRESULT * result) {
	const NMLVDISPINFO* ListViewNotificationDisplayInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	const LVITEM Item = ListViewNotificationDisplayInfo->item;
	EoDbLayer* Layer = (EoDbLayer*) m_LayersList.GetItemData(Item.iItem);
	OdDbLayerTableRecordPtr LayerTableRecord(Layer->TableRecord());

	OdString NewName(Item.pszText);
	if (!NewName.isEmpty()) {
		OdDbLayerTablePtr Layers = m_Document->LayerTable(OdDb::kForWrite);
		if (LayerTableRecord->objectId() == m_Database->getLayerZeroId()) {
			theApp.WarningMessageBox(IDS_MSG_LAYER_NO_RENAME_0);
		} else {

			if (Layers->getAt(NewName).isNull()) {
				Layer->SetName(NewName);
				if (LayerTableRecord->objectId() == m_Database->getCLAYER()) {
					m_Document->SetCurrentLayer(LayerTableRecord);
					UpdateCurrentLayerInfoField();
				}
				m_LayersList.SetItemText(Item.iItem, 0, NewName);
			}
		}
	}
	result = 0;
}
void EoDlgFileManage::OnLvnKeydownLayersListControl(LPNMHDR pNMHDR, LRESULT * result) {
	const LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	if (pLVKeyDow->wVKey == VK_DELETE) {
		const int SelectionMark = m_LayersList.GetSelectionMark();
		EoDbLayer* Layer = (EoDbLayer*) m_LayersList.GetItemData(SelectionMark);

		OdDbLayerTableRecordPtr LayerTableRecord(Layer->TableRecord());
		OdString Name(Layer->Name());
		const OdResult Result = LayerTableRecord->erase(true);
		if (Result) {
			OdString ErrorDescription = m_Database->appServices()->getErrorDescription(Result);
			ErrorDescription += L": <%s> layer can not be deleted";
			theApp.AddStringToMessageList(ErrorDescription, Name);
		} else {
			m_Document->UpdateLayerInAllViews(EoDb::kLayerErase, Layer);
			const int LayerIndex = m_Document->FindLayerAt(Name);
			m_Document->RemoveLayerAt(LayerIndex);
			m_LayersList.DeleteItem(SelectionMark);
			theApp.AddStringToMessageList(IDS_MSG_LAYER_ERASED, Name);
		}
	}
	*result = 0;
}
