#include "stdafx.h"
#include "EoZoomCmd.h"
#include <Gi/GiDrawableImpl.h>
#include <Gs/Gs.h>
#include <Gs/GsBaseVectorizer.h>
#include <GiContextForDbDatabase.h>
#include <DbLayout.h>
#include <DbCommandContext.h>
#include <DbAbstractViewportData.h>
#include <DbViewport.h>
#include <DbBlockTableRecord.h>
#include <DbViewportTable.h>
#include <DbHostAppServices.h>
#include <OdDToStr.h>
#include <ExTrackers.h>
#include "EoRtZoomTracker.h"

static bool GetLayoutExtents(const OdDbObjectId& spaceId, const OdGsView* view, OdGeBoundBlock3d& boundBox) {
	OdDbBlockTableRecordPtr Space {spaceId.safeOpenObject()};
	OdSmartPtr<OdDbLayout> Layout {Space->getLayoutId().safeOpenObject()};
	OdGeExtents3d Extents;
	if (Layout->getGeomExtents(Extents) == eOk) {
		Extents.transformBy(view->viewingMatrix());
		boundBox.set(Extents.minPoint(), Extents.maxPoint());
		return Extents.minPoint() != Extents.maxPoint();
	}
	return false;
}

void ZoomWindow(OdGePoint3d& firstCorner, OdGePoint3d& oppositeCorner, OdGsView* view) {
	const auto WorldToEyeTransform {OdAbstractViewPEPtr(view)->worldToEye(view)};
	firstCorner.transformBy(WorldToEyeTransform);
	oppositeCorner.transformBy(WorldToEyeTransform);
	auto Diagonal {oppositeCorner - firstCorner};
	if (OdNonZero(Diagonal.x) && OdNonZero(Diagonal.y)) {
		auto NewPosition {firstCorner + Diagonal / 2.0};
		Diagonal.x = fabs(Diagonal.x);
		Diagonal.y = fabs(Diagonal.y);
		view->dolly(NewPosition.asVector());
		const auto FieldWidth {view->fieldWidth() / Diagonal.x};
		const auto FieldHeight {view->fieldHeight() / Diagonal.y};
		view->zoom(odmin(FieldWidth, FieldHeight));
	}
}

void ZoomWindow2(const OdGePoint3d& pt1, const OdGePoint3d& pt2, OdGsView* pView) {
	auto pt1c {pt1};
	auto pt2c {pt2};
	ZoomWindow(pt1c, pt2c, pView);
}

void ZoomScaleXp(double /*factor*/) noexcept {
}

void ZoomExtents(OdGsView* view, OdDbObject* viewportObject) {
	const auto Database {viewportObject->database()};
	OdAbstractViewPEPtr AbstractView(view);
	OdGeBoundBlock3d BoundBox;
	auto ValidBoundBox {AbstractView->viewExtents(view, BoundBox)};
	// paper space overall view
	auto Viewport {OdDbViewport::cast(viewportObject)};
	if (Viewport.get() != nullptr && Viewport->number() == 1) {
		if (!ValidBoundBox || !(BoundBox.minPoint().x < BoundBox.maxPoint().x && BoundBox.minPoint().y < BoundBox.maxPoint().y)) {
			ValidBoundBox = GetLayoutExtents(Database->getPaperSpaceId(), view, BoundBox);
		}
	} else if (!ValidBoundBox) { // model space viewport
		ValidBoundBox = GetLayoutExtents(Database->getPaperSpaceId(), view, BoundBox);
	}
	if (!ValidBoundBox) { // set to somewhat reasonable (e.g. paper size)
		if (Database->getMEASUREMENT() == OdDb::kMetric) {
			BoundBox.set(OdGePoint3d::kOrigin, OdGePoint3d(297.0, 210.0, 0.0)); // set to paper size ISO A4 (portrait)
		} else {
			BoundBox.set(OdGePoint3d::kOrigin, OdGePoint3d(11.0, 8.5, 0.0)); // ANSI A (8.50 x 11.00) (landscape)
		}
		BoundBox.transformBy(view->viewingMatrix());
	}
	AbstractView->zoomExtents(view, &BoundBox);
}

