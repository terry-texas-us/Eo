#pragma once

#include "DbAudit.h"

class EoAppAuditInfo 
	: public OdDbAuditInfo {
public:
	EoAppAuditInfo() noexcept;

	void setHostAppServices(OdDbHostAppServices* hostAppServices) noexcept {
		m_pHostAppServices = hostAppServices;
	}
	void printError(const OdString& name, const OdString& value, const OdString& validation = L"", const OdString& defaultValue = L"") override;
	void printError(const OdRxObject* object, const OdString& value, const OdString& validation = L"", const OdString& defaultValue = L"") override {} // OdDbAuditInfo (to suppress C4266 warning)
	
	void printInfo(const OdString& info) override;
	const MsgInfo& getLastInfo() override;
	void setLastInfo(MsgInfo& messageInfo) override;
private:
	OdDbHostAppServices* m_pHostAppServices;
};
