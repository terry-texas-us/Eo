// From OdaMfcApp\OdaMfcExport.cpp (last compare 19.12)

#include "stdafx.h"
#include "AeSysDoc.h"
#include "AeSysApp.h"
#include "AeSysView.h"
#include "OdApplicationImpl.h"

ODRX_CONS_DEFINE_MEMBERS(OdApplicationReactor, OdRxObject, RXIMPL_CONSTR);
ODRX_NO_CONS_DEFINE_MEMBERS(OdApplicationDocument, OdRxObject);

OdSmartPtr<OdApplicationDocumentImpl> OdApplicationDocumentImpl::createObject(CDocument* document) {
	OdSmartPtr<OdApplicationDocumentImpl> pRes = OdRxObjectImpl<OdApplicationDocumentImpl>::createObject();
	pRes->m_pImp = new MfcObjectWrapper<AeSysDoc>((AeSysDoc*) document);
	return pRes;
}

OdApplicationDocumentImpl::~OdApplicationDocumentImpl() {
	delete m_pImp;
}

OdString OdApplicationDocumentImpl::fileName() const {
	return (const wchar_t*) (*m_pImp)->GetPathName();
}

CDocument* OdApplicationDocumentImpl::cDoc() const noexcept {
	return (*m_pImp).get();
}

OdDbDatabasePtr OdApplicationDocumentImpl::database() const {
	return (*m_pImp)->m_DatabasePtr;
}

void OdApplicationDocumentImpl::lockMode(bool includeMyLocks) const noexcept {}

void OdApplicationDocumentImpl::myLockMode() const noexcept {}

bool OdApplicationDocumentImpl::isQuiescent() const noexcept { return false; }

void* OdApplicationDocumentImpl::contextPtr() const noexcept { return 0; }

OdEdBaseIO* OdApplicationDocumentImpl::BaseIO() {
	return (*m_pImp)->BaseIO();
}

OdDbCommandContextPtr OdApplicationDocumentImpl::CommandContext() {
	return (*m_pImp)->CommandContext();
}

void OdApplicationDocumentImpl::ExecuteCommand(const OdString& command, bool echo) {
	(*m_pImp)->ExecuteCommand(command, echo);
}

OdString OdApplicationDocumentImpl::RecentCommand() {
	return (*m_pImp)->RecentCommand();
}

OdDbSelectionSetPtr OdApplicationDocumentImpl::selectionSet() const {
	return (*m_pImp)->selectionSet();
}

OdApDocumentPtr odGetApplicationDocument(CDocument* document) {
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
			return ((AeSysView*) View)->m_LayoutHelper;
		}
	}
	return OdGsLayoutHelperPtr();
}

bool odGetDocOsnapPoint(CDocument* document, OdGePoint3d& point) {
	auto ViewPosition {document->GetFirstViewPosition()};

	while (ViewPosition != 0) {

		auto View {document->GetNextView(ViewPosition)};
		
		if (View->IsKindOf(RUNTIME_CLASS(AeSysView))) {
			return ((AeSysView*)View)->editorObject().Snap(point, 0);
		}
	}
	return false;
}
