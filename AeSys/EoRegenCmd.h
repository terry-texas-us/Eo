#pragma once
#include <Ed/EdCommandStack.h>
#include <DbGsManager.h>
#include "AeSysView.h"

struct OdExRegenCmd : OdEdCommand {
	OdGsLayoutHelper* m_LayoutHelper {nullptr};
	AeSysView* m_View {nullptr};

	const OdString groupName() const override { return L"REGEN"; }

	const OdString globalName() const override { return L"REGEN"; }

	[[nodiscard]] long flags() const override {
		return OdEdCommand::flags() | kNoUndoMarker;
	}

	void execute(OdEdCommandContext* /*edCommandContext*/) noexcept override {
		// <tas="placeholder until implemented" m_View->OnViewerRegen();"</tas>
	}
};
