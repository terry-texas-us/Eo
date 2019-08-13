#include <OdaCommon.h>
#include <DbOrdinateDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "DbOrdinateDimGripPoints.h"

OdResult OdDbOrdinateDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 4);
	OdDbOrdinateDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + kDefiningPoint] = Dimension->definingPoint();
	gripPoints[GripPointsSize + kLeaderEndPoint] = Dimension->leaderEndPoint();
	gripPoints[GripPointsSize + kOrigin] = Dimension->origin();
	gripPoints[GripPointsSize + kTextPosition] = Dimension->textPosition();
	return eOk;
}

OdResult OdDbOrdinateDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool /*stretch*/) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbOrdinateDimensionPtr Dimension {entity};
	auto DefiningPoint {Dimension->definingPoint()};
	auto LeaderEndPoint {Dimension->leaderEndPoint()};
	auto TextPosition {Dimension->textPosition()};
	const auto Normal {Dimension->normal()};
	const auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Normal)};
	auto ocsDefiningPoint {DefiningPoint};
	auto ocsLeaderEndPoint {LeaderEndPoint};
	auto ocsTextPosition {TextPosition};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDefiningPoint.transformBy(WorldToPlaneTransform);
		ocsLeaderEndPoint.transformBy(WorldToPlaneTransform);
		ocsTextPosition.transformBy(WorldToPlaneTransform);
	}
	const auto SavedZCoordinate {ocsDefiningPoint.z};
	ocsDefiningPoint.z = ocsLeaderEndPoint.z = ocsTextPosition.z = 0.0;
	auto GripPoint = &gripPoints[indices[0]];
	auto ocsNewGripPoint {*GripPoint};
	auto NewGripPoint {ocsNewGripPoint};
	if (NeedTransform) {
		ocsNewGripPoint.transformBy(WorldToPlaneTransform);
	}
	ocsNewGripPoint.z = 0.0;
	auto v1 {ocsTextPosition - ocsLeaderEndPoint};
	for (auto Index : indices) {
		GripPoint = &gripPoints[Index];
		NewGripPoint = *GripPoint;
		ocsNewGripPoint = NewGripPoint;
		switch (Index) {
			case kDefiningPoint:
				Dimension->setDefiningPoint(NewGripPoint);
				break;
			case kLeaderEndPoint:
				v1.normalize();
				v1 *= OdGeVector3d(ocsTextPosition - ocsLeaderEndPoint).length();
				ocsTextPosition = ocsNewGripPoint + v1;
				ocsTextPosition.z = SavedZCoordinate;
				ocsNewGripPoint.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsTextPosition.transformBy(OdGeMatrix3d::planeToWorld(Normal));
					ocsNewGripPoint.transformBy(OdGeMatrix3d::planeToWorld(Normal));
				}
				if (!Dimension->isUsingDefaultTextPosition() && Dimension->dimtmove() == 2) {
					Dimension->setLeaderEndPoint(ocsNewGripPoint);
				} else {
					Dimension->setTextPosition(ocsTextPosition);
				}
				break;
			case kOrigin:
				Dimension->setOrigin(NewGripPoint);
				break;
			case kTextPosition:
				v1.normalize();
				v1 *= OdGeVector3d(ocsTextPosition - ocsLeaderEndPoint).length();
				ocsLeaderEndPoint = ocsNewGripPoint - v1;
				ocsNewGripPoint.z = SavedZCoordinate;
				ocsLeaderEndPoint.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsNewGripPoint.transformBy(OdGeMatrix3d::planeToWorld(Normal));
					ocsLeaderEndPoint.transformBy(OdGeMatrix3d::planeToWorld(Normal));
				}
				Dimension->setTextPosition(ocsNewGripPoint);
				if (Dimension->isUsingDefaultTextPosition() && Dimension->dimtmove() != 2) {
					Dimension->setLeaderEndPoint(ocsLeaderEndPoint);
				}
				break;
			default: ;
		}
	}
	return eOk;
}
