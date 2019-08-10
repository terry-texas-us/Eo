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
	gripPoints[GripPointsSize + 0] = Dimension->center();
	gripPoints[GripPointsSize + 1] = Dimension->chordPoint();
	gripPoints[GripPointsSize + 2] = Dimension->textPosition();
	return eOk;
}

OdResult OdDbRadialDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool /*bStretch*/) {
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
		case 0: {
			// radius is constant
			// chord point is calculated using center and text points
			// text point may be changed in recompute dimension block
			const auto dimCenterPt {Dimension->center()};
			auto ChordPoint {Dimension->chordPoint()};
			const auto radius {dimCenterPt.distanceTo(ChordPoint)};
			const auto TextPosition {Dimension->textPosition()};
			auto v {TextPosition - dimNewPt};
			v.normalize();
			ChordPoint = dimNewPt + v * radius;
			Dimension->setChordPoint(ChordPoint);
			Dimension->setCenter(dimNewPt);
			break;
		}
		case 1: {
			// radius is constant, center point is constant
			// text points is calculated and may be changed in recompute dimension block
			const auto dimCenterPt {Dimension->center()};
			auto ChordPoint {Dimension->chordPoint()};
			const auto dist {dimCenterPt.distanceTo(Dimension->textPosition())};
			const auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
			auto ocsDimCenterPt {dimCenterPt};
			auto ocsDimChordPt {ChordPoint};
			OdGeLine3d line1;
			OdGeLine3d line2;
			auto ocsDimNewPt(dimNewPt);
			if (NeedTransform) {
				ocsDimCenterPt.transformBy(WorldToPlaneTransform);
				ocsDimChordPt.transformBy(WorldToPlaneTransform);
				ocsDimNewPt.transformBy(WorldToPlaneTransform);
			}
			const auto SavedZCoordinate {ocsDimCenterPt.z};
			ocsDimCenterPt.z = ocsDimChordPt.z = 0.0;
			ocsDimNewPt.z = 0.0;
			const auto vX {ocsDimCenterPt - ocsDimNewPt};
			auto vY {ocsDimCenterPt - ocsDimChordPt};
			const auto angle {vY.angleTo(vX, OdGeVector3d::kZAxis)};
			// if( !angle ) return eOk;
			vY.rotateBy(Oda2PI - (OdaPI - angle) / 2, OdGeVector3d::kZAxis);
			line1.set(ocsDimChordPt, vY);
			line2.set(ocsDimCenterPt, vX);
			line1.intersectWith(line2, ocsDimChordPt);
			dimNewPt = ocsDimChordPt;
			dimNewPt.z = SavedZCoordinate;
			if (NeedTransform) {
				dimNewPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
			}
			auto v {dimNewPt - dimCenterPt};
			v.normalize();
			const auto textPt {dimCenterPt + v * dist};
			Dimension->setTextPosition(textPt);
			Dimension->setChordPoint(dimNewPt);
			break;
		}
		case 2: {
			// radius is constant, center point is constant
			// chord point is calculated using center and text points
			const auto Center {Dimension->center()};
			auto ChordPoint {Dimension->chordPoint()};
			const auto Radius {Center.distanceTo(ChordPoint)};
			const auto textPt {dimNewPt};
			auto v {textPt - Center};
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
