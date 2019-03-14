#pragma once

#ifdef ODAMFC_EXPORT_SYMBOL
#   define ODAMFC_EXPORT OD_TOOLKIT_EXPORT
#else
#   define ODAMFC_EXPORT OD_TOOLKIT_IMPORT
#endif // ODAMFC_EXPORT_SYMBOL

#include "DbGsManager.h"

class ODAMFC_EXPORT EoApplicationReactor : public OdRxObject {
public:
	ODRX_DECLARE_MEMBERS( EoApplicationReactor );

	// App events.
	virtual void OnBeginQuit(){}
	virtual void OnEnterModal(){}
	virtual void OnIdle( int ){}
	virtual void OnLeaveModal(){}
	virtual void OnPreTranslateMessage(MSG* message) {}
	virtual void OnQuitAborted(){}
	virtual void OnQuitWillStart(){}

	// Document events.
	virtual void documentCreateStarted(CDocument* document) {}
	virtual void documentCreated(CDocument* document) {}
	virtual void documentToBeDestroyed(CDocument* document) {}
	virtual void documentDestroyed(const OdString& document) {}
	virtual void documentCreateCanceled(CDocument* document) {}

	//virtual void documentLockModeWillChange(CDocument* document, AcAp::DocLockMode myCurrentMode, AcAp::DocLockMode myNewMode, AcAp::DocLockMode currentMode, const ACHAR* pGlobalCmdName);
	//virtual void documentLockModeChangeVetoed(CDocument* document, const ACHAR* pGlobalCmdName);
	//virtual void documentLockModeChanged(CDocument* document, AcAp::DocLockMode myPreviousMode, AcAp::DocLockMode myCurrentMode, AcAp::DocLockMode currentMode, const ACHAR* pGlobalCmdName);

	virtual void documentBecameCurrent(CDocument* document) {}
	virtual void documentToBeActivated(CDocument* document) {}
	virtual void documentToBeDeactivated(CDocument* document) {}
	virtual void documentActivationModified(bool document) {}
	virtual void documentActivated(CDocument* document) {}
};

typedef OdSmartPtr< EoApplicationReactor > EoApplicationReactorPtr;

ODAMFC_EXPORT void EoAddAppReactor(EoApplicationReactor* reactor);

class ODAMFC_EXPORT EoApDocument : public OdRxObject {
public:
	ODRX_DECLARE_MEMBERS(EoApDocument);

	virtual OdString fileName() const = 0;
	virtual CDocument* cDoc() const = 0;
	//virtual AcTransactionManager* transactionManager() const = 0;
	virtual OdDbDatabasePtr database() const = 0;
	virtual void lockMode(bool includeMyLocks) const = 0;
	virtual void myLockMode() const = 0;
	virtual bool isQuiescent() const = 0;
	virtual void* contextPtr() const = 0;
	virtual void ExecuteCommand(const OdString& command, bool echo) = 0;
#ifdef DEV_COMMAND_CONSOLE
	virtual OdEdBaseIO* cmdIO() = 0;
	virtual OdDbCommandContextPtr cmdCtx() = 0;
	virtual OdString recentCmd() = 0;
#endif // DEV_COMMAND_CONSOLE
	virtual OdDbSelectionSetPtr selectionSet() const = 0;
};
typedef OdSmartPtr< EoApDocument > EoApDocumentPtr;

ODAMFC_EXPORT EoApDocumentPtr odGetAppDocument(CDocument* document);
ODAMFC_EXPORT OdGsLayoutHelperPtr odGetDocDevice( CDocument* document);
ODAMFC_EXPORT bool odGetDocOsnapPoint(CDocument* document, OdGePoint3d& point);