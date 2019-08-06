#include <OdaCommon.h>
#include "DbCircleGripPoints.h"
#include "OdGripPointsModule.h"

// Returns 5 Points: center + 4 points on Circle
OdResult OdDbCircleGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbCirclePtr Circle {entity};
	const auto Size {gripPoints.size()};
	gripPoints.resize(Size + 5);
	gripPoints[Size + 0] = Circle->center();
	Circle->getPointAtParam(0, gripPoints[Size + 1]); // 1 - right  (0)
	Circle->getPointAtParam(OdaPI, gripPoints[Size + 2]); // 2 - left (pi)
	Circle->getPointAtParam(OdaPI2, gripPoints[Size + 3]); // 3 - top (pi/2)
	Circle->getPointAtParam(OdaPI + OdaPI2, gripPoints[Size + 4]); // 4 - bottom (pi + pi/2)
	if (!OdZero(Circle->thickness())) {
		const auto Extrusion {Circle->normal() * Circle->thickness()};
		for (auto i = 0; i < 5; i++) {
			gripPoints.append(gripPoints[Size + i] + Extrusion);
		}
	}
	return eOk;
}

// Move circle or change radius
OdResult OdDbCircleGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& vOffset) {
	if (indices.size() == 0) { // indices[0] - defines for what point we pull:
		return eOk; // center or other
	} 
	OdDbCirclePtr circle = entity;
	auto bCenter {true};
	auto offset {vOffset};
	if (!projectOffset(circle->database(), circle->normal(), offset))
		// Project offset on entity's plane in view direction
	{
		circle->setCenter(circle->center() + offset);
		// View direction is perpendicular to normal
		return eOk; // Move the circle
	}
	for (unsigned i = 0; i < indices.size(); i++) {
		if (indices[i] % 5 == 0) // point center - move circle
		{
			if (bCenter) // move center only one time
			{
				circle->setCenter(circle->center() + offset);
				bCenter = false;
			}
		} else // change radius
		{
			OdGePoint3dArray gripPoints;
			circle->getGripPoints(gripPoints);
			auto point {gripPoints[indices[i] % 5] + offset};
			circle->setRadius(circle->center().distanceTo(point));
		}
	}
	return eOk;
}

// Cannot be stretched
OdResult OdDbCircleGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDbCirclePtr circle = entity;
	stretchPoints.append(circle->center()); // center
	if (!OdZero(circle->thickness()))       // next center
	{
		stretchPoints.append(circle->center() + circle->normal() * circle->thickness());
	}
	return eOk;
}

OdResult OdDbCircleGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& /*indices_*/, const OdGeVector3d& offset) {
	OdDbCirclePtr circle = entity;
	circle->setCenter(circle->center() + offset);
	return eOk;
}


/**
 * \brief Return snap Points into snapPoints, depending on type objectSnapMode
 * \param entity 
 * \param objectSnapMode 
 * \param pickPoint_  Point, which moves
 * \param lastPoint_  Point, from which draw line
 * \param xWorldToEye 
 * \param snapPoints 
 * \return 
 */
