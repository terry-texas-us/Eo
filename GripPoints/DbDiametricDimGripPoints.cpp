#include <OdaCommon.h>
#include <DbDiametricDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "DbDiametricDimGripPoints.h"

OdResult OdDbDiametricDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 3);
	OdDbDiametricDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + kChordPoint] = Dimension->chordPoint();
	gripPoints[GripPointsSize + kFarChordPoint] = Dimension->farChordPoint();
	gripPoints[GripPointsSize + kTextPosition] = Dimension->textPosition();
	return eOk;
}

OdResult OdDbDiametricDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool /*stretch*/) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbDiametricDimensionPtr Dimension {entity};
	const auto ChordPoint {Dimension->chordPoint()};
	const auto FarChordPoint {Dimension->farChordPoint()};
	auto TextPosition {Dimension->textPosition()};
	const auto MidPoint {FarChordPoint + (ChordPoint - FarChordPoint) / 2.0};
	const auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
	auto ocsChordPoint {ChordPoint};
	auto ocsFarChordPoint {FarChordPoint};
	auto ocsTextPosition {TextPosition};
	auto ocsMidPoint {MidPoint};
	const auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsChordPoint.transformBy(WorldToPlaneTransform);
		ocsFarChordPoint.transformBy(WorldToPlaneTransform);
		ocsTextPosition.transformBy(WorldToPlaneTransform);
		ocsMidPoint.transformBy(WorldToPlaneTransform);
	}
	const auto SavedZCoordinate {ocsChordPoint.z};
	ocsChordPoint.z = ocsFarChordPoint.z = ocsTextPosition.z = ocsMidPoint.z = 0.0;
	const auto GripPoint = &gripPoints[indices[0]];
	auto ocsDimNewPt {*GripPoint};
	if (NeedTransform) {
		ocsDimNewPt.transformBy(WorldToPlaneTransform);
	}
	ocsDimNewPt.z = 0.0;
	const auto vX {ocsMidPoint - ocsDimNewPt};
	const auto vY {ocsMidPoint - ocsChordPoint};
	auto Angle {vY.angleTo(vX, OdGeVector3d::kZAxis)};
	if (indices[0] == kFarChordPoint) {
		Angle += OdaPI;
	}
	auto vLen {ocsFarChordPoint - ocsChordPoint};
	vLen.rotateBy(Angle, OdGeVector3d::kZAxis);
	if (indices[0] == kFarChordPoint) {
		const auto newLen {MidPoint.distanceTo(ocsDimNewPt)};
		vLen.normalize();
		vLen *= newLen;
	} else {
		vLen *= 0.5;
	}
	ocsChordPoint = MidPoint - vLen;
	ocsFarChordPoint = MidPoint + vLen;
	if (OdNonZero(Angle)) {
		ocsTextPosition.rotateBy(Angle, OdGeVector3d::kZAxis, MidPoint);
	}
	if (indices[0] == kTextPosition) {
		ocsTextPosition = ocsDimNewPt;
		Dimension->useSetTextPosition();
		if (ocsTextPosition.distanceTo(ocsFarChordPoint) < ocsTextPosition.distanceTo(ocsChordPoint)) {
			std::swap(ocsFarChordPoint, ocsChordPoint);
		}
	}
	ocsFarChordPoint.z = SavedZCoordinate;
	ocsChordPoint.z = SavedZCoordinate;
	ocsTextPosition.z = SavedZCoordinate;
	if (NeedTransform) {
		ocsFarChordPoint.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
		ocsChordPoint.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
		ocsTextPosition.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
	}
	Dimension->setFarChordPoint(ocsFarChordPoint);
	Dimension->setChordPoint(ocsChordPoint);
	Dimension->setTextPosition(ocsTextPosition);
	return eOk;
}
