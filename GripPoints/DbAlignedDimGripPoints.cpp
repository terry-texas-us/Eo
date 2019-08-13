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
	auto Oblique {Dimension->oblique()};
	const auto Normal {Dimension->normal()};
	const auto NeedTransform {Normal != OdGeVector3d::kZAxis};
	auto ocsFirstExtensionLineStartPoint {FirstExtensionLineStartPoint};
	auto ocsSecondExtensionLineStartPoint {SecondExtensionLineStartPoint};
	auto ocsDimensionLinePoint {DimensionLinePoint};
	if (NeedTransform) {
		const auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Normal)};
		ocsFirstExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
		ocsSecondExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
		ocsDimensionLinePoint.transformBy(WorldToPlaneTransform);
	}
	const auto SavedZCoordinate {ocsFirstExtensionLineStartPoint.z};
	ocsFirstExtensionLineStartPoint.z = ocsSecondExtensionLineStartPoint.z = ocsDimensionLinePoint.z = 0.0;
	if (OdNonZero(Oblique)) {
		Oblique = Oda2PI - Oblique;
	} else {
		Oblique = OdaPI2;
	}
	auto v2 {ocsDimensionLinePoint - ocsSecondExtensionLineStartPoint};
	if (OdZero(v2.length())) {
		v2 = OdGeVector3d::kYAxis;
		const auto dlv {SecondExtensionLineStartPoint - FirstExtensionLineStartPoint};
		if (OdNonZero(dlv.length())) {
			v2 = dlv.perpVector();
		}
	}
	auto v1 {v2};
	v1.rotateBy(Oblique, OdGeVector3d::kZAxis);
	const OdGeLine3d Line1(ocsDimensionLinePoint, v1);
	const OdGeLine3d Line2(ocsFirstExtensionLineStartPoint, v2);
	OdGePoint3d ExtensionLineEnd;
	Line1.intersectWith(Line2, ExtensionLineEnd);
	ExtensionLineEnd.z = SavedZCoordinate;
	if (NeedTransform) {
		ExtensionLineEnd.transformBy(OdGeMatrix3d::planeToWorld(Normal));
	}
	gripPoints[GripPointsSize + kFirstExtensionLineStartPoint] = FirstExtensionLineStartPoint;
	gripPoints[GripPointsSize + kSecondExtensionLineStartPoint] = SecondExtensionLineStartPoint;
	gripPoints[GripPointsSize + kExtensionLineEnd] = ExtensionLineEnd;
	gripPoints[GripPointsSize + kDimensionLinePoint] = DimensionLinePoint;
	gripPoints[GripPointsSize + kTextPosition] = TextPosition;
	if (Dimension->jogSymbolOn()) {
		gripPoints[GripPointsSize + kJogSymbolPosition] = Dimension->jogSymbolPosition();
	}
	return eOk;
}

