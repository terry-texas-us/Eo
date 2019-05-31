#pragma once

#include "DbAudit.h"

class EoAppAuditInfo : public OdDbAuditInfo {
public:
	EoAppAuditInfo() noexcept;
	~EoAppAuditInfo() {
	}

public:
	void setHostAppServices(OdDbHostAppServices* hostAppServices) noexcept {
		m_pHostAppServices = hostAppServices;
	}
	void printError(const OdString& name, const OdString& value, const OdString& validation = OdString(), const OdString& defaultValue = OdString()) override;
	void printInfo(const OdString& info) override;
	const OdDbAuditInfo::MsgInfo& getLastInfo() override;
	void setLastInfo(OdDbAuditInfo::MsgInfo& messageInfo) override;
private:
	OdDbHostAppServices* m_pHostAppServices;
};
