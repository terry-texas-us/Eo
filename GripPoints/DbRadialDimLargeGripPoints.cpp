#include <OdaCommon.h>
#include <DbRadialDimensionLarge.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "DbRadialDimLargeGripPoints.h"

OdResult OdDbRadialDimLargeGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 4);
	OdDbRadialDimensionLargePtr Dimension {entity};
	gripPoints[GripPointsSize + kChordPoint] = Dimension->chordPoint();
	gripPoints[GripPointsSize + kOverrideCenter] = Dimension->overrideCenter();
	gripPoints[GripPointsSize + kJogPoint] = Dimension->jogPoint();
	gripPoints[GripPointsSize + kTextPosition] = Dimension->textPosition();
	return eOk;
}

OdResult OdDbRadialDimLargeGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool stretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbRadialDimensionLargePtr Dimension {entity};
	auto ChordPoint {Dimension->chordPoint()};
	auto OverrideCenter {Dimension->overrideCenter()};
	auto JogPoint {Dimension->jogPoint()};
	auto TextPosition {Dimension->textPosition()};
	auto Center {Dimension->center()};
	auto JogAngle {Dimension->jogAngle()};
	auto Normal {Dimension->normal()};
	auto NeedTransform {Normal != OdGeVector3d::kZAxis};
	auto WorldToPlaneTransform(OdGeMatrix3d::worldToPlane(Normal));
	auto PlaneToWorldTransform(OdGeMatrix3d::planeToWorld(Normal));
	auto ocsChordPoint {ChordPoint};
	auto ocsOverrideCenter {OverrideCenter};
	auto ocsJogPoint {JogPoint};
	auto ocsTextPosition {TextPosition};
	auto ocsCenter {Center};
	auto GripPoint {&gripPoints[indices[0]]};
	auto ocsGripPoint {*GripPoint};
	if (NeedTransform) {
		ocsChordPoint.transformBy(WorldToPlaneTransform);
		ocsOverrideCenter.transformBy(WorldToPlaneTransform);
		ocsJogPoint.transformBy(WorldToPlaneTransform);
		ocsTextPosition.transformBy(WorldToPlaneTransform);
		ocsCenter.transformBy(WorldToPlaneTransform);
		ocsGripPoint.transformBy(WorldToPlaneTransform);
	}
	auto SavedZCoordinate {ocsChordPoint.z};
	ocsChordPoint.z = ocsOverrideCenter.z = ocsJogPoint.z = ocsTextPosition.z = ocsCenter.z = 0.0;
	ocsGripPoint.z = 0.0;
	if (indices[0] == kChordPoint) {
		auto Radius {fabs(OdGeVector3d(ocsCenter - ocsChordPoint).length())};
		auto RadiusVector {ocsCenter - ocsChordPoint};
		RadiusVector = ocsCenter - ocsGripPoint;
		RadiusVector.normalize();
		RadiusVector *= Radius;
		ocsGripPoint = ocsCenter - RadiusVector;
		ocsChordPoint = ocsGripPoint;
		ocsGripPoint.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsGripPoint.transformBy(PlaneToWorldTransform);
		}
		Dimension->setChordPoint(ocsGripPoint);
		RadiusVector.normalize();
		RadiusVector *= ocsTextPosition.distanceTo(ocsCenter);
		ocsTextPosition = ocsCenter - RadiusVector;
		ocsTextPosition.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsTextPosition.transformBy(PlaneToWorldTransform);
		}
		Dimension->setTextPosition(ocsTextPosition);
	}
	if (indices[0] == kOverrideCenter) {
		ocsOverrideCenter = ocsGripPoint;
		if (NeedTransform) {
			ocsGripPoint.transformBy(PlaneToWorldTransform);
		}
		Dimension->setOverrideCenter(ocsGripPoint);
	}
	if (indices[0] == kJogPoint) {
		auto vR {ocsCenter - ocsChordPoint};
		auto vX1 {vR};
		vX1.rotateBy(JogAngle, OdGeVector3d::kZAxis);
		OdGeLine3d Line1(ocsCenter, vR);
		OdGeLine3d Line2(ocsGripPoint, vX1);
		OdGePoint3d IntersectPoint;
		Line1.intersectWith(Line2, IntersectPoint);
		Line1.set(ocsOverrideCenter, vR);
		OdGePoint3d Intersect2Point;
		Line1.intersectWith(Line2, Intersect2Point);
		auto dimLen {fabs(OdGeVector3d(IntersectPoint - Intersect2Point).length())};
		auto vR2 {IntersectPoint - Intersect2Point};
		vR2.normalize();
		vR2 *= dimLen / 2.0;
		ocsJogPoint = IntersectPoint - vR2;
		ocsJogPoint.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsJogPoint.transformBy(PlaneToWorldTransform);
		}
		Dimension->setJogPoint(ocsJogPoint);
	}
	if (indices[0] == kTextPosition) {
		auto dimLRadius {fabs(OdGeVector3d(ocsCenter - ocsChordPoint).length())};
		auto vR {ocsCenter - ocsChordPoint};
		vR = ocsCenter - ocsGripPoint;
		vR.normalize();
		vR *= dimLRadius;
		ocsChordPoint = ocsCenter - vR;
		ocsChordPoint.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsChordPoint.transformBy(PlaneToWorldTransform);
		}
		Dimension->setChordPoint(ocsChordPoint);
		ocsTextPosition = ocsGripPoint;
		ocsTextPosition.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsTextPosition.transformBy(PlaneToWorldTransform);
		}
		Dimension->setTextPosition(ocsTextPosition); // correct jog point
		if (indices.size() == 1 || !stretch) {
			Dimension->useSetTextPosition();
		}
	}
	if (indices[0] != kJogPoint) { // correct jog point
		auto dimLen {fabs(OdGeVector3d(ocsChordPoint - ocsOverrideCenter).length())};
		auto vR2 {ocsChordPoint - ocsOverrideCenter};
		vR2.normalize();
		vR2 *= dimLen / 2.0;
		ocsJogPoint = ocsChordPoint - vR2;
		ocsJogPoint.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsJogPoint.transformBy(PlaneToWorldTransform);
		}
		Dimension->setJogPoint(ocsJogPoint);
	}
	return eOk;
}
