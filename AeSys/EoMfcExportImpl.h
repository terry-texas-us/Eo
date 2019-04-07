#pragma once
#include "stdafx.h"
#include "EoMfcExport.h"

class AeSysDoc;

template< class T >
class MfcObjectWrapper {
public:
	MfcObjectWrapper(T* object) noexcept :
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
	T* get() noexcept {
		return m_pUnderlayObj;
	}
	void SetNull() noexcept {
		m_pUnderlayObj = 0;
	}

private:
	T* m_pUnderlayObj;
};

class EoApDocumentImpl : public EoApDocument {
public:
	static OdSmartPtr<EoApDocumentImpl> createObject(CDocument* document);

	virtual ~EoApDocumentImpl();
	virtual OdString fileName() const override;
	virtual CDocument* cDoc() const noexcept override;
	//virtual AcTransactionManager* transactionManager() const;
	virtual OdDbDatabasePtr database() const override;
	virtual void lockMode(bool includeMyLocks) const noexcept override;
	virtual void myLockMode() const noexcept override;
	virtual bool isQuiescent() const noexcept override;
	virtual void* contextPtr() const noexcept override;
	virtual void ExecuteCommand(const OdString& command, bool echo) override;
#ifdef DEV_COMMAND_CONSOLE
	virtual OdEdBaseIO* cmdIO();
	virtual OdDbCommandContextPtr cmdCtx();
	virtual OdString recentCmd();
#endif // DEV_COMMAND_CONSOLE
	virtual OdDbSelectionSetPtr selectionSet() const override;

	MfcObjectWrapper< AeSysDoc >* m_pImp;
};