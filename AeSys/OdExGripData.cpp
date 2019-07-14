// Extracted class from Examples\Editor\ExGripManager.cpp (last compare 20.5)
#include "stdafx.h"
#include <Gi/GiCommonDraw.h>
#include <Gi/GiWorldDraw.h>
#include <Gi/GiViewportDraw.h>
#include "OdExGripData.h"
#include "ExGripManager.h"

OdExGripDataPtr OdExGripData::CreateObject(OdDbStub* id, const OdDbGripDataPtr& gripData, const OdGePoint3d& point, OdBaseGripManager* gripManager) {
	auto GripData {RXIMPL_CONSTR(OdExGripData)};
	GripData->m_SubentPath.objectIds().append(id);
	GripData->m_GripData = gripData;
	GripData->m_GripManager = gripManager;
	GripData->m_Point = point;
	return GripData;
}

OdExGripDataPtr OdExGripData::CreateObject(const OdDbBaseFullSubentPath& entityPath, const OdDbGripDataPtr& gripData, const OdGePoint3d& point, OdBaseGripManager* gripManager) {
	auto GripData {RXIMPL_CONSTR(OdExGripData)};
	GripData->m_SubentPath = entityPath;
	GripData->m_GripData = gripData;
	GripData->m_GripManager = gripManager;
	GripData->m_Point = point;
	return GripData;
}

OdExGripData::~OdExGripData() {
	if (m_GripData.get() != nullptr && m_GripData->alternateBasePoint() != nullptr) {
		delete m_GripData->alternateBasePoint();
		m_GripData->setAlternateBasePoint(nullptr);
	}
}

bool OdExGripData::ComputeDragPoint(OdGePoint3d& computedPoint) const {
	auto BasePoint {Point()};
	if (GripData().get() != nullptr && GripData()->alternateBasePoint() != nullptr) {
		BasePoint = *GripData()->alternateBasePoint();
	}
	auto Override {false};
	computedPoint = BasePoint;
	if (Status() == OdDbGripOperations::kDragImageGrip && GripData().get() != nullptr && GripData()->drawAtDragImageGripPoint()) {
		computedPoint = BasePoint + (m_GripManager->m_LastPoint - m_GripManager->m_BasePoint);
		Override = true;
	}
	return Override;
}

unsigned long OdExGripData::subSetAttributes(OdGiDrawableTraits* drawableTraits) const {
	if (IsInvisible()) {
		return kDrawableIsInvisible;
	}
	auto EntityTraits = OdGiSubEntityTraits::cast(drawableTraits);
	if (EntityTraits.get() == nullptr) {
		return kDrawableNone;
	}
	switch (Status()) {
		case OdDbGripOperations::kWarmGrip:
			EntityTraits->setTrueColor(m_GripManager->m_GripColor);
			break;
		case OdDbGripOperations::kHotGrip: case OdDbGripOperations::kDragImageGrip:
			EntityTraits->setTrueColor(m_GripManager->m_GripHotColor);
			break;
		case OdDbGripOperations::kHoverGrip:
			EntityTraits->setTrueColor(m_GripManager->m_GripHoverColor);
			break;
	}
	EntityTraits->setMaterial(nullptr);
	EntityTraits->setLineWeight(OdDb::kLnWt000);
	return kDrawableRegenDraw;
}

bool OdExGripData::subWorldDraw(OdGiWorldDraw* worldDraw) const {
	auto GripSize {static_cast<double>(m_GripManager->m_GripSize)};
	if (worldDraw->context() == nullptr || worldDraw->context()->database() == nullptr) {
		GripSize = m_GripManager->m_GripSize;
	}
	// Here is the design flaw: ARX help says that grip size passed in callback below should be calculated individually for each viewport.
	if (GripData().get() != nullptr && GripData()->worldDraw() != nullptr) {
		OdGePoint3d ComputedPoint;
		OdGePoint3d* DrawAtDrag {nullptr};
		if (ComputeDragPoint(ComputedPoint)) {
			DrawAtDrag = &ComputedPoint;
		}
		OdGiDrawFlagsHelper DrawFlagsHelper(worldDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);
		return (*GripData()->worldDraw())(static_cast<OdDbGripData*>(GripData().get()), worldDraw, EntityId(), Status(), DrawAtDrag, GripSize);
	}
	return false;
}

void OdExGripData::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	OdGePoint3d ComputedPoint;
	OdGePoint3d* DrawAtDrag {nullptr};
	if (ComputeDragPoint(ComputedPoint)) {
		DrawAtDrag = &ComputedPoint;
	}
	OdGiDrawFlagsHelper DrawFlagsHelper(viewportDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);
	auto Default {true};
	if (GripData().get() != nullptr && GripData()->viewportDraw() != nullptr) {
		(*GripData()->viewportDraw())(static_cast<OdDbGripData*>(GripData().get()), viewportDraw, EntityId(), Status(), DrawAtDrag, m_GripManager->m_GripSize);
		Default = false;
	}
	if (Default) {
		OdGePoint2d PixelDensity;
		viewportDraw->viewport().getNumPixelsInUnitSquare(Point(), PixelDensity);
		OdGeVector3d GripEdgeSize(m_GripManager->m_GripSize / PixelDensity.x, 0.0, 0.0);
		GripEdgeSize.transformBy(viewportDraw->viewport().getWorldToEyeTransform());
		const auto GripSize {GripEdgeSize.length()};
		auto OnScreenPoint {ComputedPoint};
		OnScreenPoint.transformBy(viewportDraw->viewport().getWorldToEyeTransform());
		viewportDraw->subEntityTraits().setFillType(kOdGiFillAlways);
		viewportDraw->subEntityTraits().setDrawFlags(OdGiSubEntityTraits::kDrawSolidFill | OdGiSubEntityTraits::kDrawPolygonFill);
		OdGePoint3d PolygonPoints[4];
		PolygonPoints[0].set(OnScreenPoint.x - GripSize, OnScreenPoint.y - GripSize, OnScreenPoint.z);
		PolygonPoints[1].set(OnScreenPoint.x + GripSize, OnScreenPoint.y - GripSize, OnScreenPoint.z);
		PolygonPoints[2].set(OnScreenPoint.x + GripSize, OnScreenPoint.y + GripSize, OnScreenPoint.z);
		PolygonPoints[3].set(OnScreenPoint.x - GripSize, OnScreenPoint.y + GripSize, OnScreenPoint.z);
		viewportDraw->geometry().polygonEye(4, PolygonPoints);
	}
}
