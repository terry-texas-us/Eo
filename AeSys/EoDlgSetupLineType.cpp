#include "stdafx.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include <DbLinetypeTable.h>
#include <DbLinetypeTableRecord.h>
#include "EoDlgSetupLinetype.h"
IMPLEMENT_DYNAMIC(EoDlgSetupLinetype, CDialog)
#pragma warning(push)
#pragma warning(disable : 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
BEGIN_MESSAGE_MAP(EoDlgSetupLinetype, CDialog)
		ON_BN_CLICKED(IDC_BYLAYER_BUTTON, &EoDlgSetupLinetype::OnBnClickedBylayerButton)
		ON_BN_CLICKED(IDC_BYBLOCK_BUTTON, &EoDlgSetupLinetype::OnBnClickedByblockButton)
		ON_WM_DRAWITEM()
END_MESSAGE_MAP()
#pragma warning (pop)
EoDlgSetupLinetype::~EoDlgSetupLinetype() = default;

void EoDlgSetupLinetype::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_LINETYPES_LIST_CONTROL, m_LinetypesListControl);
}

EoDlgSetupLinetype::EoDlgSetupLinetype(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgSetupLinetype::EoDlgSetupLinetype(OdDbLinetypeTablePtr linetypeTable, CWnd* parent)
	: CDialog(IDD, parent)
	, m_LinetypeTable(linetypeTable) {
}

void EoDlgSetupLinetype::OnBnClickedByblockButton() {
	linetype = m_LinetypeTable->getLinetypeByBlockId().safeOpenObject(OdDb::kForRead);
	CDialog::OnOK();
}

void EoDlgSetupLinetype::OnBnClickedBylayerButton() {
	linetype = m_LinetypeTable->getLinetypeByLayerId().safeOpenObject(OdDb::kForRead);
	CDialog::OnOK();
}

void EoDlgSetupLinetype::OnDrawItem(const int controlIdentifier, LPDRAWITEMSTRUCT drawItemStruct) {
	if (controlIdentifier == IDC_LINETYPES_LIST_CONTROL) {
		switch (drawItemStruct->itemAction) {
			case ODA_DRAWENTIRE: {
				CRect ItemRectangle(drawItemStruct->rcItem);
				const auto BackgroundColor {GetSysColor(drawItemStruct->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW)};
				CDC DeviceContext;
				DeviceContext.Attach(drawItemStruct->hDC);
				CBrush BackgroundBrush(BackgroundColor);
				DeviceContext.FillRect(ItemRectangle, &BackgroundBrush);
				if (drawItemStruct->itemState & ODS_FOCUS) {
					DeviceContext.DrawFocusRect(ItemRectangle);
				}
				const auto Item {static_cast<int>(drawItemStruct->itemID)};
				if (Item != -1) {
					const auto rgbText {drawItemStruct->itemState & ODS_SELECTED ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_WINDOWTEXT)};
					DeviceContext.SetBkColor(BackgroundColor);
					DeviceContext.SetTextColor(rgbText);
					const OdDbObjectId ItemData = reinterpret_cast<OdDbStub*>(static_cast<unsigned long>(m_LinetypesListControl.GetItemData(Item)));
					OdDbLinetypeTableRecordPtr Linetype = ItemData.safeOpenObject(OdDb::kForRead);
					CRect SubItemRectangle;
					m_LinetypesListControl.GetSubItemRect(Item, Name, LVIR_LABEL, SubItemRectangle);
					const auto Name {Linetype->getName()};
					DeviceContext.ExtTextOutW(SubItemRectangle.left + 6, SubItemRectangle.top + 1, ETO_CLIPPED, &SubItemRectangle, Name, static_cast<unsigned>(Name.getLength()), nullptr);
					m_LinetypesListControl.GetSubItemRect(Item, Appearance, LVIR_LABEL, SubItemRectangle);
					const auto ColorIndex {g_PrimitiveState.ColorIndex()};
					g_PrimitiveState.SetPen(nullptr, &DeviceContext, 0, static_cast<short>(EoDbLinetypeTable::LegacyLinetypeIndex(Name)));
					auto ActiveView {AeSysView::GetActiveView()};
					ActiveView->ViewportPushActive();
					ActiveView->PushViewTransform();
					ActiveView->SetViewportSize(SubItemRectangle.right + SubItemRectangle.left, SubItemRectangle.bottom + SubItemRectangle.top);
					const auto FieldWidth {(double(SubItemRectangle.right) + double(SubItemRectangle.left)) / double(DeviceContext.GetDeviceCaps(LOGPIXELSX))};
					const auto FieldHeight {(double(SubItemRectangle.bottom) + double(SubItemRectangle.top)) / double(DeviceContext.GetDeviceCaps(LOGPIXELSY))};
					ActiveView->ModelViewInitialize();
					ActiveView->SetViewWindow(0.0, 0.0, FieldWidth, FieldHeight);
					ActiveView->SetCameraTarget(OdGePoint3d::kOrigin);
					ActiveView->SetCameraPosition(OdGeVector3d::kZAxis);
					const auto FieldWidthMinimum {double(SubItemRectangle.left) / double(DeviceContext.GetDeviceCaps(LOGPIXELSX))};
					const auto FieldWidthMaximum {double(SubItemRectangle.right) / double(DeviceContext.GetDeviceCaps(LOGPIXELSX))};
					auto Line {EoGeLineSeg3d(OdGePoint3d(FieldWidthMinimum, FieldHeight / 2., 0.0), OdGePoint3d(FieldWidthMaximum, FieldHeight / 2., 0.0))};
					Line.Display(ActiveView, &DeviceContext);
					ActiveView->PopViewTransform();
					ActiveView->ViewportPopActive();
					m_LinetypesListControl.GetSubItemRect(Item, Description, LVIR_LABEL, SubItemRectangle);
					const auto Description {Linetype->comments()};
					DeviceContext.ExtTextOutW(SubItemRectangle.left + 6, SubItemRectangle.top + 1, ETO_CLIPPED, &SubItemRectangle, Description, static_cast<unsigned>(Description.getLength()), nullptr);
					g_PrimitiveState.SetColorIndex(&DeviceContext, ColorIndex);
				}
				DeviceContext.Detach();
				break;
			}
			case ODA_SELECT:
				InvertRect(drawItemStruct->hDC, &drawItemStruct->rcItem);
				break;
			case ODA_FOCUS:
				// TODO: Focus indication?
				break;
			default: ;
		}
		return;
	}
	CDialog::OnDrawItem(controlIdentifier, drawItemStruct);
}

