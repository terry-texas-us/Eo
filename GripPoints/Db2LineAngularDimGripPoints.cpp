#include <OdaCommon.h>
#include <Db2LineAngularDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "Db2LineAngularDimGripPoints.h"

OdResult OdDb2LineAngularDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 6);
	OdDb2LineAngularDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + 0] = Dimension->xLine1Start();
	gripPoints[GripPointsSize + 1] = Dimension->xLine1End();
	gripPoints[GripPointsSize + 2] = Dimension->xLine2Start();
	gripPoints[GripPointsSize + 3] = Dimension->xLine2End();
	gripPoints[GripPointsSize + 4] = Dimension->arcPoint();
	gripPoints[GripPointsSize + 5] = Dimension->textPosition();
	return eOk;
}

OdResult OdDb2LineAngularDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
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
	OdGePoint3d dimCenterPt;
	auto dimArcNewPt {ArcPoint};
	OdGePoint3d dimNewTextPt;
	auto WorldToPlaneTransform(OdGeMatrix3d::worldToPlane(Dimension->normal()));
	auto ocsDimLine1StartPt {FirstExtensionLineStartPoint};
	auto ocsDimLine1EndPt {FirstExtensionLineEndPoint};
	auto ocsDimLine2StartPt {SecondExtensionLineStartPoint};
	auto ocsDimLine2EndPt {SecondExtensionLineEndPoint};
	auto ocsDimArcPt {ArcPoint};
	auto ocsDimTextPt {TextPosition};
	auto ocsDimArcNewPt {dimArcNewPt};
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
	OdGeVector3d vX1;
	OdGeVector3d vX2;
	OdGeVector3d vArc;
	OdGeVector3d vTxt;
	const OdGePoint3d* GripPoint = nullptr;
	OdGeLine3d line1;
	OdGeLine3d line2;
	GripPoint = &gripPoints[indices[0]];
	OdGePoint3d ocsDimNewPt;
	OdGePoint3d dimNewPt;
	if (NeedTransform) {
		ocsDimNewPt.transformBy(WorldToPlaneTransform);
	}
	ocsDimNewPt.z = 0.0;
	for (auto i = 0; i < static_cast<int>(indices.size()); i++) {
		GripPoint = &gripPoints[indices[i]];
		dimNewPt = *GripPoint;
		ocsDimNewPt = dimNewPt;
		if (indices[i] < 5 && !Dimension->isUsingDefaultTextPosition()) {
			Dimension->useDefaultTextPosition();
		}
		switch (indices[i]) {
			case 0:
				Dimension->setXLine1Start(dimNewPt);
				break;
			case 1:
				Dimension->setXLine1End(dimNewPt);
				break;
			case 2:
				Dimension->setXLine2Start(dimNewPt);
				break;
			case 3:
				Dimension->setXLine2End(dimNewPt);
				break;
			case 4: case 5: {
				vX1 = ocsDimLine1EndPt - ocsDimLine1StartPt;
				vX2 = ocsDimLine2StartPt - ocsDimLine2EndPt;
				line1.set(ocsDimLine1EndPt, vX1);
				line2.set(ocsDimLine2StartPt, vX2);
				line1.intersectWith(line2, dimCenterPt);
				auto Angle1 {vX2.angleTo(vX1)};
				if (indices[0] == 4) {
					ocsDimArcNewPt = ocsDimNewPt;
				}
				if (indices[0] == 5) {
					auto vT {dimCenterPt - ocsDimArcNewPt};
					vT.normalize();
					vT *= fabs(OdGeVector3d(dimCenterPt - ocsDimNewPt).length());
					ocsDimArcNewPt = dimCenterPt - vT;
					ocsDimTextPt = ocsDimNewPt;
				}
				vArc = dimCenterPt - ocsDimArcNewPt;
				auto Angle2 {vX1.angleTo(vArc)};
				auto Angle3 {vX2.angleTo(vArc)};
				OdGeVector3d vXA;
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
				vXA = !IsAngleDirectionBetweenVectors(vArc, vX1) ? vX1 : vX2;
				vTxt = vXA;
				vTxt.rotateBy(vX1.angleTo(vX2) / 2, OdGeVector3d::kZAxis);
				vXA.rotateBy(Angle1 / 3, OdGeVector3d::kZAxis);
				auto vY {vXA};
				Angle1 = vArc.angleTo(vXA, OdGeVector3d::kZAxis);
				vY.rotateBy((OdaPI - Angle1) / 2, OdGeVector3d::kZAxis);
				line1.set(ocsDimArcNewPt, vY);
				line2.set(dimCenterPt, vXA);
				OdGePoint3d IntersectPoint;
				line1.intersectWith(line2, IntersectPoint);
				ocsDimArcPt = IntersectPoint;
				auto vT1 {vTxt};
				vT1.normalize();
				vT1 *= OdGeVector3d(dimCenterPt - ocsDimArcPt).length();
				dimNewTextPt = dimCenterPt - vT1;
				if (indices[0] == 5) {
					dimNewTextPt = ocsDimTextPt;
				}
				ocsDimArcPt.z = SavedZCoordinate;
				dimNewTextPt.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsDimArcPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
					dimNewTextPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
				}
				if (indices[0] == 4 || Dimension->dimtmove() == 0) {
					Dimension->setArcPoint(ocsDimArcPt);
				}
				if (indices[0] == 5) {
					if (indices.size() == 1 || !bStretch) {
						Dimension->useSetTextPosition();
					}
					Dimension->setTextPosition(dimNewTextPt);
				}
			}
			default:
				break;
		}
	}
	return eOk;
}
