#include <OdaCommon.h>
#include <DbRadialDimensionLarge.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "DbRadialDimLargeGripPoints.h"

OdResult OdDbRadialDimLargeGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 4);
	OdDbRadialDimensionLargePtr Dimension {entity};
	gripPoints[GripPointsSize + 0] = Dimension->chordPoint();
	gripPoints[GripPointsSize + 1] = Dimension->overrideCenter();
	gripPoints[GripPointsSize + 2] = Dimension->jogPoint();
	gripPoints[GripPointsSize + 3] = Dimension->textPosition();
	return eOk;
}

OdResult OdDbRadialDimLargeGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
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
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
	}
	auto WorldToPlaneTransform(OdGeMatrix3d::worldToPlane(Normal));
	auto PlaneToWorldTransform(OdGeMatrix3d::planeToWorld(Normal));
	auto ocsDimChordPt {ChordPoint};
	auto ocsDimOverrideCenterPt {OverrideCenter};
	auto ocsDimJogPt {JogPoint};
	auto ocsDimTextPt {TextPosition};
	auto ocsDimCenterPt {Center};
	OdGeVector3d vX1;
	const OdGePoint3d* GripPoint = nullptr;
	OdGeLine3d line1;
	OdGeLine3d line2;
	GripPoint = &gripPoints[indices[0]];
	auto ocsDimNewPt {*GripPoint};
	if (NeedTransform) {
		ocsDimChordPt.transformBy(WorldToPlaneTransform);
		ocsDimOverrideCenterPt.transformBy(WorldToPlaneTransform);
		ocsDimJogPt.transformBy(WorldToPlaneTransform);
		ocsDimTextPt.transformBy(WorldToPlaneTransform);
		ocsDimCenterPt.transformBy(WorldToPlaneTransform);
		ocsDimNewPt.transformBy(WorldToPlaneTransform);
	}
	auto SavedZCoordinate {ocsDimChordPt.z};
	ocsDimChordPt.z = ocsDimOverrideCenterPt.z = ocsDimJogPt.z = ocsDimTextPt.z = ocsDimCenterPt.z = 0.0;
	ocsDimNewPt.z = 0.0;
	if (indices[0] == 0) {
		auto dimLRadius {fabs(OdGeVector3d(ocsDimCenterPt - ocsDimChordPt).length())};
		auto vR {ocsDimCenterPt - ocsDimChordPt};
		vR = ocsDimCenterPt - ocsDimNewPt;
		vR.normalize();
		vR *= dimLRadius;
		ocsDimNewPt = ocsDimCenterPt - vR;
		ocsDimChordPt = ocsDimNewPt;
		ocsDimNewPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimNewPt.transformBy(PlaneToWorldTransform);
		}
		Dimension->setChordPoint(ocsDimNewPt); // correct text point
		vR.normalize();
		vR *= ocsDimTextPt.distanceTo(ocsDimCenterPt);
		ocsDimTextPt = ocsDimCenterPt - vR;
		ocsDimTextPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimTextPt.transformBy(PlaneToWorldTransform);
		}
		Dimension->setTextPosition(ocsDimTextPt);
	}
	if (indices[0] == 1) {
		ocsDimOverrideCenterPt = ocsDimNewPt;
		if (NeedTransform) {
			ocsDimNewPt.transformBy(PlaneToWorldTransform);
		}
		Dimension->setOverrideCenter(ocsDimNewPt);
	}
	if (indices[0] == 2) {
		auto vR {ocsDimCenterPt - ocsDimChordPt};
		vX1 = vR;
		vX1.rotateBy(JogAngle, OdGeVector3d::kZAxis);
		line1.set(ocsDimCenterPt, vR);
		line2.set(ocsDimNewPt, vX1);
		OdGePoint3d IntersectPoint;
		line1.intersectWith(line2, IntersectPoint);
		line1.set(ocsDimOverrideCenterPt, vR);
		OdGePoint3d Intersect2Point;
		line1.intersectWith(line2, Intersect2Point);
		auto dimLen {fabs(OdGeVector3d(IntersectPoint - Intersect2Point).length())};
		auto vR2 {IntersectPoint - Intersect2Point};
		vR2.normalize();
		vR2 *= dimLen / 2;
		ocsDimJogPt = IntersectPoint - vR2;
		ocsDimJogPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimJogPt.transformBy(PlaneToWorldTransform);
		}
		Dimension->setJogPoint(ocsDimJogPt);
	}
	if (indices[0] == 3) {
		auto dimLRadius {fabs(OdGeVector3d(ocsDimCenterPt - ocsDimChordPt).length())};
		auto vR {ocsDimCenterPt - ocsDimChordPt};
		vR = ocsDimCenterPt - ocsDimNewPt;
		vR.normalize();
		vR *= dimLRadius;
		ocsDimChordPt = ocsDimCenterPt - vR;
		ocsDimChordPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimChordPt.transformBy(PlaneToWorldTransform);
		}
		Dimension->setChordPoint(ocsDimChordPt);
		ocsDimTextPt = ocsDimNewPt;
		ocsDimTextPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimTextPt.transformBy(PlaneToWorldTransform);
		}
		Dimension->setTextPosition(ocsDimTextPt); // correct jog point
		if (indices.size() == 1 || !bStretch) {
			Dimension->useSetTextPosition();
		}
	}
	if (indices[0] != 2) {
		// correct jog point
		auto dimLen {fabs(OdGeVector3d(ocsDimChordPt - ocsDimOverrideCenterPt).length())};
		auto vR2 {ocsDimChordPt - ocsDimOverrideCenterPt};
		vR2.normalize();
		vR2 *= dimLen / 2;
		ocsDimJogPt = ocsDimChordPt - vR2;
		ocsDimJogPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimJogPt.transformBy(PlaneToWorldTransform);
		}
		Dimension->setJogPoint(ocsDimJogPt);
	}
	return eOk;
}
