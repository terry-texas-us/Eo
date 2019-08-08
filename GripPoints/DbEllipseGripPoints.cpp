#include <OdaCommon.h>
#include "DbEllipseGripPoints.h"
#include "OdGripPointsModule.h"

// Returns 5 Points: center + 4 points on Circle
OdResult OdDbEllipseGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbEllipsePtr circle = entity;
	const auto size {gripPoints.size()};
	gripPoints.resize(size + 5);
	gripPoints[size + 0] = circle->center();                            // 0 - center
	circle->getPointAtParam(circle->startAngle(), gripPoints[size + 1]);             // 1 - right  (0)
	circle->getPointAtParam(circle->startAngle() + OdaPI, gripPoints[size + 2]);             // 2 - left   (pi)
	circle->getPointAtParam(circle->startAngle() + OdaPI2, gripPoints[size + 3]);             // 3 - top    (pi/2)
	circle->getPointAtParam(circle->startAngle() + OdaPI + OdaPI2, gripPoints[size + 4]);     // 4 - bottom (pi + pi/2)
	//if (!OdZero(circle->thickness()))
	//{
	//  OdGeVector3d vExtrusion = circle->normal() * circle->thickness();
	//  for (int i = 0; i<5; i++)
	//    gripPoints.append(gripPoints[size + i] + vExtrusion);
	//}
	return eOk;
}

// Move circle or change radius
OdResult OdDbEllipseGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& vOffset) {
	if (indices.size() == 0) { // indices[0] - defines for what point we pull:
		return eOk; // center or other
	}
	OdDbEllipsePtr circle = entity;
	auto Center {true};
	auto offset {vOffset};
	if (!ProjectOffset(circle->database(), circle->normal(), offset))   // Project offset on entity's plane in view direction
	{
		circle->setCenter(circle->center() + offset);                   // View direction is perpendicular to normal
		return eOk;                                                       // Move the circle
	}
	for (unsigned i = 0; i < indices.size(); i++) {
		if (indices[i] % 5 == 0)                                          // point center - move circle
		{
			if (Center)                                                  // move center only one time
			{
				circle->setCenter(circle->center() + offset);
				Center = false;
			}
		} else                                                            // change radius
		{
			OdGePoint3dArray gripPoints;
			circle->getGripPoints(gripPoints);
			const auto Dist1 {gripPoints[1].distanceTo(gripPoints[2])};
			const auto Dist2 {gripPoints[3].distanceTo(gripPoints[4])};
			auto point {gripPoints[indices[i] % 5] + offset};
			auto Normal {circle->normal()};
			auto Center {circle->center()};
			auto newDist {circle->center().distanceTo(point)};
			newDist = newDist < 1.e-10 ? 1.e-10 : newDist;
			auto major {circle->majorAxis()};
			auto minor {circle->minorAxis()};
			auto radiusRatio {circle->radiusRatio()};
			auto startAngle {circle->startAngle()};
			auto endAngle {circle->endAngle()};
			auto SwapMajorMinor {false};
			if (indices[i] < 3 && Dist1 > Dist2 || indices[i] >= 3 && Dist1 < Dist2) {
				radiusRatio = minor.length() / newDist;
				major.setLength(newDist);
				if (radiusRatio > 1) {
					radiusRatio = 1.0 / radiusRatio;
					major = minor;
					SwapMajorMinor = true;
				}
			} else {
				radiusRatio = newDist / major.length();
				if (radiusRatio > 1) {
					radiusRatio = major.length() / newDist;
					major = minor;
					major.setLength(newDist);
					SwapMajorMinor = true;
				}
			}
			try {
				if (SwapMajorMinor) {
					startAngle = startAngle - OdaPI2;
					endAngle = endAngle - OdaPI2;
					if (startAngle < 0) {
						startAngle = startAngle + Oda2PI;
						endAngle = endAngle + Oda2PI;
					}
				}
				circle->set(Center, Normal, major, radiusRatio, startAngle, endAngle);
			} catch (...) { }
		}
	}
	return eOk;
}

// Cannot be stretched
OdResult OdDbEllipseGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDbEllipsePtr circle = entity;
	stretchPoints.append(circle->center());                             // center
	//if (!OdZero(circle->thickness()))                                   // next center
	//{
	//  stretchPoints.append(circle->center() + circle->normal()*circle->thickness());
	//}
	return eOk;
}

OdResult OdDbEllipseGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& /*indices_*/, const OdGeVector3d& offset) {
	OdDbEllipsePtr circle = entity;
	circle->setCenter(circle->center() + offset);
	return eOk;
}

/**
 * \brief Return snap Points into snapPoints, depending on type objectSnapMode
 * \param entity 
 * \param objectSnapMode 
 * \param selectionMarker 
 * \param pickPoint_         Point, which moves
 * \param lastPoint_         Point, from which draw line
 * \param worldToEyeTransform 
 * \param snapPoints 
 * \return 
 */
