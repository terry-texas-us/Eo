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

OdResult OdDb3PointAngularDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool stretch) {
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
	auto ocsDimLine1Pt {FirstExtensionLineStartPoint};
	auto ocsDimLine2Pt {SecondExtensionLineStartPoint};
	auto ocsCenterPoint {CenterPoint};
	auto ocsDimArcPt {ArcPoint};
	auto ocsDimTextPt {TextPosition};
	auto ocsNewArcPoint {NewArcPoint};
	const auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimLine1Pt.transformBy(WorldToPlaneTransform);
		ocsDimLine2Pt.transformBy(WorldToPlaneTransform);
		ocsCenterPoint.transformBy(WorldToPlaneTransform);
		ocsDimArcPt.transformBy(WorldToPlaneTransform);
		ocsDimTextPt.transformBy(WorldToPlaneTransform);
		ocsDimTextPt.transformBy(WorldToPlaneTransform);
		ocsNewArcPoint.transformBy(WorldToPlaneTransform);
	}
	const auto SavedZCoordinate {ocsDimLine1Pt.z};
	ocsDimLine1Pt.z = ocsDimLine2Pt.z = ocsCenterPoint.z = ocsDimArcPt.z = ocsDimTextPt.z = ocsNewArcPoint.z = 0.0;
	auto GripPoint = &gripPoints[indices[0]];
	auto ocsDimNewPt {*GripPoint};
	auto dimNewPt {ocsDimNewPt};
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
				Dimension->setXLine1Point(dimNewPt);
				break;
			case kSecondExtensionLineStartPoint:
				Dimension->setXLine2Point(dimNewPt);
				break;
			case kCenterPoint:
				Dimension->setCenterPoint(dimNewPt);
				break;
			case kArcPoint:
				Dimension->setArcPoint(dimNewPt);
				break;
			case kTextPosition: {
				ocsNewArcPoint = ocsCenterPoint - (ocsCenterPoint - ocsNewArcPoint);
				ocsDimTextPt = ocsDimNewPt;
				ocsDimTextPt.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsDimTextPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
				}
				if (indices.size() == 1 || !stretch) {
					Dimension->useSetTextPosition();
				}
				Dimension->setTextPosition(ocsDimTextPt);
				break;
			}
			default:
				break;
		}
	}
	return eOk;
}
