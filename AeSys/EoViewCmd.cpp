#include "stdafx.h"
#include "EoViewCmd.h"
#include <DbCommandContext.h>
#include "AeSys.h"
#include "AeSysDoc.h"
#include "EoDlgNamedViews.h"

const OdString CommandView::groupName() const {
	return L"AeSys";
}

const OdString CommandView::Name() {
	return L"VIEW";
}

const OdString CommandView::globalName() const {
	return Name();
}

void CommandView::execute(OdEdCommandContext* commandContext) {
	OdDbCommandContextPtr CommandContext(commandContext);
	OdDbDatabaseDocPtr Database {CommandContext->database()};
	EoDlgNamedViews NamedViewsDialog(Database->Document(), theApp.GetMainWnd());
	if (NamedViewsDialog.DoModal() != IDOK) {
		throw OdEdCancel();
	}
}
