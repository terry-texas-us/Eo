#include <OdaCommon.h>
#include <DbOrdinateDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "DbOrdinateDimGripPoints.h"

OdResult OdDbOrdinateDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 4);
	OdDbOrdinateDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + 0] = Dimension->definingPoint();
	gripPoints[GripPointsSize + 1] = Dimension->leaderEndPoint();
	gripPoints[GripPointsSize + 2] = Dimension->origin();
	gripPoints[GripPointsSize + 3] = Dimension->textPosition();
	return eOk;
}

OdResult OdDbOrdinateDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool /*bStretch*/) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbOrdinateDimensionPtr Dimension {entity};
	auto dimDefiningPt {Dimension->definingPoint()};
	auto dimLeaderEndPt {Dimension->leaderEndPoint()};
	auto TextPosition {Dimension->textPosition()};
	const auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
	auto ocsDimDefiningPt {dimDefiningPt};
	auto ocsDimLeaderEndPt {dimLeaderEndPt};
	auto ocsDimTextPt {TextPosition};
	const auto vNorm {Dimension->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimDefiningPt.transformBy(WorldToPlaneTransform);
		ocsDimLeaderEndPt.transformBy(WorldToPlaneTransform);
		ocsDimTextPt.transformBy(WorldToPlaneTransform);
	}
	const auto SavedZCoordinate {ocsDimDefiningPt.z};
	ocsDimDefiningPt.z = ocsDimLeaderEndPt.z = ocsDimTextPt.z = 0.0;
	auto GripPoint = &gripPoints[indices[0]];
	auto ocsDimNewPt {*GripPoint};
	auto dimNewPt {ocsDimNewPt};
	if (NeedTransform) {
		ocsDimNewPt.transformBy(WorldToPlaneTransform);
	}
	ocsDimNewPt.z = 0.0;
	auto v1 {ocsDimTextPt - ocsDimLeaderEndPt};
	for (auto Index : indices) {
		GripPoint = &gripPoints[Index];
		dimNewPt = *GripPoint;
		ocsDimNewPt = dimNewPt;
		switch (Index) {
			case 0:
				Dimension->setDefiningPoint(dimNewPt);
				break;
			case 1:
				v1.normalize();
				v1 *= OdGeVector3d(ocsDimTextPt - ocsDimLeaderEndPt).length();
				ocsDimTextPt = ocsDimNewPt + v1;
				ocsDimTextPt.z = SavedZCoordinate;
				ocsDimNewPt.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsDimTextPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
					ocsDimNewPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
				}
				if (!Dimension->isUsingDefaultTextPosition() && Dimension->dimtmove() == 2) {
					Dimension->setLeaderEndPoint(ocsDimNewPt);
				} else {
					Dimension->setTextPosition(ocsDimTextPt);
				}
				break;
			case 2:
				Dimension->setOrigin(dimNewPt);
				break;
			case 3:
				v1.normalize();
				v1 *= OdGeVector3d(ocsDimTextPt - ocsDimLeaderEndPt).length();
				ocsDimLeaderEndPt = ocsDimNewPt - v1;
				ocsDimNewPt.z = SavedZCoordinate;
				ocsDimLeaderEndPt.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsDimNewPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
					ocsDimLeaderEndPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
				}
				Dimension->setTextPosition(ocsDimNewPt);
				if (Dimension->isUsingDefaultTextPosition() && Dimension->dimtmove() != 2) {
					Dimension->setLeaderEndPoint(ocsDimLeaderEndPt);
				}
				break;
			default: ;
		}
	}
	return eOk;
}
