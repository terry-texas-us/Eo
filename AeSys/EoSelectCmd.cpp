#include "stdafx.h"
#include "EoSelectCmd.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include <DbCommandContext.h>

const OdString CommandSelect::groupName() const {
	return L"AeSys";
}

const OdString CommandSelect::Name() {
	return L"SELECT";
}

const OdString CommandSelect::globalName() const {
	return Name();
}

void CommandSelect::execute(OdEdCommandContext* commandContext) {
	OdDbCommandContextPtr CommandContext(commandContext);
	OdDbDatabaseDocPtr Database {CommandContext->database()};
	auto Document {Database->Document()};
	auto View {Document->GetViewer()};
	if (View == nullptr) { throw OdEdCancel(); }
	Document->OnEditClearSelection();
	Document->UpdateAllViews(nullptr);
	auto UserIo {CommandContext->dbUserIO()};
	UserIo->setPickfirst(nullptr);
	const auto SelectOptions {OdEd::kSelLeaveHighlighted | OdEd::kSelAllowEmpty};
	try {
		OdDbSelectionSetPtr SelectionSet {UserIo->select(L"", SelectOptions, View->EditorObject().GetWorkingSelectionSet())};
		View->EditorObject().SetWorkingSelectionSet(SelectionSet);
	} catch (const OdError&) {
		throw OdEdCancel();
	}
	View->EditorObject().SelectionSetChanged();
	Database->pageObjects();
}
