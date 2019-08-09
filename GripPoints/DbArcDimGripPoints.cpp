#include <OdaCommon.h>
#include <DbArcDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "DbArcDimGripPoints.h"

OdResult OdDbArcDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 4);
	OdDbArcDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + 0] = Dimension->xLine1Point();
	gripPoints[GripPointsSize + 1] = Dimension->xLine2Point();
	gripPoints[GripPointsSize + 2] = Dimension->arcPoint();
	gripPoints[GripPointsSize + 3] = Dimension->textPosition();
	return eOk;
}

OdResult OdDbArcDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbArcDimensionPtr Dimension {entity};
	auto FirstExtensionLineStartPoint {Dimension->xLine1Point()};
	auto SecondExtensionLineStartPoint {Dimension->xLine2Point()};
	auto dimArcPt {Dimension->arcPoint()};
	auto TextPosition {Dimension->textPosition()};
	auto dimCenterPt {Dimension->centerPoint()};
	auto dimArcNewPt {dimArcPt};
	OdGePoint3d dimNewTextPt;
	auto WorldToPlaneTransform(OdGeMatrix3d::worldToPlane(Dimension->normal()));
	auto ocsDimLine1Pt {FirstExtensionLineStartPoint};
	auto ocsDimLine2Pt {SecondExtensionLineStartPoint};
	auto ocsDimArcPt {dimArcPt};
	auto ocsDimTextPt {TextPosition};
	auto ocsDimArcNewPt {dimArcNewPt};
	auto ocsDimCenterPt {dimCenterPt};
	auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimLine1Pt.transformBy(WorldToPlaneTransform);
		ocsDimLine2Pt.transformBy(WorldToPlaneTransform);
		ocsDimArcPt.transformBy(WorldToPlaneTransform);
		ocsDimTextPt.transformBy(WorldToPlaneTransform);
		ocsDimArcNewPt.transformBy(WorldToPlaneTransform);
		ocsDimCenterPt.transformBy(WorldToPlaneTransform);
	}
	auto SavedZCoordinate {ocsDimLine1Pt.z};
	ocsDimLine1Pt.z = ocsDimLine2Pt.z = ocsDimArcPt.z = ocsDimTextPt.z = ocsDimArcNewPt.z = ocsDimCenterPt.z = 0.0;
	OdGeVector3d vX1;
	OdGeVector3d vX2;
	OdGeVector3d vArc;
	OdGeVector3d vTxt;
	const OdGePoint3d* GripPoint = nullptr;
	OdGeLine3d line1;
	OdGeLine3d line2;
	GripPoint = &gripPoints[indices[0]];
	auto ocsDimNewPt {*GripPoint};
	auto dimNewPt {ocsDimNewPt};
	if (NeedTransform) {
		ocsDimNewPt.transformBy(WorldToPlaneTransform);
	}
	ocsDimNewPt.z = 0.0;
	for (auto i = 0; i < static_cast<int>(indices.size()); i++) {
		GripPoint = &gripPoints[indices[i]];
		dimNewPt = *GripPoint;
		ocsDimNewPt = dimNewPt;
		if (indices[i] < 3 && !Dimension->isUsingDefaultTextPosition()) {
			Dimension->useDefaultTextPosition();
		}
		if (indices[i] == 0) {
			Dimension->setXLine1Point(*GripPoint);
			continue;
		}
		if (indices[i] == 1) {
			Dimension->setXLine2Point(*GripPoint);
			continue;
		}
		vX1 = ocsDimCenterPt - ocsDimLine1Pt;
		vX2 = ocsDimCenterPt - ocsDimLine2Pt;
		auto angle1 {vX2.angleTo(vX1)};
		if (indices[i] == 2) {
			ocsDimArcNewPt = ocsDimNewPt;
		}
		if (indices[i] == 3) {
			if (indices.size() == 1 || !bStretch) {
				Dimension->useSetTextPosition();
			}
			auto vT {ocsDimCenterPt - ocsDimArcNewPt};
			vT.normalize();
			vT *= fabs(OdGeVector3d(ocsDimCenterPt - ocsDimNewPt).length());
			ocsDimArcNewPt = ocsDimCenterPt - vT;
			ocsDimTextPt = ocsDimNewPt;
		}
		vArc = dimCenterPt - ocsDimArcNewPt;
		auto angle2 {vX1.angleTo(vArc)};
		auto angle3 {vX2.angleTo(vArc)};
		OdGeVector3d vXA;
		if (OdEqual(angle3 - angle2, angle1, OdGeContext::gTol.equalPoint())) {
			vX2 = ocsDimLine2Pt - ocsDimCenterPt;
		}
		if (OdEqual(angle2 - angle3, angle1, OdGeContext::gTol.equalPoint())) {
			vX1 = ocsDimLine1Pt - ocsDimCenterPt;
		}
		if (OdEqual(Oda2PI - (angle3 + angle2), angle1, OdGeContext::gTol.equalPoint())) {
			vX2 = ocsDimLine2Pt - ocsDimCenterPt;
			vX1 = ocsDimLine1Pt - ocsDimCenterPt;
		}
		angle1 = vX2.angleTo(vX1);
		vXA = !IsAngleDirectionBetweenVectors(vArc, vX1) ? vX1 : vX2;
		vTxt = vXA;
		vTxt.rotateBy(vX1.angleTo(vX2) / 2, OdGeVector3d::kZAxis);
		vXA.rotateBy(angle1 / 3, OdGeVector3d::kZAxis);
		auto vY {vXA};
		angle1 = vArc.angleTo(vXA, OdGeVector3d::kZAxis);
		vY.rotateBy((OdaPI - angle1) / 2, OdGeVector3d::kZAxis);
		line1.set(ocsDimArcNewPt, vY);
		line2.set(dimCenterPt, vXA);
		OdGePoint3d IntersectPoint;
		line1.intersectWith(line2, IntersectPoint);
		ocsDimArcPt = IntersectPoint;
		auto vT1 {vTxt};
		vT1.normalize();
		vT1 *= OdGeVector3d(dimCenterPt - ocsDimArcPt).length();
		dimNewTextPt = dimCenterPt - vT1;
		if (indices[i] == 5) {
			dimNewTextPt = ocsDimTextPt;
		}
		ocsDimArcPt.z = SavedZCoordinate;
		dimNewTextPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimArcPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
			dimNewTextPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
		}
		if (indices[i] == 2 || Dimension->dimtmove() == 0) {
			Dimension->setArcPoint(ocsDimArcPt);
		}
		Dimension->setTextPosition(dimNewTextPt);
	}
	return eOk;
}
