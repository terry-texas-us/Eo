#include "stdafx.h"
#include <OdaCommon.h>
#include <Gs/Gs.h>
#include <Gs/GsBaseVectorizer.h>
#include <GiContextForDbDatabase.h>
#include <DbLayout.h>
#include <DbCommandContext.h>
#include <DbAbstractViewportData.h>
#include <DbViewport.h>
#include <DbBlockTableRecord.h>
#include <DbHostAppServices.h>
#include <RxVariantValue.h>
#include "EditorObject.h"
#include "EoViewInteractivityMode.h"
#include "EoRtDollyTracker.h"

const OdString OdExDollyCmd::groupName() const {
	return globalName();
}

const OdString OdExDollyCmd::globalName() const {
	return L"DOLLY";
}

void OdExDollyCmd::execute(OdEdCommandContext* edCommandContext) {
	OdDbCommandContextPtr CommandContext {edCommandContext};
	OdDbDatabasePtr Database {CommandContext->database()};
	OdSmartPtr<OdDbUserIO> UserIo {CommandContext->userIO()};
	auto ActiveViewport {Database->activeViewportId().safeOpenObject(OdDb::kForWrite)};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
	auto View {AbstractViewportData->gsView(ActiveViewport)};
	// @@@ There is one special case: layout with enabled 'draw viewports first' mode
	{
		if (!Database->getTILEMODE()) {
			OdSmartPtr<OdDbLayout> Layout {Database->currentLayoutId().openObject()};
			if (Layout->drawViewportsFirst()) {
				if (View->device()->viewAt(View->device()->numViews() - 1) == View) {
					View = View->device()->viewAt(0);
				}
			}
		}
	}
	//
	const auto InteractiveMode {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"AeSys InteractiveMode"))};
	const auto InteractiveFrameRate {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"AeSys InteractiveFrameRate"))};
	ViewInteractivityMode Mode(InteractiveMode, InteractiveFrameRate, View);
	OdStaticRxObject<RtDollyTracker> DollyTracker;
	for (;;) {
		try {
			DollyTracker.Initialize(View, UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptBeginDrag, nullptr, OdString::kEmpty, &DollyTracker));
			UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptEndDrag, nullptr, OdString::kEmpty, &DollyTracker);
			DollyTracker.Reset();
		} catch (const OdEdCancel&) {
			break;
		}
	}
}
