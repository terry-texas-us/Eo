#include <OdaCommon.h>
#include "DbLineGripPoints.h"
#include <DbLine.h>
#include <Ge/GeLine3d.h>

OdResult OdDbLineGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 3);
	OdDbLinePtr Line {entity};
	const auto StartPoint {Line->startPoint()};
	const auto EndPoint {Line->endPoint()};
	gripPoints[GripPointsSize + kStartPoint] = StartPoint;
	gripPoints[GripPointsSize + kEndPoint] = EndPoint;
	gripPoints[GripPointsSize + kMidPoint] = StartPoint + (EndPoint - StartPoint) / 2.0;
	return eOk;
}

OdResult OdDbLineGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbLinePtr Line {entity};
	if (indices.size() > 1 || indices[0] == kMidPoint) {
		Line->setStartPoint(Line->startPoint() + offset);
		Line->setEndPoint(Line->endPoint() + offset);
	} else if (indices[0] == kStartPoint) {
		Line->setStartPoint(Line->startPoint() + offset);
	} else if (indices[0] == kEndPoint) {
		Line->setEndPoint(Line->endPoint() + offset);
	}
	return eOk;
}

OdResult OdDbLineGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	const auto Result {getGripPoints(entity, stretchPoints)};
	if (Result == eOk) {
		stretchPoints.resize(stretchPoints.size() - 1);
	}
	return Result;
}

OdResult OdDbLineGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbLineGripPointsPE::getOsnapPoints(const OdDbEntity* entity, const OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdDbLinePtr Line {entity};
	const auto StartPoint {Line->startPoint()};
	const auto EndPoint {Line->endPoint()};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			snapPoints.append(StartPoint);
			snapPoints.append(EndPoint);
			break;
		case OdDb::kOsModeMid:
			snapPoints.append(StartPoint + (EndPoint - StartPoint) / 2.0);
			break;
		case OdDb::kOsModePerp: {
			const OdGeLine3d InfiniteLine(StartPoint, EndPoint);
			snapPoints.append(InfiniteLine.evalPoint(InfiniteLine.paramOf(lastPoint)));
			break;
		}
		case OdDb::kOsModeNear:
			// <tas="project on view plane"/>
			if (!StartPoint.isEqualTo(EndPoint)) {
				const OdGeLine3d InfiniteLine(StartPoint, EndPoint);
				const auto PickParameter {InfiniteLine.paramOf(pickPoint)};
				if (PickParameter > 1) {
					snapPoints.append(EndPoint);
				} else if (PickParameter < 0) {
					snapPoints.append(StartPoint);
				} else {
					snapPoints.append(InfiniteLine.evalPoint(PickParameter));
				}
			} else {
				snapPoints.append(StartPoint);
			}
			break;
		default:
			break;
	}
	return eOk;
}