OdResult OdDbEllipseGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint_, const OdGePoint3d& lastPoint_, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdGePoint3dArray gripPoints;
	const auto Result {getGripPoints(entity, gripPoints)};
	if (Result != eOk || gripPoints.size() < 5) {
		return Result;
	}
	OdDbEllipsePtr Ellipse = entity;
	const auto pickPoint {GetPlanePoint(Ellipse, pickPoint_)}; // recalculated pickPoint and lastPoint in plane of circle
	const auto lastPoint {GetPlanePoint(Ellipse, lastPoint_)};
	auto Center {Ellipse->center()};
	const auto Radius {0.0}; // circle->radius();
	const auto ptPick {pickPoint - Center.asVector()};
	const auto rdPick {pickPoint.distanceTo(Center)};
	const auto ptLast {lastPoint - Center.asVector()};
	const auto rdLast {lastPoint.distanceTo(Center)};
	const auto bThickness {false}; //OdZero(circle->thickness());
	OdGeVector3d vThickness; //= circle->normal()*circle->thickness();
	switch (objectSnapMode) {
		case OdDb::kOsModeCen:                   // Center: draw cross
			for (unsigned i = 0; i < gripPoints.size(); i += 5) {
				snapPoints.append(gripPoints[i]);
			}
			break;
		case OdDb::kOsModeQuad:                  // Quadrant: cursor as square turned on 45 degrees
			for (unsigned i = 1; i < gripPoints.size(); i++) {
				if (i == 5) {
					continue;
				}
				snapPoints.append(gripPoints[i]);
			}
			break;
		case OdDb::kOsModePerp:                  // Perpendicular: cursor as angle 90
			if (rdLast > 0) {
				auto Point {ptLast * Radius / rdLast};
				snapPoints.append(Center + Point.asVector());
				snapPoints.append(Center - Point.asVector());
				if (bThickness) {
					snapPoints.append(Center + Point.asVector() + vThickness);
					snapPoints.append(Center - Point.asVector() + vThickness);
				}
			}
			break;
		case OdDb::kOsModeTan:                   // Tangent: cursor as circle with tangent
			if (rdLast <= Radius) {
				break;
			}
			if (ptLast.y != 0) {
				const auto a {1 + pow(ptLast.x, 2) / pow(ptLast.y, 2)};
				const auto b {-2 * pow(Radius, 2) * ptLast.x / pow(ptLast.y, 2)};
				const auto c {pow(Radius, 4) / pow(ptLast.y, 2) - pow(Radius, 2)};
				if (pow(b, 2) - 4 * a * c >= 0) {
					auto x {(-b + sqrt(pow(b, 2) - 4 * a * c)) / (2 * a)};
					auto y {(pow(Radius, 2) - ptLast.x * x) / ptLast.y};
					snapPoints.append(Center + OdGeVector3d(x, y, 0));
					if (bThickness) {
						snapPoints.append(Center + OdGeVector3d(x, y, 0) + vThickness);
					}
					x = (-b - sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);
					y = (pow(Radius, 2) - ptLast.x * x) / ptLast.y;
					snapPoints.append(Center + OdGeVector3d(x, y, 0));
					if (bThickness) {
						snapPoints.append(Center + OdGeVector3d(x, y, 0) + vThickness);
					}
				}
			} else {
				auto x {pow(Radius, 2) / ptLast.x};
				auto y {sqrt(pow(Radius, 2) - pow(x, 2))};
				snapPoints.append(Center + OdGeVector3d(x, y, 0));
				snapPoints.append(Center + OdGeVector3d(x, -y, 0));
				if (bThickness) {
					snapPoints.append(Center + OdGeVector3d(x, y, 0) + vThickness);
					snapPoints.append(Center + OdGeVector3d(x, -y, 0) + vThickness);
				}
			}
			break;
		case OdDb::kOsModeNear:                  // Nearest: cursor ~ hourglasses
			if (rdPick > 0) {
				auto Point {ptPick * Radius / rdPick};
				snapPoints.append(Center + Point.asVector());
				if (bThickness) {
					snapPoints.append(Center + Point.asVector() + vThickness);
				}
			}
			break;
		case OdDb::kOsModeEnd:                   // Endpoint:     cursor as square
		case OdDb::kOsModeNode:                  // Node:         cursor as cross in a square
		case OdDb::kOsModeMid:                   // Midpoint:     cursor as triangle
		case OdDb::kOsModeIntersec:              // Intersection: cursor as intersection in a square
		case OdDb::kOsModeIns:                   // Insertion:    -/-
		case OdDb::kOsModePar:                   // Parallel:
		case OdDb::kOsModeApint:                 // Apparent intersection:
			break;                                 //               isn't necessary to do
		default:
			break;
	}
	return eOk;
}
