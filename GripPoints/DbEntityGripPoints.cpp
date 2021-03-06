#include <OdaCommon.h>
#include "DbEntityGripPoints.h"
#include <DbEntity.h>

OdResult OdDbEntityGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdGeExtents3d Extents;
	const auto Result {entity->getGeomExtents(Extents)};
	if (Result == eOk) {
		const auto GripPointsSize {gripPoints.size()};
		gripPoints.resize(GripPointsSize + 1);
		gripPoints[GripPointsSize] = Extents.minPoint() + (Extents.maxPoint() - Extents.minPoint()) / 2.0;
	}
	return Result;
}

OdResult OdDbEntityGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	return entity->transformBy(OdGeMatrix3d::translation(offset));
}

OdResult OdDbEntityGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbEntityGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbEntityGripPointsPE::getOsnapPoints(const OdDbEntity* /*entity*/, OdDb::OsnapMode /*objectSnapMode*/, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& /*snapPoints*/) const {
	return eNotImplemented;
}
