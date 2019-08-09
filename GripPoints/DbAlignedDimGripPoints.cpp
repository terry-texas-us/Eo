#include <OdaCommon.h>
#include <DbAlignedDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include <Ge/GeLineSeg3d.h>
#include "DbAlignedDimGripPoints.h"

OdResult OdDbAlignedDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbAlignedDimensionPtr Dimension {entity};
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
	auto dOblique {Dimension->oblique()};
	const auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
	auto ocsDefPt1 {FirstExtensionLineStartPoint};
	auto ocsDefPt2 {SecondExtensionLineStartPoint};
	auto ocsDimLinDefPt {DimensionLinePoint};
	const auto vNorm {Dimension->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDefPt1.transformBy(WorldToPlaneTransform);
		ocsDefPt2.transformBy(WorldToPlaneTransform);
		ocsDimLinDefPt.transformBy(WorldToPlaneTransform);
	}
	const auto SavedZCoordinate {ocsDefPt1.z};
	ocsDefPt1.z = ocsDefPt1.z = ocsDimLinDefPt.z = 0.0;
	if (OdNonZero(dOblique)) {
		dOblique = Oda2PI - dOblique;
	} else {
		dOblique = OdaPI2;
	}
	auto v2 {ocsDimLinDefPt - ocsDefPt2};
	if (OdZero(v2.length())) {
		v2 = OdGeVector3d::kYAxis;
		const auto dlv {SecondExtensionLineStartPoint - FirstExtensionLineStartPoint};
		if (OdNonZero(dlv.length())) {
			v2 = dlv.perpVector();
		}
	}
	auto v1 = v2;
	v1.rotateBy(dOblique, OdGeVector3d::kZAxis);
	OdGeLine3d line1;
	OdGeLine3d line2;
	line1.set(ocsDimLinDefPt, v1);
	line2.set(ocsDefPt1, v2);
	OdGePoint3d extLineEnd;
	line1.intersectWith(line2, extLineEnd);
	extLineEnd.z = SavedZCoordinate;
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

OdResult OdDbAlignedDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbAlignedDimensionPtr Dimension {entity};
	auto GripPoint {&gripPoints[indices[0]]};
	auto dimNewPt {*GripPoint};
	auto SavedZCoordinate {0.0};
	OdGeVector3d vMoveTxt;
	auto vNorm {Dimension->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
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
				auto FirstExtensionLineStartPoint {Dimension->xLine1Point()};
				auto SecondExtensionLineStartPoint {Dimension->xLine2Point()};
				auto DimensionLinePoint {Dimension->dimLinePoint()};
				auto ocsXLinePt1(FirstExtensionLineStartPoint);
				auto ocsXLinePt2(SecondExtensionLineStartPoint);
				auto ocsDimLinePt(DimensionLinePoint);
				if (NeedTransform) {
					ocsDimNewPt.transformBy(WorldToPlaneTransform);
					ocsXLinePt1.transformBy(WorldToPlaneTransform);
					ocsXLinePt2.transformBy(WorldToPlaneTransform);
					ocsDimLinePt.transformBy(WorldToPlaneTransform);
				}
				auto SavedZCoordinate {ocsXLinePt1.z};
				ocsDimNewPt.z = 0.0;
				ocsXLinePt1.z = ocsXLinePt2.z = ocsDimLinePt.z = 0.0;
				auto Oblique {Dimension->oblique()};
				auto v1 {OdGeVector3d::kXAxis};
				auto v2 {OdGeVector3d::kYAxis};
				if (!OdNonZero(Oblique)) {
					Oblique = OdaPI2;
				}
				v1 = v2 = ocsXLinePt2 - ocsXLinePt1;
				v2.rotateBy(Oblique, OdGeVector3d::kZAxis);
				OdGeLine3d line1;
				OdGeLine3d line2;
				line1.set(ocsDimLinePt, v1);
				line2.set(ocsXLinePt1, v2);
				OdGePoint3d extLineEnd;
				line1.intersectWith(line2, extLineEnd);
				vMoveTxt = extLineEnd - ocsDimNewPt;
				line1.set(ocsDimNewPt, v1);
				line2.set(ocsDimLinePt, v2);
				line1.intersectWith(line2, ocsDimLinePt);
				dimNewPt = ocsDimLinePt;
				dimNewPt.z = SavedZCoordinate;
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
							ocsDimLinePt.transformBy(WorldToPlaneTransform);
						}
						ocsDimNewPt.z = 0.0;
						ocsDimNewPt.z = 0.0;
						vMoveTxt = ocsDimLinePt - ocsDimNewPt;
					}
					auto dimTextPt {gripPoints[4]};
					auto ocsDimTextPt {dimTextPt};
					if (NeedTransform) {
						ocsDimTextPt.transformBy(WorldToPlaneTransform);
					}
					auto SavedZCoordinate {ocsDimTextPt.z};
					ocsDimTextPt.z = 0.0;
					auto ocsDimNewTextPt {ocsDimTextPt - vMoveTxt};
					auto dimNewTextPt {ocsDimNewTextPt};
					dimNewTextPt.z = SavedZCoordinate;
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
					SavedZCoordinate = ocsDimTextPt.z;
					ocsDimTextPt.z = 0.0;
					auto ocsDimNewTextPt {ocsDimTextPt - v};
					auto dimNewTextPt {ocsDimNewTextPt};
					dimNewTextPt.z = SavedZCoordinate;
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
