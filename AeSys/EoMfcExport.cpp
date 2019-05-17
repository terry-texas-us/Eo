// From OdaMfcApp\OdaMfcExport.cpp (last compare 19.12)

#include "stdafx.h"
#include "AeSysDoc.h"
#include "AeSysApp.h"
#include "EoMfcExportImpl.h"
#include "AeSysView.h"

ODRX_CONS_DEFINE_MEMBERS(OdApplicationReactor, OdRxObject, RXIMPL_CONSTR);
ODRX_NO_CONS_DEFINE_MEMBERS(OdApDocument, OdRxObject);

OdSmartPtr<OdApDocumentImpl> OdApDocumentImpl::createObject(CDocument* document) {
	OdSmartPtr<OdApDocumentImpl> pRes = OdRxObjectImpl<OdApDocumentImpl>::createObject();
	pRes->m_pImp = new MfcObjectWrapper<AeSysDoc>((AeSysDoc*) document);
	return pRes;
}

OdApDocumentImpl::~OdApDocumentImpl() {
	delete m_pImp;
}

OdString OdApDocumentImpl::fileName() const {
	return (const wchar_t*) (*m_pImp)->GetPathName();
}

CDocument* OdApDocumentImpl::cDoc() const noexcept {
	return (*m_pImp).get();
}

OdDbDatabasePtr OdApDocumentImpl::database() const {
	return (*m_pImp)->m_DatabasePtr;
}

void OdApDocumentImpl::lockMode(bool includeMyLocks) const noexcept {}

void OdApDocumentImpl::myLockMode() const noexcept {}

bool OdApDocumentImpl::isQuiescent() const noexcept { return false; }

void* OdApDocumentImpl::contextPtr() const noexcept { return 0; }

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

OdDbSelectionSetPtr OdApDocumentImpl::selectionSet() const {
	return (*m_pImp)->selectionSet();
}

OdApDocumentPtr odGetAppDocument(CDocument* document) {
	return static_cast<AeSysDoc*>(document)->m_pRefDocument;
}

void OdAddAppReactor(OdApplicationReactor* reactor) {
	theApp.AddReactor(reactor);
}

OdGsLayoutHelperPtr odGetDocDevice(CDocument* document) {
	auto ViewPosition {document->GetFirstViewPosition()};

	while (ViewPosition != 0) {
		auto View {document->GetNextView(ViewPosition)};

		if (View->IsKindOf(RUNTIME_CLASS(AeSysView))) {
			return ((AeSysView*) View)->m_pDevice;
		}
	}
	return OdGsLayoutHelperPtr();
}

bool odGetDocOsnapPoint(CDocument* document, OdGePoint3d& point) {
	auto ViewPosition {document->GetFirstViewPosition()};

	while (ViewPosition != 0) {

		auto View {document->GetNextView(ViewPosition)};
		
		if (View->IsKindOf(RUNTIME_CLASS(AeSysView))) {
			return ((AeSysView*)View)->editorObject().snap(point, 0);
		}
	}
	return false;
}