OdResult OdDbAlignedDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool stretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbAlignedDimensionPtr Dimension {entity};
	auto SavedZCoordinate {0.0};
	OdGeVector3d MoveText;
	auto Normal {Dimension->normal()};
	auto NeedTransform {Normal != OdGeVector3d::kZAxis};
	for (auto i = 0; i < static_cast<int>(indices.size()); i++) {
		auto GripPoint {gripPoints[indices[i]]};
		switch (indices[i]) {
			case kFirstExtensionLineStartPoint:
				Dimension->setXLine1Point(GripPoint);
				break;
			case kSecondExtensionLineStartPoint:
				Dimension->setXLine2Point(GripPoint);
				break;
			case kExtensionLineEnd: {
				auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Normal)};
				auto FirstExtensionLineStartPoint {Dimension->xLine1Point()};
				auto SecondExtensionLineStartPoint {Dimension->xLine2Point()};
				auto DimensionLinePoint {Dimension->dimLinePoint()};
				auto ocsGripPoint {GripPoint};
				auto ocsFirstExtensionLineStartPoint {FirstExtensionLineStartPoint};
				auto ocsSecondExtensionLineStartPoint {SecondExtensionLineStartPoint};
				auto ocsDimensionLinePoint {DimensionLinePoint};
				if (NeedTransform) {
					ocsGripPoint.transformBy(WorldToPlaneTransform);
					ocsFirstExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
					ocsSecondExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
					ocsDimensionLinePoint.transformBy(WorldToPlaneTransform);
				}
				SavedZCoordinate = ocsFirstExtensionLineStartPoint.z;
				ocsGripPoint.z = 0.0;
				ocsFirstExtensionLineStartPoint.z = ocsSecondExtensionLineStartPoint.z = ocsDimensionLinePoint.z = 0.0;
				auto Oblique {Dimension->oblique()};
				auto v1 {OdGeVector3d::kXAxis};
				auto v2 {OdGeVector3d::kYAxis};
				if (!OdNonZero(Oblique)) {
					Oblique = OdaPI2;
				}
				v1 = v2 = ocsSecondExtensionLineStartPoint - ocsFirstExtensionLineStartPoint;
				v2.rotateBy(Oblique, OdGeVector3d::kZAxis);
				OdGeLine3d Line1(ocsDimensionLinePoint, v1);
				OdGeLine3d Line2(ocsFirstExtensionLineStartPoint, v2);
				OdGePoint3d ExtensionLineEnd;
				Line1.intersectWith(Line2, ExtensionLineEnd);
				MoveText = ExtensionLineEnd - ocsGripPoint;
				Line1.set(ocsGripPoint, v1);
				Line2.set(ocsDimensionLinePoint, v2);
				Line1.intersectWith(Line2, ocsDimensionLinePoint);
				GripPoint = ocsDimensionLinePoint;
				GripPoint.z = SavedZCoordinate;
				if (NeedTransform) {
					GripPoint.transformBy(OdGeMatrix3d::planeToWorld(Normal));
				}
			}
			case kDimensionLinePoint:
				if (!Dimension->isUsingDefaultTextPosition() && Dimension->dimtmove() == 0) {
					auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Normal)};
					if (indices[0] == kDimensionLinePoint) {
						auto ocsDimensionLinePoint(Dimension->dimLinePoint());
						auto ocsGripPoint {GripPoint};
						if (NeedTransform) {
							ocsDimensionLinePoint.transformBy(WorldToPlaneTransform);
							ocsGripPoint.transformBy(WorldToPlaneTransform);
						}
						ocsDimensionLinePoint.z = 0.0;
						ocsGripPoint.z = 0.0;
						MoveText = ocsDimensionLinePoint - ocsGripPoint;
					}
					auto TextPosition {gripPoints[kTextPosition]};
					auto ocsTextPosition {TextPosition};
					if (NeedTransform) {
						ocsTextPosition.transformBy(WorldToPlaneTransform);
					}
					SavedZCoordinate = ocsTextPosition.z;
					ocsTextPosition.z = 0.0;
					auto ocsNewTextPosition {ocsTextPosition - MoveText};
					auto NewTextPosition {ocsNewTextPosition};
					NewTextPosition.z = SavedZCoordinate;
					if (NeedTransform) {
						NewTextPosition.transformBy(OdGeMatrix3d::planeToWorld(Normal));
					}
					Dimension->setTextPosition(NewTextPosition);
				} else {
					Dimension->setDimLinePoint(GripPoint);
				}
				break;
			case kTextPosition:
				Dimension->setTextPosition(GripPoint);
				if (indices.size() == 1 || !stretch) {
					Dimension->useSetTextPosition();
				}
				break;
			case kJogSymbolPosition:
				if (!Dimension->isUsingDefaultTextPosition() && Dimension->dimtmove() == 0) {
					auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Normal)};
					auto ocsJogSymbolPosition(Dimension->jogSymbolPosition());
					auto ocsGripPoint {GripPoint};
					auto SecondExtensionLineStartPoint {Dimension->xLine2Point()};
					auto DimensionLinePoint {Dimension->dimLinePoint()};
					if (NeedTransform) {
						ocsJogSymbolPosition.transformBy(WorldToPlaneTransform);
						ocsGripPoint.transformBy(WorldToPlaneTransform);
						SecondExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
						DimensionLinePoint.transformBy(WorldToPlaneTransform);
					}
					ocsJogSymbolPosition.z = 0.0;
					ocsGripPoint.z = 0.0;
					SecondExtensionLineStartPoint.z = DimensionLinePoint.z = 0.0;
					auto v {DimensionLinePoint - SecondExtensionLineStartPoint};
					MoveText = ocsJogSymbolPosition - ocsGripPoint;
					if (v.length()) {
						v.normalize();
					}
					v *= MoveText.length() * cos(v.angleTo(MoveText));
					auto TextPosition {gripPoints[4]};
					auto ocsTextPosition {TextPosition};
					if (NeedTransform) {
						ocsTextPosition.transformBy(WorldToPlaneTransform);
					}
					SavedZCoordinate = ocsTextPosition.z;
					ocsTextPosition.z = 0.0;
					auto ocsNewTextPosition {ocsTextPosition - v};
					auto NewTextPosition {ocsNewTextPosition};
					NewTextPosition.z = SavedZCoordinate;
					if (NeedTransform) {
						NewTextPosition.transformBy(OdGeMatrix3d::planeToWorld(Normal));
					}
					Dimension->setTextPosition(NewTextPosition);
				} else {
					Dimension->setDimLinePoint(GripPoint);
				}
				Dimension->setJogSymbolPosition(GripPoint);
				break;
			default:
				break;
		}
	}
	return eOk;
}