OdResult OdDbCircleGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& pickPoint_, const OdGePoint3d& lastPoint_, const OdGeMatrix3d& /*xWorldToEye*/, OdGePoint3dArray& snapPoints) const {
	OdGePoint3dArray gripPoints;
	const auto Result {getGripPoints(entity, gripPoints)};
	if (Result != eOk || gripPoints.size() < 5) {
		return Result;
	}
	OdDbCirclePtr Circle = entity;
	const auto pickPoint {getPlanePoint(Circle, pickPoint_)}; // recalculated pickPoint and lastPoint in plane of circle
	const auto lastPoint {getPlanePoint(Circle, lastPoint_)};
	auto center {Circle->center()};
	const auto radius {Circle->radius()};
	const auto ptPick {pickPoint - center.asVector()};
	const auto rdPick {pickPoint.distanceTo(center)};
	const auto ptLast {lastPoint - center.asVector()};
	const auto rdLast {lastPoint.distanceTo(center)};
	const auto bThickness {!OdZero(Circle->thickness())};
	const auto vThickness {Circle->normal() * Circle->thickness()};
	switch (objectSnapMode) {
		case OdDb::kOsModeCen: // Center: draw cross
			for (unsigned i = 0; i < gripPoints.size(); i += 5) {
				snapPoints.append(gripPoints[i]);
			}
			break;
		case OdDb::kOsModeQuad: // Quadrant: cursor as square turned on 45 degrees
			for (unsigned i = 1; i < gripPoints.size(); i++) {
				if (i == 5) {
					continue;
				}
				snapPoints.append(gripPoints[i]);
			}
			break;
		case OdDb::kOsModePerp: // Perpendicular: cursor as angle 90
			if (rdLast > 0) {
				auto Point {ptLast * radius / rdLast};
				snapPoints.append(center + Point.asVector());
				snapPoints.append(center - Point.asVector());
				if (bThickness) {
					snapPoints.append(center + Point.asVector() + vThickness);
					snapPoints.append(center - Point.asVector() + vThickness);
				}
			}
			break;
		case OdDb::kOsModeTan: // Tangent: cursor as circle with tangent
			if (rdLast <= radius) {
				break;
			}
			if (ptLast.y != 0) {
				const auto a {1 + pow(ptLast.x, 2) / pow(ptLast.y, 2)};
				const auto b {-2 * pow(radius, 2) * ptLast.x / pow(ptLast.y, 2)};
				const auto c {pow(radius, 4) / pow(ptLast.y, 2) - pow(radius, 2)};
				if (pow(b, 2) - 4 * a * c >= 0) {
					auto x {(-b + sqrt(pow(b, 2) - 4 * a * c)) / (2 * a)};
					auto y {(pow(radius, 2) - ptLast.x * x) / ptLast.y};
					snapPoints.append(center + OdGeVector3d(x, y, 0));
					if (bThickness) {
						snapPoints.append(center + OdGeVector3d(x, y, 0) + vThickness);
					}
					x = (-b - sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);
					y = (pow(radius, 2) - ptLast.x * x) / ptLast.y;
					snapPoints.append(center + OdGeVector3d(x, y, 0));
					if (bThickness) {
						snapPoints.append(center + OdGeVector3d(x, y, 0) + vThickness);
					}
				}
			} else {
				const auto x {pow(radius, 2) / ptLast.x};
				const auto y {sqrt(pow(radius, 2) - pow(x, 2))};
				snapPoints.append(center + OdGeVector3d(x, y, 0));
				snapPoints.append(center + OdGeVector3d(x, -y, 0));
				if (bThickness) {
					snapPoints.append(center + OdGeVector3d(x, y, 0) + vThickness);
					snapPoints.append(center + OdGeVector3d(x, -y, 0) + vThickness);
				}
			}
			break;
		case OdDb::kOsModeNear: // Nearest: cursor ~ hourglasses
			if (rdPick > 0) {
				auto Point {ptPick * radius / rdPick};
				snapPoints.append(center + Point.asVector());
				if (bThickness) {
					snapPoints.append(center + Point.asVector() + vThickness);
				}
			}
			break;
		case OdDb::kOsModeEnd:  // Endpoint:     cursor as square
		case OdDb::kOsModeNode: // Node:         cursor as cross in a square
		case OdDb::kOsModeMid:  // Midpoint:     cursor as triangle
		case OdDb::kOsModeIntersec:
			// Intersection: cursor as intersection in a square
		case OdDb::kOsModeIns:   // Insertion:    -/-
		case OdDb::kOsModePar:   // Parallel:
		case OdDb::kOsModeApint: // Apparent intersection:
			break;                 //               isn't necessary to do
		default:
			break;
	}
	return eOk;
}
