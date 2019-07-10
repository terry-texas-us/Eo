#pragma once
#include "EoDlgPageSetup.h"
#include <DbBlockTableRecord.h>
#include <DbCommandContext.h>
#include <DbLayout.h>
#include <Ed/EdCommandStack.h>

class OdPageSetupCmd : public OdEdCommand {
public:
	const OdString groupName() const override {
		return globalName();
	}

	const OdString globalName() const override {
		return L"PageSetup";
	}

	void execute(OdEdCommandContext* commandContext) override {
		OdDbCommandContextPtr CommandContext(commandContext);
		OdDbDatabasePtr Database {CommandContext->database()};
		const OdSmartPtr<OdDbUserIO> UserIO {CommandContext->userIO()};
		Database->startTransaction();
		const auto LayoutId {OdDbBlockTableRecordPtr(Database->getActiveLayoutBTRId().safeOpenObject())->getLayoutId()};
		OdDbLayoutPtr Layout {LayoutId.safeOpenObject(OdDb::kForWrite)};
		OdDbPlotSettings* PlotSettings {Layout.get()};
		EoDlgPageSetup PageSetupDialog(*PlotSettings, UserIO);
		if (PageSetupDialog.DoModal() == IDOK) {
			Database->endTransaction();
		} else {
			Database->abortTransaction();
		}
	}
};
