#include <OdaCommon.h>
#include "DbMlineGripPoints.h"
#include <DbMline.h>

OdResult OdDbMlineGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbMline* Mline = OdDbMline::cast(entity);
	const auto Size {gripPoints.size()};
	const auto NumberOfVertices {Mline->numVertices()};
	gripPoints.resize(Size + NumberOfVertices);
	auto Point {gripPoints.asArrayPtr() + Size};
	for (auto i = 0; i < NumberOfVertices; i++) {
		*Point++ = Mline->vertexAt(i);
	}
	return eOk;
}

OdResult OdDbMlineGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto Size {indices.size()};
	if (Size == 0) {
		return eOk;
	}
	OdDbMline* Mline = OdDbMline::cast(entity);
	for (unsigned i = 0; i < Size; ++i) {
		Mline->moveVertexAt(indices[i], Mline->vertexAt(indices[i]) + offset);
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
	OdRxObjectPtrArray arrExploded;
	const auto Result {entity->explode(arrExploded)};
	if (Result != eOk) {
		return Result;
	}
	for (unsigned i = 0; i < arrExploded.size(); ++i) {
		auto Entity {OdDbEntity::cast(arrExploded[i])};
		if (!Entity.isNull()) {
			Entity->getOsnapPoints(objectSnapMode, selectionMarker, pickPoint, lastPoint, worldToEyeTransform, snapPoints);
		}
	}
	return eOk;
}
