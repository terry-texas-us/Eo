// From OdaMfcApp\OdaMfcExport.cpp (last compare 20.5)
#include "stdafx.h"
#include "AeSysDoc.h"
#include "AeSys.h"
#include "AeSysView.h"
#include "OdApplicationImpl.h"
ODRX_CONS_DEFINE_MEMBERS(OdApplicationReactor, OdRxObject, RXIMPL_CONSTR);

ODRX_NO_CONS_DEFINE_MEMBERS(OdApplicationDocument, OdRxObject);

OdSmartPtr<OdApplicationDocumentImpl> OdApplicationDocumentImpl::createObject(CDocument* document) {
	OdSmartPtr<OdApplicationDocumentImpl> pRes = OdRxObjectImpl<OdApplicationDocumentImpl>::createObject();
	pRes->m_pImp = new MfcObjectWrapper<AeSysDoc>(dynamic_cast<AeSysDoc*>(document));
	return pRes;
}

OdApplicationDocumentImpl::~OdApplicationDocumentImpl() {
	delete m_pImp;
}

OdString OdApplicationDocumentImpl::fileName() const {
	return static_cast<const wchar_t*>((*m_pImp)->GetPathName());
}

CDocument* OdApplicationDocumentImpl::cDoc() const noexcept {
	return (*m_pImp).get();
}

OdDbDatabasePtr OdApplicationDocumentImpl::database() const {
	return (*m_pImp)->m_DatabasePtr;
}

void OdApplicationDocumentImpl::lockMode(bool includeMyLocks) const noexcept {
}

void OdApplicationDocumentImpl::myLockMode() const noexcept {
}

bool OdApplicationDocumentImpl::isQuiescent() const noexcept { return false; }

void* OdApplicationDocumentImpl::contextPtr() const noexcept { return nullptr; }

OdEdBaseIO* OdApplicationDocumentImpl::BaseIO() {
	return (*m_pImp)->BaseIo();
}

OdDbCommandContextPtr OdApplicationDocumentImpl::CommandContext() {
	return (*m_pImp)->CommandContext0();
}

void OdApplicationDocumentImpl::ExecuteCommand(const OdString& command, const bool echo) {
	(*m_pImp)->ExecuteCommand(command, echo);
}

OdString OdApplicationDocumentImpl::RecentCommand() {
	return (*m_pImp)->RecentCommand();
}

OdDbSelectionSetPtr OdApplicationDocumentImpl::SelectionSet() const {
	return (*m_pImp)->SelectionSet();
}

OdApDocumentPtr odGetApplicationDocument(CDocument* document) {
	return dynamic_cast<AeSysDoc*>(document)->m_pRefDocument;
}

void OdAddAppReactor(OdApplicationReactor* reactor) {
	theApp.AddReactor(reactor);
}

OdGsLayoutHelperPtr odGetDocDevice(CDocument* document) {
	auto ViewPosition {document->GetFirstViewPosition()};
	while (ViewPosition != nullptr) {
		const auto View {document->GetNextView(ViewPosition)};
		if (View->IsKindOf(RUNTIME_CLASS(AeSysView))) {
			return dynamic_cast<AeSysView*>(View)->m_LayoutHelper;
		}
	}
	return OdGsLayoutHelperPtr();
}

bool odGetDocOsnapPoint(CDocument* document, OdGePoint3d& point) {
	auto ViewPosition {document->GetFirstViewPosition()};
	while (ViewPosition != nullptr) {
		const auto View {document->GetNextView(ViewPosition)};
		if (View->IsKindOf(RUNTIME_CLASS(AeSysView))) {
			return dynamic_cast<AeSysView*>(View)->EditorObject().Snap(point, nullptr);
		}
	}
	return false;
}
