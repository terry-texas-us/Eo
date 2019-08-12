#include <OdaCommon.h>
#include <Db2LineAngularDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "Db2LineAngularDimGripPoints.h"

OdResult OdDb2LineAngularDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 6);
	OdDb2LineAngularDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + kFirstExtensionLineStartPoint] = Dimension->xLine1Start();
	gripPoints[GripPointsSize + kFirstExtensionLineEndPoint] = Dimension->xLine1End();
	gripPoints[GripPointsSize + kSecondExtensionLineStartPoint] = Dimension->xLine2Start();
	gripPoints[GripPointsSize + kSecondExtensionLineEndPoint] = Dimension->xLine2End();
	gripPoints[GripPointsSize + kArcPoint] = Dimension->arcPoint();
	gripPoints[GripPointsSize + kTextPosition] = Dimension->textPosition();
	return eOk;
}

OdResult OdDb2LineAngularDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool stretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDb2LineAngularDimensionPtr Dimension {entity};
	auto FirstExtensionLineStartPoint {Dimension->xLine1Start()};
	auto FirstExtensionLineEndPoint {Dimension->xLine1End()};
	auto SecondExtensionLineStartPoint {Dimension->xLine2Start()};
	auto SecondExtensionLineEndPoint {Dimension->xLine2End()};
	auto ArcPoint {Dimension->arcPoint()};
	auto TextPosition {Dimension->textPosition()};
	auto NewArcPoint {ArcPoint};
	OdGePoint3d NewTextPosition;
	auto WorldToPlaneTransform(OdGeMatrix3d::worldToPlane(Dimension->normal()));
	auto ocsDimLine1StartPt {FirstExtensionLineStartPoint};
	auto ocsDimLine1EndPt {FirstExtensionLineEndPoint};
	auto ocsDimLine2StartPt {SecondExtensionLineStartPoint};
	auto ocsDimLine2EndPt {SecondExtensionLineEndPoint};
	auto ocsDimArcPt {ArcPoint};
	auto ocsDimTextPt {TextPosition};
	auto ocsDimArcNewPt {NewArcPoint};
	auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimLine1StartPt.transformBy(WorldToPlaneTransform);
		ocsDimLine1EndPt.transformBy(WorldToPlaneTransform);
		ocsDimLine2StartPt.transformBy(WorldToPlaneTransform);
		ocsDimLine2EndPt.transformBy(WorldToPlaneTransform);
		ocsDimArcPt.transformBy(WorldToPlaneTransform);
		ocsDimTextPt.transformBy(WorldToPlaneTransform);
		ocsDimArcNewPt.transformBy(WorldToPlaneTransform);
	}
	auto SavedZCoordinate {ocsDimLine1StartPt.z};
	ocsDimLine1StartPt.z = ocsDimLine1EndPt.z = ocsDimLine2StartPt.z = ocsDimLine2EndPt.z = ocsDimArcPt.z = ocsDimTextPt.z = ocsDimArcNewPt.z = 0.0;
	const OdGePoint3d* GripPoint = nullptr;
	OdGePoint3d ocsDimNewPt;
	OdGePoint3d dimNewPt;
	if (NeedTransform) {
		ocsDimNewPt.transformBy(WorldToPlaneTransform);
	}
	ocsDimNewPt.z = 0.0;
	for (auto i = 0U; i < indices.size(); i++) {
		GripPoint = &gripPoints[indices[i]];
		dimNewPt = *GripPoint;
		ocsDimNewPt = dimNewPt;
		if (indices[i] < kTextPosition && !Dimension->isUsingDefaultTextPosition()) {
			Dimension->useDefaultTextPosition();
		}
		switch (indices[i]) {
			case kFirstExtensionLineStartPoint:
				Dimension->setXLine1Start(dimNewPt);
				break;
			case kFirstExtensionLineEndPoint:
				Dimension->setXLine1End(dimNewPt);
				break;
			case kSecondExtensionLineStartPoint:
				Dimension->setXLine2Start(dimNewPt);
				break;
			case kSecondExtensionLineEndPoint:
				Dimension->setXLine2End(dimNewPt);
				break;
			case kArcPoint: case kTextPosition: {
				OdGePoint3d CenterPoint;
				auto vX1 {ocsDimLine1EndPt - ocsDimLine1StartPt};
				auto vX2 {ocsDimLine2StartPt - ocsDimLine2EndPt};
				OdGeLine3d line1(ocsDimLine1EndPt, vX1);
				OdGeLine3d line2(ocsDimLine2StartPt, vX2);
				line1.intersectWith(line2, CenterPoint);
				auto Angle1 {vX2.angleTo(vX1)};
				if (indices[0] == kArcPoint) {
					ocsDimArcNewPt = ocsDimNewPt;
				}
				if (indices[0] == kTextPosition) {
					auto vT {CenterPoint - ocsDimArcNewPt};
					vT.normalize();
					vT *= fabs(OdGeVector3d(CenterPoint - ocsDimNewPt).length());
					ocsDimArcNewPt = CenterPoint - vT;
					ocsDimTextPt = ocsDimNewPt;
				}
				auto vArc {CenterPoint - ocsDimArcNewPt};
				auto Angle2 {vX1.angleTo(vArc)};
				auto Angle3 {vX2.angleTo(vArc)};
				if (OdEqual(Angle3 - Angle2, Angle1, OdGeContext::gTol.equalPoint())) {
					vX2 = ocsDimLine2EndPt - ocsDimLine2StartPt;
				}
				if (OdEqual(Angle2 - Angle3, Angle1, OdGeContext::gTol.equalPoint())) {
					vX1 = ocsDimLine1StartPt - ocsDimLine1EndPt;
				}
				if (OdEqual(Oda2PI - (Angle3 + Angle2), Angle1, OdGeContext::gTol.equalPoint())) {
					vX2 = ocsDimLine2EndPt - ocsDimLine2StartPt;
					vX1 = ocsDimLine1StartPt - ocsDimLine1EndPt;
				}
				Angle1 = vX2.angleTo(vX1);
				auto vXA {!IsAngleDirectionBetweenVectors(vArc, vX1) ? vX1 : vX2};
				auto vTxt {vXA};
				vTxt.rotateBy(vX1.angleTo(vX2) / 2, OdGeVector3d::kZAxis);
				vXA.rotateBy(Angle1 / 3, OdGeVector3d::kZAxis);
				auto vY {vXA};
				Angle1 = vArc.angleTo(vXA, OdGeVector3d::kZAxis);
				vY.rotateBy((OdaPI - Angle1) / 2, OdGeVector3d::kZAxis);
				line1.set(ocsDimArcNewPt, vY);
				line2.set(CenterPoint, vXA);
				OdGePoint3d IntersectPoint;
				line1.intersectWith(line2, IntersectPoint);
				ocsDimArcPt = IntersectPoint;
				auto vT1 {vTxt};
				vT1.normalize();
				vT1 *= OdGeVector3d(CenterPoint - ocsDimArcPt).length();
				NewTextPosition = CenterPoint - vT1;
				if (indices[0] == kTextPosition) {
					NewTextPosition = ocsDimTextPt;
				}
				ocsDimArcPt.z = SavedZCoordinate;
				NewTextPosition.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsDimArcPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
					NewTextPosition.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
				}
				if (indices[0] == kArcPoint || Dimension->dimtmove() == 0) {
					Dimension->setArcPoint(ocsDimArcPt);
				}
				if (indices[0] == kTextPosition) {
					if (indices.size() == 1 || !stretch) {
						Dimension->useSetTextPosition();
					}
					Dimension->setTextPosition(NewTextPosition);
				}
			}
			default:
				break;
		}
	}
	return eOk;
}
