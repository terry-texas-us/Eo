#pragma once
#include <Ed/EdCommandStack.h>

struct OdExInteractivityModeCmd : OdEdCommand {
	[[nodiscard]] const OdString groupName() const override;

	[[nodiscard]] const OdString globalName() const override;

	void execute(OdEdCommandContext* commandContext) override;
};
