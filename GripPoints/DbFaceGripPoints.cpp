#include <OdaCommon.h>
#include "DbFaceGripPoints.h"
#include "OdGripPointsModule.h"

OdResult OdDbFaceGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto Size {gripPoints.size()};
	gripPoints.resize(Size + 4);
	OdDbFacePtr Face = entity;
	for (unsigned i = 0; i < 4; i++) {
		Face->getVertexAt(i, gripPoints[Size + i]);
	}
	return eOk;
}

OdResult OdDbFaceGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& vOffset) {
	const auto Size {indices.size()};
	if (Size == 0) {
		return eOk;
	}
	auto offset {vOffset};
	OdDbFacePtr pFace = entity;
	OdGePlane plane;
	OdDb::Planarity planarity;
	pFace->getPlane(plane, planarity);
	if (!projectOffset(pFace->database(), plane.normal(), offset)) {
		// Project offset on entity's plane in view direction
		return eOk;
	}
	OdGePoint3d point;
	for (unsigned i = 0; i < Size; i++) {
		pFace->getVertexAt(indices[i], point);
		pFace->setVertexAt(indices[i], point + offset);
	}
	return eOk;
}

OdResult OdDbFaceGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbFaceGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbFaceGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& pickPoint_, const OdGePoint3d& lastPoint_, const OdGeMatrix3d& /*xWorldToEye*/, OdGePoint3dArray& snapPoints) const {
	OdGePoint3dArray gripPoints;
	const auto Result {getGripPoints(entity, gripPoints)};
	unsigned nSize;
	if (Result != eOk || (nSize = gripPoints.size()) < 4) {
		return Result;
	}
	const OdDbFacePtr Face = entity;
	const auto pickPoint {getPlanePoint(Face, pickPoint_)}; // recalculated pickPoint and lastPoint in plane of face
	const auto lastPoint {getPlanePoint(Face, lastPoint_)};
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
					l.getPerpPlane(lastPoint, perpPlane);
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
					const auto p {l.paramOf(pickPoint)};
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
