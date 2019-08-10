#include <OdaCommon.h>
#include <DbRotatedDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include <Ge/GeLineSeg3d.h>
#include "DbRotatedDimGripPoints.h"

OdResult OdDbRotatedDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbRotatedDimensionPtr Dimension {entity};
	const auto GripPointsSize {gripPoints.size()};
	if (Dimension->jogSymbolOn()) {
		gripPoints.resize(GripPointsSize + 6);
	} else {
		gripPoints.resize(GripPointsSize + 5);
	}
	const auto FirstExtensionLineStartPoint {Dimension->xLine1Point()};
	const auto SecondExtensionLineStartPoint {Dimension->xLine2Point()};
	const auto DimensionLinePoint {Dimension->dimLinePoint()};
	const auto TextPosition {Dimension->textPosition()};
	const auto Rotation {Dimension->rotation()};
	auto Oblique {Dimension->oblique()};
	const auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
	auto ocsDefPt1 {FirstExtensionLineStartPoint};
	auto ocsDimLinDefPt {DimensionLinePoint};
	const auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDefPt1.transformBy(WorldToPlaneTransform);
		ocsDimLinDefPt.transformBy(WorldToPlaneTransform);
	}
	const auto savedZCoordinate {ocsDefPt1.z};
	ocsDefPt1.z = ocsDimLinDefPt.z = 0.0;
	auto v1 {OdGeVector3d::kXAxis};
	v1.rotateBy(Rotation, OdGeVector3d::kZAxis);
	auto v2 {OdGeVector3d::kYAxis};
	if (OdNonZero(Oblique)) {
		Oblique = Oblique - OdaPI2;
	}
	v2.rotateBy(Rotation + Oblique, OdGeVector3d::kZAxis);
	OdGeLine3d line1;
	OdGeLine3d line2;
	line1.set(ocsDimLinDefPt, v1);
	line2.set(ocsDefPt1, v2);
	OdGePoint3d extLineEnd;
	line1.intersectWith(line2, extLineEnd);
	extLineEnd.z = savedZCoordinate;
	if (NeedTransform) {
		extLineEnd.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
	}
	gripPoints[GripPointsSize + 0] = FirstExtensionLineStartPoint;
	gripPoints[GripPointsSize + 1] = SecondExtensionLineStartPoint;
	gripPoints[GripPointsSize + 2] = extLineEnd;
	gripPoints[GripPointsSize + 3] = DimensionLinePoint;
	gripPoints[GripPointsSize + 4] = TextPosition;
	if (Dimension->jogSymbolOn()) {
		gripPoints[GripPointsSize + 5] = Dimension->jogSymbolPosition();
	}
	return eOk;
}