const OdString OdExZoomCmd::groupName() const {
	return globalName();
}

const OdString OdExZoomCmd::globalName() const {
	return L"ZOOM";
}

void OdExZoomCmd::execute(OdEdCommandContext* edCommandContext) {
	OdDbCommandContextPtr CommandContext {edCommandContext};
	OdDbDatabasePtr Database {CommandContext->database()};
	OdSmartPtr<OdDbUserIO> UserIo {CommandContext->userIO()};
	const auto Keywords {L"All Center Dynamic Extents Previous Scale Window Object"};
	auto ActiveViewport {Database->activeViewportId().safeOpenObject(OdDb::kForWrite)};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
	auto ActiveView {AbstractViewportData->gsView(ActiveViewport)};
	try {
		auto FirstCorner {UserIo->getPoint(L"Specify corner of window, enter a scale factor (nX or nXP), or\n[All/Center/Dynamic/Extents/Previous/Scale/Window/Object] <real time>:", OdEd::kInpThrowEmpty | OdEd::kInpThrowOther | OdEd::kGptNoOSnap, nullptr, Keywords)};
		auto OppositeCorner {UserIo->getPoint(L"Specify opposite corner:", OdEd::kGptNoUCS | OdEd::kGptRectFrame | OdEd::kGptNoOSnap)};
		ZoomWindow(FirstCorner, OppositeCorner, ActiveView);
	} catch (const OdEdEmptyInput&) // real time
	{
		OdStaticRxObject<RTZoomTracker> Tracker;
		for (;;) {
			try {
				Tracker.Initialize(ActiveView, UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptBeginDrag | OdEd::kGptNoOSnap));
				UserIo->getPoint(L"Press ESC or ENTER to exit.", OdEd::kInpThrowEmpty | OdEd::kGptNoUCS | OdEd::kGptEndDrag | OdEd::kGptNoOSnap, nullptr, OdString::kEmpty, &Tracker);
			} catch (const OdEdCancel&) {
				break;
			}
		}
	} catch (const OdEdOtherInput& OtherInput) { // scale factor (nX or nXP)
		wchar_t* End;
		const auto ScaleFactor {odStrToD(OtherInput.string(), &End)};
		if (OdString(End).compare(OtherInput.string()) > 0) {
			if (OdString(End).iCompare(L"X") == 0) {
				ActiveView->zoom(ScaleFactor);
			} else if (OdString(End).iCompare(L"XP") == 0) {
				ZoomScaleXp(ScaleFactor);
			} else if (*End == 0U) {
				ActiveView->zoom(ScaleFactor);
			}
		}
		UserIo->putString(L"Requires a distance, numberX, or option keyword.");
	} catch (const OdEdKeyword& Keyword) {
		switch (Keyword.keywordIndex()) {
			case 0: // All
				break;
			case 1: // Center
				break;
			case 2: // Dynamic
				break;
			case 3: // Extents
				ZoomExtents(ActiveView, ActiveViewport);
				break;
			case 4: // Previous
				break;
			case 5: // Scale
				break;
			case 6: { // Window
				auto FirstCorner {UserIo->getPoint(L"Specify first corner:", OdEd::kGptNoUCS | OdEd::kGptNoOSnap)};
				auto OppositeCorner {UserIo->getPoint(L"Specify opposite corner:", OdEd::kGptNoUCS | OdEd::kGptNoOSnap | OdEd::kGptRectFrame)};
				ZoomWindow(FirstCorner, OppositeCorner, ActiveView);
				break;
			}
			case 7: // Object
				break;
			default: ;
		}
	}
	AbstractViewportData->setView(ActiveViewport, ActiveView);
}
