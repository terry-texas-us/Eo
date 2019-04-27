// From OdaMfcApp\OdaMfcExport.cpp (last compare 19.12)

#include "stdafx.h"
#include "AeSysDoc.h"
#include "AeSysApp.h"
#include "EoMfcExportImpl.h"
#include "AeSysView.h"

#ifdef ODAMFC_EXPORT_SYMBOL

ODRX_CONS_DEFINE_MEMBERS(OdApplicationReactor, OdRxObject, RXIMPL_CONSTR);
ODRX_NO_CONS_DEFINE_MEMBERS(OdApDocument, OdRxObject);

OdSmartPtr<OdApDocumentImpl> OdApDocumentImpl::createObject(CDocument* document) {
    OdSmartPtr<OdApDocumentImpl> pRes = OdRxObjectImpl<OdApDocumentImpl>::createObject();
    pRes->m_pImp = new MfcObjectWrapper<AeSysDoc>((AeSysDoc*)document);
    return pRes;
}

OdApDocumentImpl::~OdApDocumentImpl() {
    delete m_pImp;
}

OdString OdApDocumentImpl::fileName() const {
    return (const wchar_t*)(*m_pImp)->GetPathName();
}

CDocument* OdApDocumentImpl::cDoc() const noexcept {
    return (*m_pImp).get();
}

OdDbDatabasePtr OdApDocumentImpl::database() const {
    return (*m_pImp)->m_DatabasePtr;
}

void OdApDocumentImpl::lockMode(bool includeMyLocks) const noexcept {}

void OdApDocumentImpl::myLockMode() const noexcept {}

bool OdApDocumentImpl::isQuiescent() const noexcept {
    return false;
}

void* OdApDocumentImpl::contextPtr() const noexcept {
    return 0;
}

// <command_console>
OdEdBaseIO* OdApDocumentImpl::cmdIO() {
    return (*m_pImp)->cmdIO();
}

OdDbCommandContextPtr OdApDocumentImpl::cmdCtx() {
    return (*m_pImp)->cmdCtx();
}

void OdApDocumentImpl::ExecuteCommand(const OdString& command, bool echo) {
    (*m_pImp)->ExecuteCommand(command, echo);
}

OdString OdApDocumentImpl::recentCmd() {
    return (*m_pImp)->recentCmd();
}
// </command_console>

OdDbSelectionSetPtr OdApDocumentImpl::selectionSet() const {
    return (*m_pImp)->selectionSet();
}

OdApDocumentPtr odGetAppDocument(CDocument* document) {
    return static_cast<AeSysDoc*>(document)->m_pRefDocument;
}

void OdAddAppReactor(OdApplicationReactor* reactor) {
    AeSysApp* TheApp = (AeSysApp*)AfxGetApp();
    TheApp->AddReactor(reactor);
}

OdGsLayoutHelperPtr odGetDocDevice(CDocument* document) {
    POSITION Position = document->GetFirstViewPosition();
    while (Position != 0) {
        CView* pView = document->GetNextView(Position);
        if (pView->IsKindOf(RUNTIME_CLASS(AeSysView))) {
            AeSysView* pViewer = (AeSysView*)pView;
            return pViewer->m_pDevice;
        }
    }
    return OdGsLayoutHelperPtr();
}

// <command_view>
bool odGetDocOsnapPoint(CDocument* document, OdGePoint3d& pt) noexcept {
    POSITION Position = document->GetFirstViewPosition();
    while (Position != 0) {
        CView* pView = document->GetNextView(Position);
        if (pView->IsKindOf(RUNTIME_CLASS(AeSysView))) {
            AeSysView* pViewer = (AeSysView*)pView;
            return pViewer->editorObject().snap(pt, 0);
        }
    }
    return false;
}
// </command_view>
#endif // ODAMFC_EXPORT_SYMBOL
