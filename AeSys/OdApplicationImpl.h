#pragma once

// From OdaMfcApp\OdaMfcExportImpl.h  (last compare 19.12)
#include "stdafx.h"
#include "OdApplication.h"
class AeSysDoc;

template <class T>
class MfcObjectWrapper {
public:
	MfcObjectWrapper(T* object) noexcept
		: m_pUnderlayObj(object) {
	}

	T* operator->() {
		if (m_pUnderlayObj == nullptr) throw OdError(eNullObjectPointer);
		return m_pUnderlayObj;
	}

	const T* operator->() const {
		if (m_pUnderlayObj == nullptr) throw OdError(eNullObjectPointer);
		return m_pUnderlayObj;
	}

	const T* get() const {
		return m_pUnderlayObj;
	}

	T* get() noexcept {
		return m_pUnderlayObj;
	}

	void SetNull() noexcept {
		m_pUnderlayObj = nullptr;
	}

private:
	T* m_pUnderlayObj;
};

class OdApplicationDocumentImpl : public OdApplicationDocument {
public:
	static OdSmartPtr<OdApplicationDocumentImpl> createObject(CDocument* document);
	~OdApplicationDocumentImpl();
	[[nodiscard]] OdString fileName() const override;
	[[nodiscard]] CDocument* cDoc() const noexcept override;
	[[nodiscard]] OdDbDatabasePtr database() const override;
	void lockMode(bool includeMyLocks) const noexcept override;
	void myLockMode() const noexcept override;
	[[nodiscard]] bool isQuiescent() const noexcept override;
	[[nodiscard]] void* contextPtr() const noexcept override;
	void ExecuteCommand(const OdString& command, bool echo) override;
	OdEdBaseIO* BaseIO() override;
	OdDbCommandContextPtr CommandContext() override;
	OdString RecentCommand() override;
	[[nodiscard]] OdDbSelectionSetPtr SelectionSet() const override;
	MfcObjectWrapper<AeSysDoc>* m_pImp;
};
