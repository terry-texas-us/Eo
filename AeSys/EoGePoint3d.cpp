#include "stdafx.h"

bool ContainmentOf(const OdGePoint3d& point, const OdGePoint3d& lowerLeftPoint, const OdGePoint3d& upperRightPoint) noexcept {
	auto RelativeTolerance {DBL_EPSILON + fabs(DBL_EPSILON * point.x)};
	if (lowerLeftPoint.x > point.x + RelativeTolerance || upperRightPoint.x < point.x - RelativeTolerance) {
		return false;
	}
	RelativeTolerance = DBL_EPSILON + fabs(DBL_EPSILON * point.y);
	if (lowerLeftPoint.y > point.y + RelativeTolerance || upperRightPoint.y < point.y - RelativeTolerance) {
		return false;
	}
	return true;
}

OdGePoint3d ProjectToward(const OdGePoint3d& fromPoint, const OdGePoint3d& toPoint, const double distance) {
	auto Direction {toPoint - fromPoint};
	if (!Direction.isZeroLength()) {
		Direction *= distance / Direction.length();
	}
	return fromPoint + Direction;
}

int RelationshipToRectangleOf(const OdGePoint3d& point, const OdGePoint3d& lowerLeftPoint, const OdGePoint3d& upperRightPoint) noexcept {
	auto returnValue {0};
	if (point.y > upperRightPoint.y + DBL_EPSILON) {
		returnValue = 1;
	} else if (point.y < lowerLeftPoint.y - DBL_EPSILON) {
		returnValue = 2;
	}
	if (point.x > upperRightPoint.x + DBL_EPSILON) {
		returnValue |= 4;
	} else if (point.x < lowerLeftPoint.x - DBL_EPSILON) {
		returnValue |= 8;
	}
	return returnValue;
}
