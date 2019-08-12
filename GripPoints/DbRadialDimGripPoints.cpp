#include <OdaCommon.h>
#include <DbRadialDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include <Ge/GeLineSeg3d.h>
#include "DbRadialDimGripPoints.h"

OdResult OdDbRadialDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 3);
	OdDbRadialDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + kCenter] = Dimension->center();
	gripPoints[GripPointsSize + kChordPoint] = Dimension->chordPoint();
	gripPoints[GripPointsSize + kTextPosition] = Dimension->textPosition();
	return eOk;
}

OdResult OdDbRadialDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool /*stretch*/) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbRadialDimensionPtr Dimension {entity};
	const auto GripPoint {&gripPoints[indices[0]]};
	auto dimNewPt {*GripPoint};
	const auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
	}
	switch (indices[0]) {
		case kCenter: {
			auto ChordPoint {Dimension->chordPoint()};
			const auto Radius {Dimension->center().distanceTo(ChordPoint)};
			const auto TextPosition {Dimension->textPosition()};
			auto v {TextPosition - dimNewPt};
			v.normalize();
			ChordPoint = dimNewPt + v * Radius;
			Dimension->setChordPoint(ChordPoint);
			Dimension->setCenter(dimNewPt);
			break;
		}
		case kChordPoint: {
			const auto Center {Dimension->center()};
			auto ChordPoint {Dimension->chordPoint()};
			const auto dist {Center.distanceTo(Dimension->textPosition())};
			const auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
			auto ocsCenter {Center};
			auto ocsChordPoint {ChordPoint};
			auto ocsDimNewPt(dimNewPt);
			if (NeedTransform) {
				ocsCenter.transformBy(WorldToPlaneTransform);
				ocsChordPoint.transformBy(WorldToPlaneTransform);
				ocsDimNewPt.transformBy(WorldToPlaneTransform);
			}
			const auto SavedZCoordinate {ocsCenter.z};
			ocsCenter.z = ocsChordPoint.z = 0.0;
			ocsDimNewPt.z = 0.0;
			const auto vX {ocsCenter - ocsDimNewPt};
			auto vY {ocsCenter - ocsChordPoint};
			const auto Angle {vY.angleTo(vX, OdGeVector3d::kZAxis)};
			vY.rotateBy(Oda2PI - (OdaPI - Angle) / 2, OdGeVector3d::kZAxis);
			const OdGeLine3d Line1(ocsChordPoint, vY);
			const OdGeLine3d Line2(ocsCenter, vX);
			Line1.intersectWith(Line2, ocsChordPoint);
			dimNewPt = ocsChordPoint;
			dimNewPt.z = SavedZCoordinate;
			if (NeedTransform) {
				dimNewPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
			}
			auto v {dimNewPt - Center};
			v.normalize();
			const auto TextPosition {Center + v * dist};
			Dimension->setTextPosition(TextPosition);
			Dimension->setChordPoint(dimNewPt);
			break;
		}
		case kTextPosition: {
			const auto Center {Dimension->center()};
			auto ChordPoint {Dimension->chordPoint()};
			const auto Radius {Center.distanceTo(ChordPoint)};
			auto v {dimNewPt - Center};
			v.normalize();
			ChordPoint = Center + v * Radius;
			Dimension->setChordPoint(ChordPoint);
			Dimension->setTextPosition(dimNewPt);
			break;
		}
		default:
			break;
	}
	return eOk;
}
