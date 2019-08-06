#include <OdaCommon.h>
#include "DbLineGripPoints.h"
#include <DbLine.h>
#include <Ge/GeLine3d.h>

OdResult OdDbLineGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto Size {gripPoints.size()};
	gripPoints.resize(Size + 3);
	OdDbLinePtr Line = entity;
	Line->getStartPoint(gripPoints[Size + 0]);
	Line->getEndPoint(gripPoints[Size + 1]);
	gripPoints[Size + 2] = gripPoints[Size + 0] + (gripPoints[Size + 1] - gripPoints[Size + 0]) / 2;
	return eOk;
}

OdResult OdDbLineGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto size {indices.size()};
	if (size == 0) {
		return eOk;
	}
	OdDbLinePtr pLine = entity;
	if (size > 1 || indices[0] == 2) {
		pLine->setStartPoint(pLine->startPoint() + offset);
		pLine->setEndPoint(pLine->endPoint() + offset);
	} else if (indices[0] == 0) {
		pLine->setStartPoint(pLine->startPoint() + offset);
	} else if (indices[0] == 1) {
		pLine->setEndPoint(pLine->endPoint() + offset);
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

OdResult OdDbLineGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& /*xWorldToEye*/, OdGePoint3dArray& snapPoints) const {
	OdDbLinePtr line = entity;
	OdGePoint3d start;
	OdGePoint3d end;
	line->getStartPoint(start);
	line->getEndPoint(end);
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			snapPoints.append(start);
			snapPoints.append(end);
			break;
		case OdDb::kOsModeMid:
			snapPoints.append(start + (end - start) / 2);
			break;
		case OdDb::kOsModePerp: {
			const OdGeLine3d l(start, end);
			snapPoints.append(l.evalPoint(l.paramOf(lastPoint)));
		}
		break;
		case OdDb::kOsModeNear: // TODO: project on view plane
			if (!start.isEqualTo(end)) {
				const OdGeLine3d l(start, end);
				const auto p {l.paramOf(pickPoint)};
				if (p > 1) {
					snapPoints.append(end);
				} else if (p < 0) {
					snapPoints.append(start);
				} else {
					snapPoints.append(l.evalPoint(p));
				}
			} else {
				snapPoints.append(start);
			}
			break;
		default:
			break;
	}
	return eOk;
}
