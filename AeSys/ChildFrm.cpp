#include "stdafx.h"
#include "ChildFrm.h"
#include "AeSysApp.h"
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

CChildFrame::CChildFrame() {
}
CChildFrame::~CChildFrame() {
}

void CChildFrame::ActivateFrame(int nCmdShow) {
	nCmdShow = SW_SHOWMAXIMIZED;
	CMDIChildWndEx::ActivateFrame(nCmdShow);
}
BOOL CChildFrame::DestroyWindow() {
	CDC* DeviceContext = GetDC();

	// Stock objects are never left "current" so it is safe to delete whatever the old object is

	DeviceContext->SelectStockObject(BLACK_PEN)->DeleteObject();
	DeviceContext->SelectStockObject(WHITE_BRUSH)->DeleteObject();

	return CMDIChildWndEx::DestroyWindow();
}
BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& createStructure) {
	if (!CMDIChildWndEx::PreCreateWindow(createStructure))
		return FALSE;

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

#ifdef ODAMFC_EXPORT
	CFrameWnd* ActivatedFrame = (CFrameWnd*) activateWnd;
	CFrameWnd* DeactivatedFrame = (CFrameWnd*) deactivateWnd;

	CDocument* ActivatedDocument = ActivatedFrame != 0 ? ActivatedFrame->GetActiveDocument() : 0;
	CDocument* DeactivatedDocument = DeactivatedFrame != 0 ? DeactivatedFrame->GetActiveDocument() : 0;

	//if (ActivatedDocument == DeactivatedDocument) {
	//	return;
	//}

	AeSysApp* TheApp = (AeSysApp*) AfxGetApp();
	const size_t NumberOfReactors = TheApp->m_aAppReactors.size();

	if (activate) {
		if (DeactivatedDocument)
			for (size_t ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
				TheApp->m_aAppReactors[ReactorIndex]->documentToBeDeactivated(DeactivatedDocument);
			}

		if (ActivatedDocument) {
			for (size_t ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
				TheApp->m_aAppReactors[ReactorIndex]->documentActivated(ActivatedDocument);
				TheApp->m_aAppReactors[ReactorIndex]->documentBecameCurrent(ActivatedDocument);
			}
		}
	}
	else {
		if (ActivatedDocument) {
			for (size_t ReactorIndex = 0; ReactorIndex < NumberOfReactors; ReactorIndex++) {
				TheApp->m_aAppReactors[ReactorIndex]->documentToBeActivated(ActivatedDocument);
			}
		}
	}
#endif // ODAMFC_EXPORT
}

static void UpdateAnnotationScalesPopupMenu(CMenu* popupMenu, OdDbDatabase* database) {
	size_t ScaleMenuPosition = popupMenu->GetMenuItemCount() - 1;
	for (; ScaleMenuPosition > 0; ScaleMenuPosition--) {
		if (CMenu* SubMenu = popupMenu->GetSubMenu(ScaleMenuPosition)) {
			SubMenu->DestroyMenu();
		}
		popupMenu->DeleteMenu(ScaleMenuPosition, MF_BYPOSITION);
	}

	OdDbObjectContextManagerPtr ContextManager(database->objectContextManager());
	const OdDbObjectContextCollection* ScalesCollection(ContextManager->contextCollection(ODDB_ANNOTATIONSCALES_COLLECTION));
	OdDbObjectContextCollectionIteratorPtr ScalesCollectionIterator = ScalesCollection->newIterator();

	ScaleMenuPosition = 1;
	OdIntPtr CurrentScaleIdentifier = database->getCANNOSCALE()->uniqueIdentifier();
	for (; !ScalesCollectionIterator->done() && ScaleMenuPosition < 100; ScalesCollectionIterator->next()) {
		OdString ScaleName = (LPWSTR) (LPCWSTR) ScalesCollectionIterator->getContext()->getName();
		OdIntPtr ScaleIdentifier = ScalesCollectionIterator->getContext()->uniqueIdentifier();

		MENUITEMINFO MenuItemInfo;
		::ZeroMemory(&MenuItemInfo, sizeof(MENUITEMINFO));
		MenuItemInfo.cbSize = sizeof(MENUITEMINFO);
		MenuItemInfo.fMask = MIIM_DATA | MIIM_ID | MIIM_STRING;
		MenuItemInfo.fType = MFT_STRING;
		MenuItemInfo.fState = MFS_ENABLED;
		MenuItemInfo.wID = ScaleMenuPosition + _APS_NEXT_COMMAND_VALUE;
		MenuItemInfo.dwItemData = (UINT) ScaleMenuPosition;
		MenuItemInfo.dwTypeData = (LPWSTR) (LPCWSTR) ScaleName;

		if (ScaleIdentifier == CurrentScaleIdentifier) {
			MenuItemInfo.fMask |= MIIM_STATE;
			MenuItemInfo.fState |= MFS_CHECKED;
		}
		popupMenu->InsertMenuItemW(ScaleMenuPosition++, &MenuItemInfo, TRUE);
	}
}
const int kViewMenuPosition(2);
const int kAnnotationScalesMenuPosition(19);

void CChildFrame::OnUpdateFrameMenu(BOOL active, CWnd* activeWindow, HMENU menuAlt) {
	CMDIChildWndEx::OnUpdateFrameMenu(active, activeWindow, menuAlt);

	CDocument* ActiveDocument = GetActiveDocument();
	if (active && ActiveDocument) {
		const CMenu* TopMenu = CMenu::FromHandle(theApp.GetAeSysMenu());
		ENSURE(TopMenu);

		CMenu* ScalesSubMenu = TopMenu->GetSubMenu(kViewMenuPosition)->GetSubMenu(kAnnotationScalesMenuPosition);

		if (ScalesSubMenu) {
			UpdateAnnotationScalesPopupMenu(ScalesSubMenu, ((AeSysDoc*) ActiveDocument)->m_DatabasePtr);
		}
	}
}
