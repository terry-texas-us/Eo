#include <OdaCommon.h>
#include "DbUnderlayGripPoints.h"
#include <DbUnderlayDefinition.h>
#include <Ge/GeLineSeg2d.h>
#define STL_USING_ALGORITHM
#define STL_USING_UTILITY
#include <OdaSTL.h>

OdResult OdDbUnderlayGripPointsPE::CheckBorder(const OdDbUnderlayReferencePtr& underlayReference, OdDb::OsnapMode objectSnapMode, const OdGePoint3d& pickPoint, OdGePoint3dArray& snapPoints) const {
	if (underlayReference.isNull() || objectSnapMode != OdDb::kOsModeEnd) {
		return eNotImplemented;
	}
	OdGePoint2dArray Points;
	if (underlayReference->isClipped() && underlayReference->clipBoundary().size()) { // fill from clip boundary
		Points.insert(Points.begin(), underlayReference->clipBoundary().asArrayPtr(), underlayReference->clipBoundary().asArrayPtr() + underlayReference->clipBoundary().size());
	} else { // fill from extents
		OdDbUnderlayDefinitionPtr UnderlayDefinition {underlayReference->definitionId().openObject()};
		if (UnderlayDefinition.isNull()) {
			return eNullEntityPointer;
		}
		auto UnderlayItem {UnderlayDefinition->getUnderlayItem()};
		if (UnderlayItem.isNull()) {
			return eNullEntityPointer;
		}
		Points.resize(2);
		UnderlayItem->getExtents(Points[0], Points[1]);
	}
	if (Points.size() == 2) {
		if (Points[0].x > Points[1].x) {
			std::swap(Points[0].x, Points[1].x);
		}
		if (Points[0].y > Points[1].y) {
			std::swap(Points[0].y, Points[1].y);
		}
		const auto Point0 = Points[0];
		const auto Point1 = Points[1];
		Points.resize(4);
		Points[0].set(Point0.x, Point0.y);
		Points[1].set(Point0.x, Point1.y);
		Points[2].set(Point1.x, Point1.y);
		Points[3].set(Point1.x, Point0.y);
	}
	Points.append(Points[0]);
	for (unsigned f = 0; f < Points.size() - 1; f++) {
		OdGeLineSeg2d LineSeg(Points[f], Points[f + 1]);
		OdGeTol Tolerance(LineSeg.length() * 0.05);
		if (LineSeg.isOn(pickPoint.convert2d(), Tolerance)) {
			snapPoints.append(OdGePoint3d(LineSeg.startPoint().x, LineSeg.startPoint().y, 0.));
			snapPoints.append(OdGePoint3d(LineSeg.endPoint().x, LineSeg.endPoint().y, 0.));
			return eOk;
		}
	}
	return eOk;
}

OdResult OdDbUnderlayGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	auto pRef {OdDbUnderlayReference::cast(entity)};
	if (pRef.isNull()) {
		return eNullObjectPointer;
	}
	OdDbUnderlayDefinitionPtr pDef = pRef->definitionId().openObject(OdDb::kForWrite);
	if (pDef.isNull()) {
		return eNullObjectPointer;
	}
	if (!pDef->isLoaded()) { // nothing can be rendered
		return eFileNotFound;
	}
	CheckBorder(pRef, objectSnapMode, pickPoint, snapPoints);
	return eOk;
}
