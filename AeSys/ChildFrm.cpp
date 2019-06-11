#include "stdafx.h"

#include "DbSymUtl.h"

#include "ChildFrm.h"
#include "AeSys.h"
#include "AeSysDoc.h"

#include "DbObjectContextCollection.h"
#include "DbObjectContextManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

CChildFrame::CChildFrame() noexcept {
}

CChildFrame::~CChildFrame() {
}

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

	if (theApp.m_Options.m_nTabsStyle != EoApOptions::None) {
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

void CChildFrame::OnMDIActivate(BOOL activate, CWnd* activateWnd, CWnd* deactivateWnd) {
	CMDIChildWndEx::OnMDIActivate(activate, activateWnd, deactivateWnd);

	auto ActivatedFrame {dynamic_cast<CFrameWnd*>(activateWnd)};
	auto DeactivatedFrame {dynamic_cast<CFrameWnd*>(deactivateWnd)};

	CDocument* ActivatedDocument {ActivatedFrame != nullptr ? ActivatedFrame->GetActiveDocument() : nullptr};
	CDocument* DeactivatedDocument {DeactivatedFrame != nullptr ? DeactivatedFrame->GetActiveDocument() : nullptr};

	const auto NumberOfReactors {theApp.m_ApplicationReactors.size()};

	if (activate) {
		if (DeactivatedDocument) {
			for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
				theApp.m_ApplicationReactors.at(ReactorIndex)->DocumentToBeDeactivated(DeactivatedDocument);
			}
		}
		if (ActivatedDocument) {
			for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
				theApp.m_ApplicationReactors.at(ReactorIndex)->DocumentActivated(ActivatedDocument);
				theApp.m_ApplicationReactors.at(ReactorIndex)->DocumentBecameCurrent(ActivatedDocument);
			}
		}
	} else {
		if (ActivatedDocument) {
			for (unsigned ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
				theApp.m_ApplicationReactors.at(ReactorIndex)->DocumentToBeActivated(ActivatedDocument);
			}
		}
	}
}

static void UpdateAnnotationScalesPopupMenu(CMenu* popupMenu, OdDbDatabase* database) {
	auto ScaleMenuPosition {popupMenu->GetMenuItemCount() - 1};
	
	for (; ScaleMenuPosition > 0; ScaleMenuPosition--) {
		if (CMenu* SubMenu = popupMenu->GetSubMenu(ScaleMenuPosition)) {
			SubMenu->DestroyMenu();
		}
		popupMenu->DeleteMenu(static_cast<unsigned>(ScaleMenuPosition), MF_BYPOSITION);
	}
	auto ContextManager {database->objectContextManager()};
	const auto ScalesCollection {ContextManager->contextCollection(ODDB_ANNOTATIONSCALES_COLLECTION)};
	auto ScalesCollectionIterator {ScalesCollection->newIterator()};

	ScaleMenuPosition = 1;
	auto CurrentScaleIdentifier {database->getCANNOSCALE()->uniqueIdentifier()};

	for (; !ScalesCollectionIterator->done() && ScaleMenuPosition < 100; ScalesCollectionIterator->next()) {
		auto ScaleName {ScalesCollectionIterator->getContext()->getName()};
		auto ScaleIdentifier {ScalesCollectionIterator->getContext()->uniqueIdentifier()};
		
		MENUITEMINFO MenuItemInfo;
		::ZeroMemory(&MenuItemInfo, sizeof(MENUITEMINFO));
		MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
		MenuItemInfo.fMask = MIIM_DATA | MIIM_ID | MIIM_STRING;
		MenuItemInfo.fType = MFT_STRING;
		MenuItemInfo.fState = MFS_ENABLED;
		MenuItemInfo.wID = static_cast<unsigned>(ScaleMenuPosition + _APS_NEXT_COMMAND_VALUE);
		MenuItemInfo.dwItemData = gsl::narrow_cast<unsigned long>(ScaleMenuPosition);
		MenuItemInfo.dwTypeData = ScaleName.getBuffer(ScaleName.getLength());
		ScaleName.releaseBuffer();

		if (ScaleIdentifier == CurrentScaleIdentifier) {
			MenuItemInfo.fMask |= MIIM_STATE;
			MenuItemInfo.fState |= MFS_CHECKED;
		}
		popupMenu->InsertMenuItemW(static_cast<unsigned>(ScaleMenuPosition++), &MenuItemInfo, TRUE);
	}
}
const int kViewMenuPosition(2);
const int kAnnotationScalesMenuPosition(19);

void CChildFrame::OnUpdateFrameMenu(BOOL active, CWnd* activeWindow, HMENU menuAlt) {
	CMDIChildWndEx::OnUpdateFrameMenu(active, activeWindow, menuAlt);

	auto ActiveDocument {GetActiveDocument()};
	
	if (active && ActiveDocument) {
		const auto TopMenu {CMenu::FromHandle(theApp.GetAeSysMenu())};
		ENSURE(TopMenu);

		auto ScalesSubMenu {TopMenu->GetSubMenu(kViewMenuPosition)->GetSubMenu(kAnnotationScalesMenuPosition)};

		if (ScalesSubMenu) {
			UpdateAnnotationScalesPopupMenu(ScalesSubMenu, dynamic_cast<AeSysDoc*>(ActiveDocument)->m_DatabasePtr);
		}
	}
}
