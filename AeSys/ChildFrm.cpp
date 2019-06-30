#include "stdafx.h"
#include "DbSymUtl.h"
#include "ChildFrm.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "DbObjectContextCollection.h"
#include "DbObjectContextManager.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
		ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

void CChildFrame::ActivateFrame(int nCmdShow) {
	nCmdShow = SW_SHOWMAXIMIZED;
	CMDIChildWndEx::ActivateFrame(nCmdShow);
}

BOOL CChildFrame::DestroyWindow() {
	auto DeviceContext {GetDC()};
	if (DeviceContext) { // Stock objects are never left "current" so it is safe to delete whatever the old object is
		DeviceContext->SelectStockObject(BLACK_PEN)->DeleteObject();
		DeviceContext->SelectStockObject(WHITE_BRUSH)->DeleteObject();
	}
	return CMDIChildWndEx::DestroyWindow();
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& createStructure) {
	if (!CMDIChildWndEx::PreCreateWindow(createStructure)) { return FALSE; }
	if (theApp.applicationOptions.tabsStyle != EoApOptions::kNone) {
		createStructure.style &= ~WS_SYSMENU;
	}
	return TRUE;
}
#ifdef _DEBUG // Diagnostic overrides
void CChildFrame::AssertValid() const {
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const {
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG
void CChildFrame::OnMDIActivate(const BOOL activate, CWnd* activateWnd, CWnd* deactivateWnd) {
	CMDIChildWndEx::OnMDIActivate(activate, activateWnd, deactivateWnd);
	auto ActivatedFrame {dynamic_cast<CFrameWnd*>(activateWnd)};
	auto DeactivatedFrame {dynamic_cast<CFrameWnd*>(deactivateWnd)};
	const auto ActivatedDocument {ActivatedFrame != nullptr ? ActivatedFrame->GetActiveDocument() : nullptr};
	const auto DeactivatedDocument {DeactivatedFrame != nullptr ? DeactivatedFrame->GetActiveDocument() : nullptr};
	const auto NumberOfReactors {theApp.applicationReactors.size()};
	if (activate) {
		if (DeactivatedDocument) {
			for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
				theApp.applicationReactors.at(ReactorIndex)->DocumentToBeDeactivated(DeactivatedDocument);
			}
		}
		if (ActivatedDocument) {
			for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
				theApp.applicationReactors.at(ReactorIndex)->DocumentActivated(ActivatedDocument);
				theApp.applicationReactors.at(ReactorIndex)->DocumentBecameCurrent(ActivatedDocument);
			}
		}
	} else {
		if (ActivatedDocument) {
			for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
				theApp.applicationReactors.at(ReactorIndex)->DocumentToBeActivated(ActivatedDocument);
			}
		}
	}
}

static void UpdateAnnotationScalesPopupMenu(CMenu* popupMenu, OdDbDatabase* database) {
	auto ScaleMenuPosition {popupMenu->GetMenuItemCount() - 1};
	for (; ScaleMenuPosition > 0; ScaleMenuPosition--) {
		if (auto SubMenu = popupMenu->GetSubMenu(ScaleMenuPosition)) {
			SubMenu->DestroyMenu();
		}
		popupMenu->DeleteMenu(static_cast<unsigned>(ScaleMenuPosition), MF_BYPOSITION);
	}
	auto ContextManager {database->objectContextManager()};
	const auto ScalesCollection {ContextManager->contextCollection(ODDB_ANNOTATIONSCALES_COLLECTION)};
	auto ScalesCollectionIterator {ScalesCollection->newIterator()};
	ScaleMenuPosition = 1;
	const auto CurrentScaleIdentifier {database->getCANNOSCALE()->uniqueIdentifier()};
	wchar_t TypeData[32];
	for (; !ScalesCollectionIterator->done() && ScaleMenuPosition < 100; ScalesCollectionIterator->next()) {
		auto ScaleName {ScalesCollectionIterator->getContext()->getName()};
		const auto ScaleIdentifier {ScalesCollectionIterator->getContext()->uniqueIdentifier()};
		MENUITEMINFO MenuItemInfo;
		::ZeroMemory(&MenuItemInfo, sizeof(MENUITEMINFO));
		MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
		MenuItemInfo.fMask = MIIM_DATA | MIIM_ID | MIIM_STRING;
		MenuItemInfo.fType = MFT_STRING;
		MenuItemInfo.fState = MFS_ENABLED;
		MenuItemInfo.wID = static_cast<unsigned>(ScaleMenuPosition + _APS_NEXT_COMMAND_VALUE);
		MenuItemInfo.dwItemData = gsl::narrow_cast<unsigned long>(ScaleMenuPosition);
		wcscpy_s(TypeData, 32, ScaleName);
		MenuItemInfo.dwTypeData = TypeData;
		if (ScaleIdentifier == CurrentScaleIdentifier) {
			MenuItemInfo.fMask |= MIIM_STATE;
			MenuItemInfo.fState |= MFS_CHECKED;
		}
		popupMenu->InsertMenuItemW(static_cast<unsigned>(ScaleMenuPosition++), &MenuItemInfo, TRUE);
	}
}

const int gc_ViewMenuPosition(2);
const int gc_AnnotationScalesMenuPosition(19);

void CChildFrame::OnUpdateFrameMenu(const BOOL active, CWnd* activeWindow, const HMENU menuAlt) {
	CMDIChildWndEx::OnUpdateFrameMenu(active, activeWindow, menuAlt);
	const auto ActiveDocument {GetActiveDocument()};
	if (active && ActiveDocument) {
		const auto TopMenu {CMenu::FromHandle(theApp.GetAeSysMenu())};
		ENSURE(TopMenu);
		const auto ScalesSubMenu {TopMenu->GetSubMenu(gc_ViewMenuPosition)->GetSubMenu(gc_AnnotationScalesMenuPosition)};
		if (ScalesSubMenu) {
			UpdateAnnotationScalesPopupMenu(ScalesSubMenu, dynamic_cast<AeSysDoc*>(ActiveDocument)->m_DatabasePtr);
		}
	}
}
