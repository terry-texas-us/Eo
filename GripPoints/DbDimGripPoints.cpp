#include <OdaCommon.h>
#include "DbDimGripPoints.h"

bool OdDbDimGripPointsPE::IsAngleDirectionBetweenVectors(const OdGeVector3d firstVector, const OdGeVector3d secondVector) {
	return firstVector.angleTo(secondVector, OdGeVector3d::kZAxis) > OdaPI ? false : true;
}

OdResult OdDbDimGripPointsPE::getGripPoints(const OdDbEntity* /*entity*/, OdGePoint3dArray& /*gripPoints*/) const {
	return eOk;
}

OdResult OdDbDimGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	try {
		OdGePoint3dArray GripPoints;
		getGripPoints(entity, GripPoints);
		for (auto Index : indices) {
			if (Index < static_cast<int>(GripPoints.size())) {
				GripPoints[Index] += offset;
			}
		}
		moveGripPoint(entity, GripPoints, indices, false);
		OdDbDimensionPtr Dimension {entity};
		Dimension->recomputeDimBlock();
	} catch (const OdError& Error) {
		return Error.code();
	}
	return eOk;
}

OdResult OdDbDimGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbDimGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	try {
		OdGePoint3dArray GripPoints;
		getGripPoints(entity, GripPoints);
		for (auto Index : indices) {
			if (Index < static_cast<int>(GripPoints.size())) {
				GripPoints[Index] += offset;
			}
		}
		moveGripPoint(entity, GripPoints, indices, true);
		OdDbDimensionPtr Dimension {entity};
		Dimension->recomputeDimBlock();
	} catch (const OdError& Error) {
		return Error.code();
	}
	return eOk;
}

OdResult OdDbDimGripPointsPE::getOsnapPoints(const OdDbEntity* /*entity*/, OdDb::OsnapMode /*objectSnapMode*/, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& /*snapPoints*/) const {
	return eOk;
}
