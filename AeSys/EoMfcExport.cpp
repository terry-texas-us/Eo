// From OdaMfcApp\OdaMfcExport.cpp

#include "stdafx.h"
#include "AeSysDoc.h"
#include "AeSysApp.h"
#include "EoMfcExportImpl.h"
#include "AeSysView.h"

#ifdef ODAMFC_EXPORT_SYMBOL

ODRX_CONS_DEFINE_MEMBERS(EoApplicationReactor, OdRxObject, RXIMPL_CONSTR);
ODRX_NO_CONS_DEFINE_MEMBERS(EoApDocument, OdRxObject);

OdSmartPtr<EoApDocumentImpl> EoApDocumentImpl::createObject(CDocument* document) {
    OdSmartPtr<EoApDocumentImpl> pRes = OdRxObjectImpl<EoApDocumentImpl>::createObject();
    pRes->m_pImp = new MfcObjectWrapper<AeSysDoc>((AeSysDoc*)document);
    return pRes;
}

EoApDocumentImpl::~EoApDocumentImpl() {
    delete m_pImp;
}

OdString EoApDocumentImpl::fileName() const {
    return (const wchar_t*)(*m_pImp)->GetPathName();
}

CDocument* EoApDocumentImpl::cDoc() const noexcept {
    return (*m_pImp).get();
}

OdDbDatabasePtr EoApDocumentImpl::database() const {
    return (*m_pImp)->m_DatabasePtr;
}

void EoApDocumentImpl::lockMode(bool includeMyLocks) const noexcept {}

void EoApDocumentImpl::myLockMode() const noexcept {}

bool EoApDocumentImpl::isQuiescent() const noexcept {
    return false;
}

void* EoApDocumentImpl::contextPtr() const noexcept {
    return 0;
}

// <command_console>
OdEdBaseIO* EoApDocumentImpl::cmdIO() {
    return (*m_pImp)->cmdIO();
}

OdDbCommandContextPtr EoApDocumentImpl::cmdCtx() {
    return (*m_pImp)->cmdCtx();
}

void EoApDocumentImpl::ExecuteCommand(const OdString& command, bool echo) {
    (*m_pImp)->ExecuteCommand(command, echo);
}

OdString EoApDocumentImpl::recentCmd() {
    return (*m_pImp)->recentCmd();
}
// </command_console>

OdDbSelectionSetPtr EoApDocumentImpl::selectionSet() const {
    return (*m_pImp)->selectionSet();
}

EoApDocumentPtr odGetAppDocument(CDocument* document) {
    return static_cast<AeSysDoc*>(document)->m_pRefDocument;
}

void EoAddAppReactor(EoApplicationReactor* reactor) {
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
