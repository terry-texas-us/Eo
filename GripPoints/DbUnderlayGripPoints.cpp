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
	OdGePoint2dArray pnts2d;
	if (underlayReference->isClipped() && underlayReference->clipBoundary().size()) {
		//fill from clip boundary
		pnts2d.insert(pnts2d.begin(), underlayReference->clipBoundary().asArrayPtr(), underlayReference->clipBoundary().asArrayPtr() + underlayReference->clipBoundary().size());
	} else {
		//fill from extents
		OdDbUnderlayDefinitionPtr pDef = underlayReference->definitionId().openObject();
		if (pDef.isNull()) {
			return eNullEntityPointer;
		}
		auto pItem {pDef->getUnderlayItem()};
		if (pItem.isNull()) {
			return eNullEntityPointer;
		}
		pnts2d.resize(2);
		pItem->getExtents(pnts2d[0], pnts2d[1]);
	}
	if (pnts2d.size() == 2) {
		if (pnts2d[0].x > pnts2d[1].x) {
			std::swap(pnts2d[0].x, pnts2d[1].x);
		}
		if (pnts2d[0].y > pnts2d[1].y) {
			std::swap(pnts2d[0].y, pnts2d[1].y);
		}
		OdGePoint2d pt0 = pnts2d[0], pt1 = pnts2d[1];
		pnts2d.resize(4);
		pnts2d[0].set(pt0.x, pt0.y);
		pnts2d[1].set(pt0.x, pt1.y);
		pnts2d[2].set(pt1.x, pt1.y);
		pnts2d[3].set(pt1.x, pt0.y);
	}
	pnts2d.append(pnts2d[0]);
	for (OdUInt32 f = 0; f < pnts2d.size() - 1; f++) {
		OdGeLineSeg2d lSeg(pnts2d[f], pnts2d[f + 1]);
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