OdResult OdDbRotatedDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbRotatedDimensionPtr Dimension {entity};
	const OdGePoint3d* GripPoint;
	OdGePoint3d dimNewPt;
	auto savedZCoordinate {0.0};
	OdGeVector3d vMoveTxt;
	auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
	}
	for (auto i = 0; i < static_cast<int>(indices.size()); i++) {
		GripPoint = &gripPoints[indices[i]];
		dimNewPt = *GripPoint;
		switch (indices[i]) {
			case 0:
				Dimension->setXLine1Point(dimNewPt);
				break;
			case 1:
				Dimension->setXLine2Point(dimNewPt);
				break;
			case 2: {
				auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
				auto ocsDimNewPt(dimNewPt);
				auto DimensionLinePoint {Dimension->dimLinePoint()};
				auto ocsDimLinePt(DimensionLinePoint);
				auto FirstExtensionLineStartPoint {Dimension->xLine1Point()};
				auto ocsDefPt1(FirstExtensionLineStartPoint);
				if (NeedTransform) {
					ocsDimNewPt.transformBy(WorldToPlaneTransform);
					ocsDimLinePt.transformBy(WorldToPlaneTransform);
					ocsDefPt1.transformBy(WorldToPlaneTransform);
				}
				savedZCoordinate = ocsDefPt1.z;
				ocsDimNewPt.z = 0.0;
				ocsDimLinePt.z = 0.0;
				ocsDefPt1.z = 0.0;
				auto Rotation {Dimension->rotation()};
				auto Oblique {Dimension->oblique()};
				auto v1 {OdGeVector3d::kXAxis};
				v1.rotateBy(Rotation, OdGeVector3d::kZAxis);
				auto v2 {OdGeVector3d::kYAxis};
				if (OdNonZero(Oblique)) {
					Oblique = Oblique - OdaPI2;
				}
				v2.rotateBy(Rotation + Oblique, OdGeVector3d::kZAxis);
				OdGeLine3d line1;
				OdGeLine3d line2;
				line1.set(ocsDimLinePt, v1);
				line2.set(ocsDefPt1, v2);
				OdGePoint3d extLineEnd;
				line1.intersectWith(line2, extLineEnd);
				vMoveTxt = extLineEnd - ocsDimNewPt;
				line1.set(ocsDimNewPt, v1);
				line2.set(ocsDimLinePt, v2);
				line1.intersectWith(line2, ocsDimNewPt);
				dimNewPt = ocsDimNewPt;
				dimNewPt.z = savedZCoordinate;
				if (NeedTransform) {
					dimNewPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
				}
			}
			case 3:
				if (!Dimension->isUsingDefaultTextPosition() && Dimension->dimtmove() == 0) {
					auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
					if (indices[0] == 3) {
						auto ocsDimLinePt(Dimension->dimLinePoint());
						auto ocsDimNewPt {dimNewPt};
						if (NeedTransform) {
							ocsDimLinePt.transformBy(WorldToPlaneTransform);
							ocsDimNewPt.transformBy(WorldToPlaneTransform);
						}
						ocsDimLinePt.z = 0.0;
						ocsDimNewPt.z = 0.0;
						vMoveTxt = ocsDimLinePt - ocsDimNewPt;
					}
					auto dimTextPt {gripPoints[4]};
					auto ocsDimTextPt {dimTextPt};
					if (NeedTransform) {
						ocsDimTextPt.transformBy(WorldToPlaneTransform);
					}
					savedZCoordinate = ocsDimTextPt.z;
					ocsDimTextPt.z = 0.0;
					auto ocsDimNewTextPt {ocsDimTextPt - vMoveTxt};
					auto dimNewTextPt {ocsDimNewTextPt};
					dimNewTextPt.z = savedZCoordinate;
					if (NeedTransform) {
						dimNewTextPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
					}
					Dimension->setTextPosition(dimNewTextPt);
				} else {
					Dimension->setDimLinePoint(dimNewPt);
				}
				break;
			case 4:
				Dimension->setTextPosition(dimNewPt);
				if (indices.size() == 1 || !bStretch) {
					Dimension->useSetTextPosition();
				}
				break;
			case 5:
				if (!Dimension->isUsingDefaultTextPosition() && Dimension->dimtmove() == 0) {
					auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
					auto ocsDimJogPt(Dimension->jogSymbolPosition());
					auto ocsDimNewPt {dimNewPt};
					auto SecondExtensionLineStartPoint {Dimension->xLine2Point()};
					auto DimensionLinePoint {Dimension->dimLinePoint()};
					if (NeedTransform) {
						ocsDimJogPt.transformBy(WorldToPlaneTransform);
						ocsDimNewPt.transformBy(WorldToPlaneTransform);
						SecondExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
						DimensionLinePoint.transformBy(WorldToPlaneTransform);
					}
					ocsDimJogPt.z = 0.0;
					ocsDimNewPt.z = 0.0;
					SecondExtensionLineStartPoint.z = DimensionLinePoint.z = 0.0;
					auto v {DimensionLinePoint - SecondExtensionLineStartPoint};
					vMoveTxt = ocsDimJogPt - ocsDimNewPt;
					if (v.length()) {
						v.normalize();
					}
					v *= vMoveTxt.length() * cos(v.angleTo(vMoveTxt));
					auto dimTextPt {gripPoints[4]};
					auto ocsDimTextPt {dimTextPt};
					if (NeedTransform) {
						ocsDimTextPt.transformBy(WorldToPlaneTransform);
					}
					savedZCoordinate = ocsDimTextPt.z;
					ocsDimTextPt.z = 0.0;
					auto ocsDimNewTextPt {ocsDimTextPt - v};
					auto dimNewTextPt {ocsDimNewTextPt};
					dimNewTextPt.z = savedZCoordinate;
					if (NeedTransform) {
						dimNewTextPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
					}
					Dimension->setTextPosition(dimNewTextPt);
				} else {
					Dimension->setDimLinePoint(dimNewPt);
				}
				Dimension->setJogSymbolPosition(dimNewPt);
				break;
			default:
				break;
		}
	}
	return eOk;
}
