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
		OdDbUnderlayDefinitionPtr pDef = underlayReference->definitionId().openObject();
		if (pDef.isNull()) {
			return eNullEntityPointer;
		}
		auto pItem {pDef->getUnderlayItem()};
		if (pItem.isNull()) {
			return eNullEntityPointer;
		}
		Points.resize(2);
		pItem->getExtents(Points[0], Points[1]);
	}
	if (Points.size() == 2) {
		if (Points[0].x > Points[1].x) {
			std::swap(Points[0].x, Points[1].x);
		}
		if (Points[0].y > Points[1].y) {
			std::swap(Points[0].y, Points[1].y);
		}
		auto pt0 = Points[0];
		auto pt1 = Points[1];
		Points.resize(4);
		Points[0].set(pt0.x, pt0.y);
		Points[1].set(pt0.x, pt1.y);
		Points[2].set(pt1.x, pt1.y);
		Points[3].set(pt1.x, pt0.y);
	}
	Points.append(Points[0]);
	for (OdUInt32 f = 0; f < Points.size() - 1; f++) {
		OdGeLineSeg2d lSeg(Points[f], Points[f + 1]);
		OdGeTol tol(lSeg.length() * 0.05);
		if (lSeg.isOn(pickPoint.convert2d(), tol)) {
			snapPoints.append(OdGePoint3d(lSeg.startPoint().x, lSeg.startPoint().y, 0.));
			snapPoints.append(OdGePoint3d(lSeg.endPoint().x, lSeg.endPoint().y, 0.));
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
