#include <OdaCommon.h>
#include "DbFaceGripPoints.h"
#include "OdGripPointsModule.h"

OdResult OdDbFaceGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 4);
	OdDbFacePtr Face {entity};
	for (unsigned i = 0; i < 4; i++) {
		Face->getVertexAt(i, gripPoints[GripPointsSize + i]);
	}
	return eOk;
}

OdResult OdDbFaceGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	auto Offset {offset};
	OdDbFacePtr Face {entity};
	OdGePlane Plane;
	OdDb::Planarity Planarity;
	Face->getPlane(Plane, Planarity);
	if (!ProjectOffset(Face->database(), Plane.normal(), Offset)) {
		// Project offset on entity's plane in view direction
		return eOk;
	}
	OdGePoint3d Point;
	for (auto Index : indices) {
		Face->getVertexAt(Index, Point);
		Face->setVertexAt(Index, Point + Offset);
	}
	return eOk;
}

OdResult OdDbFaceGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbFaceGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbFaceGripPointsPE::getOsnapPoints(const OdDbEntity* entity, const OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdGePoint3dArray GripPoints;
	const auto Result {getGripPoints(entity, GripPoints)};
	const auto GripPointsSize {GripPoints.size()};
	if (Result != eOk || GripPointsSize < 4) {
		return Result;
	}
	const OdDbFacePtr Face {entity};
	const auto PickPointInPlane {GetPlanePoint(Face, pickPoint)}; // recalculated pickPoint and lastPoint in plane of face
	const auto LastPointInPlane {GetPlanePoint(Face, lastPoint)};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			for (unsigned i = 0; i < GripPointsSize; i++) {
				snapPoints.append(GripPoints[i]);
			}
			break;
		case OdDb::kOsModeMid:
			for (unsigned i = 0; i < GripPointsSize; i++) {
				const auto j {i + 1 - GripPointsSize * ((i + 1) / GripPointsSize)};
				if (!GripPoints[i].isEqualTo(GripPoints[j])) {
					auto Mid {GripPoints[i] + (GripPoints[j] - GripPoints[i]) / 2.0};
					snapPoints.append(Mid);
				}
			}
			break;
		case OdDb::kOsModePerp:
			for (unsigned i = 0; i < GripPointsSize; i++) {
				const auto j {i + 1 - GripPointsSize * ((i + 1) / GripPointsSize)};
				if (!GripPoints[i].isEqualTo(GripPoints[j])) {
					OdGeLine3d Line(GripPoints[i], GripPoints[i + 1 - GripPointsSize * ((i + 1) / GripPointsSize)]);
					OdGePlane PerpendicularPlane;
					Line.getPerpPlane(LastPointInPlane, PerpendicularPlane);
					OdGePoint3d Intersection;
					if (PerpendicularPlane.intersectWith(Line, Intersection)) {
						snapPoints.append(Intersection);
					}
				}
			}
			break;
		case OdDb::kOsModeNear:
			for (unsigned i = 0; i < GripPointsSize; i++) {
				auto FirstPoint {GripPoints[i]};
				auto SecondPoint {GripPoints[i + 1 - GripPointsSize * ((i + 1) / GripPointsSize)]};
				if (!FirstPoint.isEqualTo(SecondPoint)) {
					OdGeLine3d Line(FirstPoint, SecondPoint);
					const auto p {Line.paramOf(PickPointInPlane)};
					if (p > 1) {
						snapPoints.append(SecondPoint);
					} else if (p < 0) {
						snapPoints.append(FirstPoint);
					} else {
						snapPoints.append(Line.evalPoint(p));
					}
				} else {
					snapPoints.append(FirstPoint);
				}
			}
			break;
		default:
			break;
	}
	return eOk;
}
