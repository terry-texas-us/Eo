#include "stdafx.h"
#include "AeSysDoc.h"
#include "AeSysApp.h"
#include "EoMfcExportImpl.h"
#include "AeSysView.h"

#ifdef ODAMFC_EXPORT_SYMBOL

ODRX_CONS_DEFINE_MEMBERS(EoApplicationReactor, OdRxObject, RXIMPL_CONSTR);
ODRX_NO_CONS_DEFINE_MEMBERS(EoApDocument, OdRxObject);

OdSmartPtr<EoApDocumentImpl> EoApDocumentImpl::createObject(CDocument* document) {
	OdSmartPtr< EoApDocumentImpl > pRes = OdRxObjectImpl< EoApDocumentImpl >::createObject();
	pRes->m_pImp = new MfcObjectWrapper< AeSysDoc >((AeSysDoc*) document);
	return pRes;
}  
EoApDocumentImpl::~EoApDocumentImpl() {
	delete m_pImp;
}
OdString EoApDocumentImpl::fileName() const {
	return (const wchar_t*) (*m_pImp)->GetPathName();
}
CDocument* EoApDocumentImpl::cDoc() const {
	return (*m_pImp).get();
}
//AcTransactionManager* transactionManager() const {};
OdDbDatabasePtr EoApDocumentImpl::database() const {
	return (*m_pImp)->m_DatabasePtr;
}
void EoApDocumentImpl::lockMode(bool includeMyLocks ) const {
}
void EoApDocumentImpl::myLockMode() const {
}
bool EoApDocumentImpl::isQuiescent() const {
	return false;
}
void* EoApDocumentImpl::contextPtr() const {
	return 0;
}

#ifdef DEV_COMMAND_CONSOLE
OdEdBaseIO* EoApDocumentImpl::cmdIO() {
	return (*m_pImp)->cmdIO();
}

OdDbCommandContextPtr EoApDocumentImpl::cmdCtx() {
	return (*m_pImp)->cmdCtx();
}
#endif // DEV_COMMAND_CONSOLE

void EoApDocumentImpl::ExecuteCommand(const OdString& command, bool echo) {
	(*m_pImp)->ExecuteCommand(command, echo);
}

#ifdef DEV_COMMAND_CONSOLE
OdString EoApDocumentImpl::recentCmd() {
	return (*m_pImp)->recentCmd();
}
#endif // DEV_COMMAND_CONSOLE

OdDbSelectionSetPtr EoApDocumentImpl::selectionSet() const {
	return (*m_pImp)->selectionSet();
}

EoApDocumentPtr odGetAppDocument(CDocument* document) {
	return static_cast<AeSysDoc*>(document)->m_pRefDocument;
}
void EoAddAppReactor(EoApplicationReactor* reactor) {
	AeSysApp* TheApp = (AeSysApp*) AfxGetApp();
	TheApp->AddReactor(reactor);
}
OdGsLayoutHelperPtr odGetDocDevice(CDocument* document) {
	POSITION Position = document->GetFirstViewPosition();
	while (Position != 0) {
		CView* pView = document->GetNextView(Position);
		if (pView->IsKindOf(RUNTIME_CLASS(AeSysView))) {
			AeSysView* pViewer = (AeSysView*) pView;
			return pViewer->m_pDevice;
		}
	}
	return OdGsLayoutHelperPtr();
}
bool odGetDocOsnapPoint(CDocument* document, OdGePoint3d& pt) {
#ifdef DEV_COMMAND_VIEW
	POSITION Position = document->GetFirstViewPosition();
	while (Position != 0) {
		CView* pView = document->GetNextView(Position);
		if (pView->IsKindOf(RUNTIME_CLASS(AeSysView))) {
			AeSysView* pViewer = (AeSysView*) pView;
			return pViewer->editorObject().snap(pt, 0);
		}
	}
#endif // DEV_COMMAND_VIEW
	return false;
}
#endif // ODAMFC_EXPORT_SYMBOL
