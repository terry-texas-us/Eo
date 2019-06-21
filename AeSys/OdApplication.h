#pragma once

// From OdaMfcApp\OdaMfcExport.h  (last compare 19.12)
#include "DbGsManager.h"

class __declspec(dllexport) OdApplicationReactor : public OdRxObject {
public:
ODRX_DECLARE_MEMBERS(OdApplicationReactor);

	// App events.
	virtual void OnBeginQuit() noexcept {
	}

	virtual void OnEnterModal() noexcept {
	}

	virtual void OnIdle(int) noexcept {
	}

	virtual void OnLeaveModal() noexcept {
	}

	virtual void OnPreTranslateMessage(MSG* message) noexcept {
	}

	virtual void OnQuitAborted() noexcept {
	}

	virtual void OnQuitWillStart() noexcept {
	}

	// Document events.
	virtual void DocumentCreateStarted(CDocument* document) noexcept {
	}

	virtual void DocumentCreated(CDocument* document) noexcept {
	}

	virtual void DocumentToBeDestroyed(CDocument* document) noexcept {
	}

	virtual void DocumentDestroyed(const OdString& pathName) noexcept {
	}

	virtual void DocumentCreateCanceled(CDocument* document) noexcept {
	}

	virtual void DocumentBecameCurrent(CDocument* document) noexcept {
	}

	virtual void DocumentToBeActivated(CDocument* document) noexcept {
	}

	virtual void DocumentToBeDeactivated(CDocument* document) noexcept {
	}

	virtual void DocumentActivationModified(bool modified) noexcept {
	}

	virtual void DocumentActivated(CDocument* document) noexcept {
	}
};

using OdApplicationReactorPtr = OdSmartPtr<OdApplicationReactor>;
__declspec(dllexport) void OdAddAppReactor(OdApplicationReactor* reactor);

class __declspec(dllexport) OdApplicationDocument : public OdRxObject {
public:
ODRX_DECLARE_MEMBERS(OdApplicationDocument);
	virtual OdString fileName() const = 0;
	virtual CDocument* cDoc() const = 0;
	virtual OdDbDatabasePtr database() const = 0;
	virtual void lockMode(bool includeMyLocks) const = 0;
	virtual void myLockMode() const = 0;
	virtual bool isQuiescent() const = 0;
	virtual void* contextPtr() const = 0;
	virtual void ExecuteCommand(const OdString& command, bool echo) = 0;
	virtual OdEdBaseIO* BaseIO() = 0;
	virtual OdDbCommandContextPtr CommandContext() = 0;
	virtual OdString RecentCommand() = 0;
	virtual OdDbSelectionSetPtr SelectionSet() const = 0;
};

using OdApDocumentPtr = OdSmartPtr<OdApplicationDocument>;
__declspec(dllexport) OdApDocumentPtr odGetApplicationDocument(CDocument* document);
__declspec(dllexport) OdGsLayoutHelperPtr odGetDocDevice(CDocument* document);
__declspec(dllexport) bool odGetDocOsnapPoint(CDocument* document, OdGePoint3d& point);
