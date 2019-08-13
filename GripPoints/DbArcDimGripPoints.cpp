#include <OdaCommon.h>
#include <DbArcDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "DbArcDimGripPoints.h"

OdResult OdDbArcDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 4);
	OdDbArcDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + kFirstExtensionLineStartPoint] = Dimension->xLine1Point();
	gripPoints[GripPointsSize + kSecondExtensionLineStartPoint] = Dimension->xLine2Point();
	gripPoints[GripPointsSize + kArcPoint] = Dimension->arcPoint();
	gripPoints[GripPointsSize + kTextPosition] = Dimension->textPosition();
	return eOk;
}

OdResult OdDbArcDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool stretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbArcDimensionPtr Dimension {entity};
	auto FirstExtensionLineStartPoint {Dimension->xLine1Point()};
	auto SecondExtensionLineStartPoint {Dimension->xLine2Point()};
	auto ArcPoint {Dimension->arcPoint()};
	auto TextPosition {Dimension->textPosition()};
	auto CenterPoint {Dimension->centerPoint()};
	auto NewArcPoint {ArcPoint};
	auto WorldToPlaneTransform(OdGeMatrix3d::worldToPlane(Dimension->normal()));
	auto ocsFirstExtensionLineStartPoint {FirstExtensionLineStartPoint};
	auto ocsSecondExtensionLineStartPoint {SecondExtensionLineStartPoint};
	auto ocsArcPoint {ArcPoint};
	auto ocsTextPosition {TextPosition};
	auto ocsNewArcPoint {NewArcPoint};
	auto ocsCenterPoint {CenterPoint};
	auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsFirstExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
		ocsSecondExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
		ocsArcPoint.transformBy(WorldToPlaneTransform);
		ocsTextPosition.transformBy(WorldToPlaneTransform);
		ocsNewArcPoint.transformBy(WorldToPlaneTransform);
		ocsCenterPoint.transformBy(WorldToPlaneTransform);
	}
	auto SavedZCoordinate {ocsFirstExtensionLineStartPoint.z};
	ocsFirstExtensionLineStartPoint.z = ocsSecondExtensionLineStartPoint.z = ocsArcPoint.z = ocsTextPosition.z = ocsNewArcPoint.z = ocsCenterPoint.z = 0.0;
	auto GripPoint {&gripPoints[indices[0]]};
	auto ocsGripPoint {*GripPoint};
	auto NewGripPoint {ocsGripPoint};
	if (NeedTransform) {
		ocsGripPoint.transformBy(WorldToPlaneTransform);
	}
	ocsGripPoint.z = 0.0;
	for (auto i = 0U; i < indices.size(); i++) {
		GripPoint = &gripPoints[indices[i]];
		NewGripPoint = *GripPoint;
		ocsGripPoint = NewGripPoint;
		if (indices[i] < kTextPosition && !Dimension->isUsingDefaultTextPosition()) {
			Dimension->useDefaultTextPosition();
		}
		if (indices[i] == kFirstExtensionLineStartPoint) {
			Dimension->setXLine1Point(*GripPoint);
			continue;
		}
		if (indices[i] == kSecondExtensionLineStartPoint) {
			Dimension->setXLine2Point(*GripPoint);
			continue;
		}
		auto vX1(ocsCenterPoint - ocsFirstExtensionLineStartPoint);
		auto vX2 {ocsCenterPoint - ocsSecondExtensionLineStartPoint};
		auto Angle1 {vX2.angleTo(vX1)};
		if (indices[i] == kArcPoint) {
			ocsNewArcPoint = ocsGripPoint;
		}
		if (indices[i] == kTextPosition) {
			if (indices.size() == 1 || !stretch) {
				Dimension->useSetTextPosition();
			}
			auto vT {ocsCenterPoint - ocsNewArcPoint};
			vT.normalize();
			vT *= fabs(OdGeVector3d(ocsCenterPoint - ocsGripPoint).length());
			ocsNewArcPoint = ocsCenterPoint - vT;
			ocsTextPosition = ocsGripPoint;
		}
		auto vArc {CenterPoint - ocsNewArcPoint};
		auto Angle2 {vX1.angleTo(vArc)};
		auto Angle3 {vX2.angleTo(vArc)};
		if (OdEqual(Angle3 - Angle2, Angle1, OdGeContext::gTol.equalPoint())) {
			vX2 = ocsSecondExtensionLineStartPoint - ocsCenterPoint;
		}
		if (OdEqual(Angle2 - Angle3, Angle1, OdGeContext::gTol.equalPoint())) {
			vX1 = ocsFirstExtensionLineStartPoint - ocsCenterPoint;
		}
		if (OdEqual(Oda2PI - (Angle3 + Angle2), Angle1, OdGeContext::gTol.equalPoint())) {
			vX2 = ocsSecondExtensionLineStartPoint - ocsCenterPoint;
			vX1 = ocsFirstExtensionLineStartPoint - ocsCenterPoint;
		}
		Angle1 = vX2.angleTo(vX1);
		auto vXA {!IsAngleDirectionBetweenVectors(vArc, vX1) ? vX1 : vX2};
		auto vTxt {vXA};
		vTxt.rotateBy(vX1.angleTo(vX2) / 2.0, OdGeVector3d::kZAxis);
		vXA.rotateBy(Angle1 / 3, OdGeVector3d::kZAxis);
		auto vY {vXA};
		Angle1 = vArc.angleTo(vXA, OdGeVector3d::kZAxis);
		vY.rotateBy((OdaPI - Angle1) / 2.0, OdGeVector3d::kZAxis);
		OdGeLine3d Line1(ocsNewArcPoint, vY);
		OdGeLine3d Line2(CenterPoint, vXA);
		OdGePoint3d IntersectPoint;
		Line1.intersectWith(Line2, IntersectPoint);
		ocsArcPoint = IntersectPoint;
		auto vT1 {vTxt};
		vT1.normalize();
		vT1 *= OdGeVector3d(CenterPoint - ocsArcPoint).length();
		auto NewTextPosition {CenterPoint - vT1};
		if (indices[i] == kTextPosition) {
			NewTextPosition = ocsTextPosition;
		}
		ocsArcPoint.z = SavedZCoordinate;
		NewTextPosition.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsArcPoint.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
			NewTextPosition.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
		}
		if (indices[i] == kArcPoint || Dimension->dimtmove() == 0) {
			Dimension->setArcPoint(ocsArcPoint);
		}
		Dimension->setTextPosition(NewTextPosition);
	}
	return eOk;
}
