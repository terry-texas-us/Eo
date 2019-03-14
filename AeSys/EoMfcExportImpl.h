#pragma once
#include "stdafx.h"
#include "EoMfcExport.h"

class AeSysDoc;

template< class T >
class MfcObjectWrapper {
public:
	MfcObjectWrapper(T* object) : 
		m_pUnderlayObj(object) {
	}
	T* operator->() {
		if ( m_pUnderlayObj == 0 )
			throw OdError( eNullObjectPointer );
		return m_pUnderlayObj;
	}
	const T* operator->() const {
		if ( m_pUnderlayObj == 0 )
			throw OdError( eNullObjectPointer );
		return m_pUnderlayObj;
	}
	const T* get() const {
		return m_pUnderlayObj;
	}
	T* get() {
		return m_pUnderlayObj;
	}
	void SetNull() {
		m_pUnderlayObj = 0;
	}

private:
	T* m_pUnderlayObj;
};

class EoApDocumentImpl : public EoApDocument {
public:
	static OdSmartPtr<EoApDocumentImpl> createObject(CDocument* document);

	virtual ~EoApDocumentImpl();
	virtual OdString fileName() const;
	virtual CDocument* cDoc() const;
	//virtual AcTransactionManager* transactionManager() const;
	virtual OdDbDatabasePtr database() const;
	virtual void lockMode(bool includeMyLocks) const;
	virtual void myLockMode() const;
	virtual bool isQuiescent() const;
	virtual void* contextPtr() const;
	virtual void ExecuteCommand(const OdString& command, bool echo);
#ifdef DEV_COMMAND_CONSOLE
	virtual OdEdBaseIO* cmdIO();
	virtual OdDbCommandContextPtr cmdCtx();
	virtual OdString recentCmd();
#endif // DEV_COMMAND_CONSOLE
	virtual OdDbSelectionSetPtr selectionSet() const;

	MfcObjectWrapper< AeSysDoc >* m_pImp;
};