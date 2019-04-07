#pragma once

#include "DbAudit.h"

class EoAppAuditInfo : public OdDbAuditInfo {
public:
	EoAppAuditInfo();
	virtual ~EoAppAuditInfo() {
	}

public:
	void setHostAppServices(OdDbHostAppServices* hostAppServices) noexcept {
		m_pHostAppServices = hostAppServices;
	}
	virtual void printError(const OdString& name, const OdString& value, const OdString& validation = OdString(), const OdString& defaultValue = OdString()) override;
	virtual void printInfo (const OdString& info) override;
	virtual const OdDbAuditInfo::MsgInfo& getLastInfo() override;
	virtual void setLastInfo(OdDbAuditInfo::MsgInfo& messageInfo);
private:
	OdDbHostAppServices* m_pHostAppServices;
};
