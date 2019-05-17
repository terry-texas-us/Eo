#pragma once

// From OdaMfcApp\OdaMfcExportImpl.h  (last compare 19.12)

#include "stdafx.h"
#include "AeSysDoc.h"

#include "OdApplication.h"

class AeSysDoc;

template< class T >
class MfcObjectWrapper {
public:
	MfcObjectWrapper(T* object) noexcept :
		m_pUnderlayObj(object) {
	}
	T* operator->() {
		if (m_pUnderlayObj == 0)
			throw OdError(eNullObjectPointer);
		return m_pUnderlayObj;
	}
	const T* operator->() const {
		if (m_pUnderlayObj == 0)
			throw OdError(eNullObjectPointer);
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

class OdApplicationDocumentImpl : public OdApplicationDocument {
public:
	static OdSmartPtr<OdApplicationDocumentImpl> createObject(CDocument* document);

	~OdApplicationDocumentImpl();
	OdString fileName() const override;
	CDocument* cDoc() const noexcept override;
	OdDbDatabasePtr database() const override;
	void lockMode(bool includeMyLocks) const noexcept override;
	void myLockMode() const noexcept override;
	bool isQuiescent() const noexcept override;
	void* contextPtr() const noexcept override;
	void ExecuteCommand(const OdString& command, bool echo) override;

	OdEdBaseIO* cmdIO() override;
	OdDbCommandContextPtr cmdCtx() override;
	OdString recentCmd() override;

	OdDbSelectionSetPtr selectionSet() const override;

	MfcObjectWrapper<AeSysDoc>* m_pImp;
};