BOOL EoDlgSetupLinetype::OnInitDialog() {
	CDialog::OnInitDialog();
	m_LinetypesListControl.DeleteAllItems();
	m_LinetypesListControl.InsertColumn(Name, L"Name", LVCFMT_LEFT, 128);
	m_LinetypesListControl.InsertColumn(Appearance, L"Appearance", LVCFMT_LEFT, 144);
	m_LinetypesListControl.InsertColumn(Description, L"Description", LVCFMT_LEFT, 128);
	OdDbDatabasePtr Database {m_LinetypeTable->database()};
	auto Iterator {m_LinetypeTable->newIterator()};
	auto ItemIndex {0};
	for (Iterator->start(); !Iterator->done(); Iterator->step()) {
		OdDbLinetypeTableRecordPtr Linetype {Iterator->getRecordId().safeOpenObject(OdDb::kForRead)};
		if (Linetype->objectId() != Database->getLinetypeByLayerId() && Linetype->objectId() != Database->getLinetypeByBlockId()) {
			m_LinetypesListControl.InsertItem(ItemIndex, nullptr);
			m_LinetypesListControl.SetItemData(ItemIndex++, reinterpret_cast<unsigned long>(static_cast<OdDbStub*>(Linetype->objectId())));
		}
	}

	// TODO: Select the current Linetype
	return TRUE;
}

void EoDlgSetupLinetype::OnOK() {
	linetype = linetype = m_LinetypeTable->getAt(L"Continuous").safeOpenObject(OdDb::kForRead);
	auto Position {m_LinetypesListControl.GetFirstSelectedItemPosition()};
	if (Position != nullptr) {
		const auto Item {m_LinetypesListControl.GetNextSelectedItem(Position)};
		const OdDbObjectId ItemData {reinterpret_cast<OdDbStub*>(static_cast<unsigned long>(m_LinetypesListControl.GetItemData(Item)))};
		linetype = ItemData.safeOpenObject(OdDb::kForRead);
	}
	CDialog::OnOK();
}
