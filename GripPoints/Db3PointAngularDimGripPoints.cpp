#include <OdaCommon.h>
#include <Db3PointAngularDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "Db3PointAngularDimGripPoints.h"

OdResult OdDb3PointAngularDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 5);
	OdDb3PointAngularDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + kFirstExtensionLineStartPoint] = Dimension->xLine1Point();
	gripPoints[GripPointsSize + kSecondExtensionLineStartPoint] = Dimension->xLine2Point();
	gripPoints[GripPointsSize + kCenterPoint] = Dimension->centerPoint();
	gripPoints[GripPointsSize + kArcPoint] = Dimension->arcPoint();
	gripPoints[GripPointsSize + kTextPosition] = Dimension->textPosition();
	return eOk;
}

OdResult OdDb3PointAngularDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, const bool stretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDb3PointAngularDimensionPtr Dimension {entity};
	auto FirstExtensionLineStartPoint {Dimension->xLine1Point()};
	auto SecondExtensionLineStartPoint {Dimension->xLine2Point()};
	auto CenterPoint {Dimension->centerPoint()};
	auto ArcPoint {Dimension->arcPoint()};
	auto TextPosition {Dimension->textPosition()};
	auto NewArcPoint {ArcPoint};
	const auto WorldToPlaneTransform(OdGeMatrix3d::worldToPlane(Dimension->normal()));
	auto ocsFirstExtensionLineStartPoint {FirstExtensionLineStartPoint};
	auto ocsSecondExtensionLineStartPoint {SecondExtensionLineStartPoint};
	auto ocsCenterPoint {CenterPoint};
	auto ocsArcPoint {ArcPoint};
	auto ocsTextPosition {TextPosition};
	auto ocsNewArcPoint {NewArcPoint};
	const auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsFirstExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
		ocsSecondExtensionLineStartPoint.transformBy(WorldToPlaneTransform);
		ocsCenterPoint.transformBy(WorldToPlaneTransform);
		ocsArcPoint.transformBy(WorldToPlaneTransform);
		ocsTextPosition.transformBy(WorldToPlaneTransform);
		ocsTextPosition.transformBy(WorldToPlaneTransform);
		ocsNewArcPoint.transformBy(WorldToPlaneTransform);
	}
	const auto SavedZCoordinate {ocsFirstExtensionLineStartPoint.z};
	ocsFirstExtensionLineStartPoint.z = ocsSecondExtensionLineStartPoint.z = ocsCenterPoint.z = ocsArcPoint.z = ocsTextPosition.z = ocsNewArcPoint.z = 0.0;
	auto GripPoint = &gripPoints[indices[0]];
	auto ocsNewGripPoint {*GripPoint};
	auto NewGripPoint {ocsNewGripPoint};
	if (NeedTransform) {
		ocsNewGripPoint.transformBy(WorldToPlaneTransform);
	}
	ocsNewGripPoint.z = 0.0;
	for (auto i = 0U; i < indices.size(); i++) {
		GripPoint = &gripPoints[indices[i]];
		NewGripPoint = *GripPoint;
		ocsNewGripPoint = NewGripPoint;
		if (indices[i] < kTextPosition && !Dimension->isUsingDefaultTextPosition()) {
			Dimension->useDefaultTextPosition();
		}
		switch (indices[i]) {
			case kFirstExtensionLineStartPoint:
				Dimension->setXLine1Point(NewGripPoint);
				break;
			case kSecondExtensionLineStartPoint:
				Dimension->setXLine2Point(NewGripPoint);
				break;
			case kCenterPoint:
				Dimension->setCenterPoint(NewGripPoint);
				break;
			case kArcPoint:
				Dimension->setArcPoint(NewGripPoint);
				break;
			case kTextPosition: {
				ocsNewArcPoint = ocsCenterPoint - (ocsCenterPoint - ocsNewArcPoint);
				ocsTextPosition = ocsNewGripPoint;
				ocsTextPosition.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsTextPosition.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
				}
				if (indices.size() == 1 || !stretch) {
					Dimension->useSetTextPosition();
				}
				Dimension->setTextPosition(ocsTextPosition);
				break;
			}
			default:
				break;
		}
	}
	return eOk;
}
