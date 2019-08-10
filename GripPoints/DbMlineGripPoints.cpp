#include <OdaCommon.h>
#include "DbMlineGripPoints.h"
#include <DbMline.h>

OdResult OdDbMlineGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbMline* Mline {OdDbMline::cast(entity)};
	const auto GripPointsSize {gripPoints.size()};
	const auto NumberOfVertices {Mline->numVertices()};
	gripPoints.resize(GripPointsSize + NumberOfVertices);
	auto Point {gripPoints.asArrayPtr() + GripPointsSize};
	for (auto i = 0; i < NumberOfVertices; i++) {
		*Point++ = Mline->vertexAt(i);
	}
	return eOk;
}

OdResult OdDbMlineGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbMline* Mline {OdDbMline::cast(entity)};
	for (auto Index : indices) {
		Mline->moveVertexAt(Index, Mline->vertexAt(Index) + offset);
	}
	return eOk;
}

OdResult OdDbMlineGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbMlineGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbMlineGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const {
	OdRxObjectPtrArray ExplodedObjects;
	const auto Result {entity->explode(ExplodedObjects)};
	if (Result != eOk) {
		return Result;
	}
	for (auto& ExplodedObject : ExplodedObjects) {
		auto Entity {OdDbEntity::cast(ExplodedObject)};
		if (!Entity.isNull()) {
			Entity->getOsnapPoints(objectSnapMode, selectionMarker, pickPoint, lastPoint, worldToEyeTransform, snapPoints);
		}
	}
	return eOk;
}
