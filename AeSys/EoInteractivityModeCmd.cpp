#include "stdafx.h"
#include <RxVariantValue.h>
#include <DbUserIO.h>
#include <DbCommandContext.h>
#include "EoCollideAllCmd.h"
#include "EoInteractivityModeCmd.h"

const OdString OdExInteractivityModeCmd::groupName() const {
	return globalName();
}

const OdString OdExInteractivityModeCmd::globalName() const {
	return L"INTERACTIVITY";
}

void OdExInteractivityModeCmd::execute(OdEdCommandContext* commandContext) {
	OdDbCommandContextPtr CommandContext(commandContext);
	OdSmartPtr<OdDbUserIO> UserIo {CommandContext->userIO()};
	const auto EnableInteractivity {UserIo->getInt(L"\nSet 0 to disable or non-zero to enable Interactivity Mode: ") != 0};
	if (EnableInteractivity) {
		const auto FrameRate {UserIo->getReal(L"\nSpecify frame rate (Hz): ")};
		commandContext->setArbitraryData(L"AeSys InteractiveMode", OdRxVariantValue(true));
		commandContext->setArbitraryData(L"AeSys InteractiveFrameRate", OdRxVariantValue(FrameRate));
	} else {
		commandContext->setArbitraryData(L"AeSys InteractiveMode", OdRxVariantValue(false));
	}
}
