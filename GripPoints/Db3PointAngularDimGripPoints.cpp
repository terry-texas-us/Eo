#include <OdaCommon.h>
#include <Db3PointAngularDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "Db3PointAngularDimGripPoints.h"

OdResult OdDb3PointAngularDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 5);
	OdDb3PointAngularDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + 0] = Dimension->xLine1Point();
	gripPoints[GripPointsSize + 1] = Dimension->xLine2Point();
	gripPoints[GripPointsSize + 2] = Dimension->centerPoint();
	gripPoints[GripPointsSize + 3] = Dimension->arcPoint();
	gripPoints[GripPointsSize + 4] = Dimension->textPosition();
	return eOk;
}

OdResult OdDb3PointAngularDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
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
	auto ocsDimCenterPt {CenterPoint};
	auto ocsDimArcPt {ArcPoint};
	auto ocsDimTextPt {TextPosition};
	auto ocsDimArcNewPt {NewArcPoint};
	const auto Normal {Dimension->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimLine1Pt.transformBy(WorldToPlaneTransform);
		ocsDimLine2Pt.transformBy(WorldToPlaneTransform);
		ocsDimCenterPt.transformBy(WorldToPlaneTransform);
		ocsDimArcPt.transformBy(WorldToPlaneTransform);
		ocsDimTextPt.transformBy(WorldToPlaneTransform);
		ocsDimTextPt.transformBy(WorldToPlaneTransform);
		ocsDimArcNewPt.transformBy(WorldToPlaneTransform);
	}
	const auto SavedZCoordinate {ocsDimLine1Pt.z};
	ocsDimLine1Pt.z = ocsDimLine2Pt.z = ocsDimCenterPt.z = ocsDimArcPt.z = ocsDimTextPt.z = ocsDimArcNewPt.z = 0.0;
	auto GripPoint = &gripPoints[indices[0]];
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
		if (indices[i] < 4 && !Dimension->isUsingDefaultTextPosition()) {
			Dimension->useDefaultTextPosition();
		}
		switch (indices[i]) {
			case 0:
				Dimension->setXLine1Point(dimNewPt);
				break;
			case 1:
				Dimension->setXLine2Point(dimNewPt);
				break;
			case 2:
				Dimension->setCenterPoint(dimNewPt);
				break;
			case 4: {
				auto v4 {ocsDimCenterPt - ocsDimArcNewPt};
				ocsDimArcNewPt = ocsDimCenterPt - v4;
				ocsDimTextPt = ocsDimNewPt;
				ocsDimTextPt.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsDimTextPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
				}
				if (indices.size() == 1 || !bStretch) {
					Dimension->useSetTextPosition();
				}
				Dimension->setTextPosition(ocsDimTextPt);
				break;
			}
			case 3: {
				Dimension->setArcPoint(dimNewPt);
				break;
			}
			default:
				break;
		}
	}
	return eOk;
}
