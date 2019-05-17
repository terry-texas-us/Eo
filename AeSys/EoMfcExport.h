#pragma once

// From OdaMfcApp\OdaMfcExport.h  (last compare 19.12)

#include "DbGsManager.h"

class __declspec(dllexport) OdApplicationReactor : public OdRxObject {
public:
	ODRX_DECLARE_MEMBERS(OdApplicationReactor);

	// App events.
	virtual void OnBeginQuit() noexcept {}
	virtual void OnEnterModal() noexcept {}
	virtual void OnIdle(int) noexcept {}
	virtual void OnLeaveModal() noexcept {}
	virtual void OnPreTranslateMessage(MSG* message) noexcept {}
	virtual void OnQuitAborted() noexcept {}
	virtual void OnQuitWillStart() noexcept {}

	// Document events.
	virtual void documentCreateStarted(CDocument* document) noexcept {}
	virtual void documentCreated(CDocument* document) noexcept {}
	virtual void documentToBeDestroyed(CDocument* document) noexcept {}
	virtual void documentDestroyed(const OdString& pathName) noexcept {}
	virtual void documentCreateCanceled(CDocument* document) noexcept {}

	virtual void documentBecameCurrent(CDocument* document) noexcept {}
	virtual void documentToBeActivated(CDocument* document) noexcept {}
	virtual void documentToBeDeactivated(CDocument* document) noexcept {}
	virtual void documentActivationModified(bool modified) noexcept {}
	virtual void documentActivated(CDocument* document) noexcept {}
};

typedef OdSmartPtr< OdApplicationReactor > OdApplicationReactorPtr;

__declspec(dllexport) void OdAddAppReactor(OdApplicationReactor* reactor);

class __declspec(dllexport) OdApDocument : public OdRxObject {
public:
	ODRX_DECLARE_MEMBERS(OdApDocument);

	virtual OdString fileName() const = 0;
	virtual CDocument* cDoc() const = 0;
	virtual OdDbDatabasePtr database() const = 0;
	virtual void lockMode(bool includeMyLocks) const = 0;
	virtual void myLockMode() const = 0;
	virtual bool isQuiescent() const = 0;
	virtual void* contextPtr() const = 0;
	virtual void ExecuteCommand(const OdString& command, bool echo) = 0;
	// <command_console>
	virtual OdEdBaseIO* cmdIO() = 0;
	virtual OdDbCommandContextPtr cmdCtx() = 0;
	virtual OdString recentCmd() = 0;
	// </command_console>
	virtual OdDbSelectionSetPtr selectionSet() const = 0;
};
typedef OdSmartPtr< OdApDocument > OdApDocumentPtr;

__declspec(dllexport) OdApDocumentPtr odGetAppDocument(CDocument* document);
__declspec(dllexport) OdGsLayoutHelperPtr odGetDocDevice(CDocument* document);
__declspec(dllexport) bool odGetDocOsnapPoint(CDocument* document, OdGePoint3d& point);