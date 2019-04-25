#pragma once

// From OdaMfcApp\OdaMfcExportImpl.h

#include "stdafx.h"
#include "EoMfcExport.h"

class AeSysDoc;

template< class T >
class MfcObjectWrapper {
public:
    MfcObjectWrapper(T* object) noexcept :
        m_pUnderlayObj(object) {}
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

class EoApDocumentImpl : public EoApDocument {
public:
    static OdSmartPtr<EoApDocumentImpl> createObject(CDocument* document);

    ~EoApDocumentImpl();
    OdString fileName() const override;
    CDocument* cDoc() const noexcept override;
    OdDbDatabasePtr database() const override;
    void lockMode(bool includeMyLocks) const noexcept override;
    void myLockMode() const noexcept override;
    bool isQuiescent() const noexcept override;
    void* contextPtr() const noexcept override;
    void ExecuteCommand(const OdString& command, bool echo) override;
    // <command_console>
    virtual OdEdBaseIO* cmdIO();
    virtual OdDbCommandContextPtr cmdCtx();
    virtual OdString recentCmd();
    // </command_console>
    OdDbSelectionSetPtr selectionSet() const override;

    MfcObjectWrapper< AeSysDoc >* m_pImp;
};