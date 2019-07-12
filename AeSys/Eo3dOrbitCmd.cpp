#include "stdafx.h"
#include <OdaCommon.h>
#include <Gs/Gs.h>
#include <Gs/GsBaseVectorizer.h>
#include <GiContextForDbDatabase.h>
#include <DbLayout.h>
#include <DbCommandContext.h>
#include <DbAbstractViewportData.h>
#include <DbBlockTableRecord.h>
#include <DbHostAppServices.h>
#include <RxVariantValue.h>
#include "Eo3dOrbitCmd.h"
#include "EoViewInteractivityMode.h"
#include "EoRtOrbitTracker.h"

const OdString OdEx3dOrbitCmd::groupName() const {
	return globalName();
}

const OdString OdEx3dOrbitCmd::globalName() const {
	return L"3DORBIT";
}

void OdEx3dOrbitCmd::execute(OdEdCommandContext* edCommandContext) {
	OdDbCommandContextPtr CommandContext(edCommandContext);
	OdDbDatabasePtr Database {CommandContext->database()};
	OdSmartPtr<OdDbUserIO> UserIo {CommandContext->userIO()};
	auto ActiveViewport {Database->activeViewportId().safeOpenObject(OdDb::kForWrite)};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
	auto View {AbstractViewportData->gsView(ActiveViewport)};
	{ // There is one special case: layout with enabled 'draw viewports first' mode
		if (!Database->getTILEMODE()) {
			OdSmartPtr<OdDbLayout> Layout {Database->currentLayoutId().openObject()};
			if (Layout->drawViewportsFirst()) {
				if (View->device()->viewAt(View->device()->numViews() - 1) == View) {
					View = View->device()->viewAt(0);
				}
			}
		}
	}
	const auto InteractiveMode {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"Bitmap InteractiveMode"))};
	const auto InteractiveFrameRate {static_cast<OdRxVariantValue>(edCommandContext->arbitraryData(L"Bitmap InteractiveFrameRate"))};
	ViewInteractivityMode Mode(InteractiveMode, InteractiveFrameRate, View);
	OdStaticRxObject<RtOrbitTracker> OrbitTracker;
	for (;;) {
		try {
			OrbitTracker.Initialize(View, UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptBeginDrag, nullptr, OdString::kEmpty, &OrbitTracker));
			UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptEndDrag, nullptr, OdString::kEmpty, &OrbitTracker);
			OrbitTracker.Reset();
		} catch (const OdEdCancel&) {
			break;
		}
	}
}
