#pragma once
#include <Ed/EdCommandStack.h>

struct CommandView : public OdEdCommand {
	static const OdString Name();

	[[nodiscard]] const OdString groupName() const final;

	[[nodiscard]] const OdString globalName() const final;

	void execute(OdEdCommandContext* commandContext) final;

	[[nodiscard]] const OdRxModule* commandApp() const;

	void commandUndef(bool undefIt);

	[[nodiscard]] long commandFlags() const;
};
