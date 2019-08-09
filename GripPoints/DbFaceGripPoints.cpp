#include <OdaCommon.h>
#include "DbFaceGripPoints.h"
#include "OdGripPointsModule.h"

OdResult OdDbFaceGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 4);
	OdDbFacePtr Face = entity;
	for (unsigned i = 0; i < 4; i++) {
		Face->getVertexAt(i, gripPoints[GripPointsSize + i]);
	}
	return eOk;
}

OdResult OdDbFaceGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto IndicesSize {indices.size()};
	if (IndicesSize == 0) {
		return eOk;
	}
	auto Offset {offset};
	OdDbFacePtr Face = entity;
	OdGePlane Plane;
	OdDb::Planarity Planarity;
	Face->getPlane(Plane, Planarity);
	if (!ProjectOffset(Face->database(), Plane.normal(), Offset)) {
		// Project offset on entity's plane in view direction
		return eOk;
	}
	OdGePoint3d Point;
	for (unsigned i = 0; i < IndicesSize; i++) {
		Face->getVertexAt(indices[i], Point);
		Face->setVertexAt(indices[i], Point + Offset);
	}
	return eOk;
}

OdResult OdDbFaceGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbFaceGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbFaceGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdGePoint3dArray gripPoints;
	const auto Result {getGripPoints(entity, gripPoints)};
	unsigned nSize;
	if (Result != eOk || (nSize = gripPoints.size()) < 4) {
		return Result;
	}
	const OdDbFacePtr Face = entity;
	const auto PickPointInPlane {GetPlanePoint(Face, pickPoint)}; // recalculated pickPoint and lastPoint in plane of face
	const auto LastPointInPlane {GetPlanePoint(Face, lastPoint)};
	OdGePoint3d start;
	OdGePoint3d end;
	OdGePoint3d mid;
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			for (unsigned i = 0; i < nSize; i++) {
				snapPoints.append(gripPoints[i]);
			}
			break;
		case OdDb::kOsModeMid:
			for (unsigned i = 0; i < nSize; i++) {
				const auto j {i + 1 - nSize * ((i + 1) / nSize)};
				start = gripPoints[i];
				end = gripPoints[i + 1 - nSize * ((i + 1) / nSize)];
				if (!gripPoints[i].isEqualTo(gripPoints[j])) {
					mid = gripPoints[i] + (gripPoints[j] - gripPoints[i]) / 2;
					snapPoints.append(mid);
				}
			}
			break;
		case OdDb::kOsModePerp:
			for (unsigned i = 0; i < nSize; i++) {
				const auto j {i + 1 - nSize * ((i + 1) / nSize)};
				start = gripPoints[i];
				end = gripPoints[i + 1 - nSize * ((i + 1) / nSize)];
				if (!gripPoints[i].isEqualTo(gripPoints[j])) {
					OdGeLine3d l(start, end);
					OdGePlane perpPlane;
					l.getPerpPlane(LastPointInPlane, perpPlane);
					if (perpPlane.intersectWith(l, mid)) {
						snapPoints.append(mid);
					}
				}
			}
			break;
		case OdDb::kOsModeNear:
			for (unsigned i = 0; i < nSize; i++) {
				start = gripPoints[i];
				end = gripPoints[i + 1 - nSize * ((i + 1) / nSize)];
				if (!start.isEqualTo(end)) {
					OdGeLine3d l(start, end);
					const auto p {l.paramOf(PickPointInPlane)};
					if (p > 1) {
						snapPoints.append(end);
					} else if (p < 0) {
						snapPoints.append(start);
					} else {
						snapPoints.append(l.evalPoint(p));
					}
				} else {
					snapPoints.append(start);
				}
			}
			break;
		default:
			break;
	}
	return eOk;
}
