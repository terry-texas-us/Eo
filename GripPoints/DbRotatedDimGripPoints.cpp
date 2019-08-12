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
	auto ocsFirstExtensionLineStartPoint {FirstExtensionLineStartPoint};
	auto ocsDimensionLinePoint {DimensionLinePoint};
	const auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsFirstExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
		ocsDimensionLinePoint.transformBy(WorldToPlaneTransform);
	}
	const auto SavedZCoordinate {ocsFirstExtensionLineStartPoint.z};
	ocsFirstExtensionLineStartPoint.z = ocsDimensionLinePoint.z = 0.0;
	auto v1 {OdGeVector3d::kXAxis};
	v1.rotateBy(Rotation, OdGeVector3d::kZAxis);
	auto v2 {OdGeVector3d::kYAxis};
	if (OdNonZero(Oblique)) {
		Oblique = Oblique - OdaPI2;
	}
	v2.rotateBy(Rotation + Oblique, OdGeVector3d::kZAxis);
	const OdGeLine3d Line1(ocsDimensionLinePoint, v1);
	const OdGeLine3d Line2(ocsFirstExtensionLineStartPoint, v2);
	OdGePoint3d ExtensionLineEnd;
	Line1.intersectWith(Line2, ExtensionLineEnd);
	ExtensionLineEnd.z = SavedZCoordinate;
	if (NeedTransform) {
		ExtensionLineEnd.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
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

OdResult OdDbRotatedDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool stretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbRotatedDimensionPtr Dimension {entity};
	auto SavedZCoordinate {0.0};
	OdGeVector3d MoveText;
	auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
	}
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
				auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
				auto ocsDimNewPt(GripPoint);
				auto DimensionLinePoint {Dimension->dimLinePoint()};
				auto ocsDimLinePt(DimensionLinePoint);
				auto FirstExtensionLineStartPoint {Dimension->xLine1Point()};
				auto ocsDefPt1(FirstExtensionLineStartPoint);
				if (NeedTransform) {
					ocsDimNewPt.transformBy(WorldToPlaneTransform);
					ocsDimLinePt.transformBy(WorldToPlaneTransform);
					ocsDefPt1.transformBy(WorldToPlaneTransform);
				}
				SavedZCoordinate = ocsDefPt1.z;
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
				OdGeLine3d Line1(ocsDimLinePt, v1);
				OdGeLine3d Line2(ocsDefPt1, v2);
				OdGePoint3d ExtensionLineEnd;
				Line1.intersectWith(Line2, ExtensionLineEnd);
				MoveText = ExtensionLineEnd - ocsDimNewPt;
				Line1.set(ocsDimNewPt, v1);
				Line2.set(ocsDimLinePt, v2);
				Line1.intersectWith(Line2, ocsDimNewPt);
				GripPoint = ocsDimNewPt;
				GripPoint.z = SavedZCoordinate;
				if (NeedTransform) {
					GripPoint.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
				}
			}
			case kDimensionLinePoint:
				if (!Dimension->isUsingDefaultTextPosition() && Dimension->dimtmove() == 0) {
					auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
					if (indices[0] == kDimensionLinePoint) {
						auto ocsDimLinePt(Dimension->dimLinePoint());
						auto ocsDimNewPt {GripPoint};
						if (NeedTransform) {
							ocsDimLinePt.transformBy(WorldToPlaneTransform);
							ocsDimNewPt.transformBy(WorldToPlaneTransform);
						}
						ocsDimLinePt.z = 0.0;
						ocsDimNewPt.z = 0.0;
						MoveText = ocsDimLinePt - ocsDimNewPt;
					}
					auto TextPosition {gripPoints[4]};
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
						NewTextPosition.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
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
					auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
					auto ocsJogSymbolPosition(Dimension->jogSymbolPosition());
					auto ocsDimNewPt {GripPoint};
					auto SecondExtensionLineStartPoint {Dimension->xLine2Point()};
					auto DimensionLinePoint {Dimension->dimLinePoint()};
					if (NeedTransform) {
						ocsJogSymbolPosition.transformBy(WorldToPlaneTransform);
						ocsDimNewPt.transformBy(WorldToPlaneTransform);
						SecondExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
						DimensionLinePoint.transformBy(WorldToPlaneTransform);
					}
					ocsJogSymbolPosition.z = 0.0;
					ocsDimNewPt.z = 0.0;
					SecondExtensionLineStartPoint.z = DimensionLinePoint.z = 0.0;
					auto v {DimensionLinePoint - SecondExtensionLineStartPoint};
					MoveText = ocsJogSymbolPosition - ocsDimNewPt;
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
						NewTextPosition.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
					}
					Dimension->setTextPosition(NewTextPosition);
				} else {
					Dimension->setDimLinePoint(GripPoint);
				}
				Dimension->setJogSymbolPosition(GripPoint);
				break;
			default: ;
		}
	}
	return eOk;
}
