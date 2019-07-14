#pragma once
#include <DbAudit.h>

class EoAppAuditInfo final : public OdDbAuditInfo {
public:
	EoAppAuditInfo() noexcept;

	void setHostAppServices(OdDbHostAppServices* hostAppServices) noexcept {
		m_pHostAppServices = hostAppServices;
	}

	void printError(const OdString& name, const OdString& value, const OdString& validation = OdString::kEmpty, const OdString& defaultValue = OdString::kEmpty) override;

	void printError(const OdRxObject* /*object*/, const OdString& /*value*/, const OdString& /*validation*/ = OdString::kEmpty, const OdString& /*defaultValue*/ = OdString::kEmpty) override { } // OdDbAuditInfo (to suppress C4266 warning)
	void printInfo(const OdString& info) override;

	const MsgInfo& getLastInfo() override;

	void setLastInfo(MsgInfo& messageInfo) override;

private:
	OdDbHostAppServices* m_pHostAppServices;
};
