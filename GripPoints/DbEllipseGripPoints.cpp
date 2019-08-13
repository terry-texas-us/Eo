#include <OdaCommon.h>
#include "DbEllipseGripPoints.h"
#include "OdGripPointsModule.h"

// Returns 5 Points: center + 4 points on Circle
OdResult OdDbEllipseGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbEllipsePtr Ellipse {entity};
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 5);
	gripPoints[GripPointsSize + 0] = Ellipse->center();                            // 0 - center
	Ellipse->getPointAtParam(Ellipse->startAngle(), gripPoints[GripPointsSize + 1]);             // 1 - right  (0)
	Ellipse->getPointAtParam(Ellipse->startAngle() + OdaPI, gripPoints[GripPointsSize + 2]);             // 2 - left   (pi)
	Ellipse->getPointAtParam(Ellipse->startAngle() + OdaPI2, gripPoints[GripPointsSize + 3]);             // 3 - top    (pi/2)
	Ellipse->getPointAtParam(Ellipse->startAngle() + OdaPI + OdaPI2, gripPoints[GripPointsSize + 4]);     // 4 - bottom (pi + pi/2)
	return eOk;
}

// Move circle or change radius
OdResult OdDbEllipseGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) { // indices[0] - defines for what point we pull:
		return eOk; // center or other
	}
	OdDbEllipsePtr Ellipse {entity};
	auto Center {true};
	auto Offset {offset};
	if (!ProjectOffset(Ellipse->database(), Ellipse->normal(), Offset)) { // Project offset on entity's plane in view direction. View direction is perpendicular to normal. Move the ellipse
		Ellipse->setCenter(Ellipse->center() + Offset);                   
		return eOk;                                                       
	}
	for (auto Index : indices) {
		if (Index % 5 == 0) { // point center - move circle
			if (Center) {// move center only one time
				Ellipse->setCenter(Ellipse->center() + Offset);
				Center = false;
			}
		} else {// change radius
			OdGePoint3dArray gripPoints;
			Ellipse->getGripPoints(gripPoints);
			const auto Dist1 {gripPoints[1].distanceTo(gripPoints[2])};
			const auto Dist2 {gripPoints[3].distanceTo(gripPoints[4])};
			auto point {gripPoints[Index % 5] + Offset};
			auto Normal {Ellipse->normal()};
			auto Center {Ellipse->center()};
			auto newDist {Ellipse->center().distanceTo(point)};
			newDist = newDist < 1.e-10 ? 1.e-10 : newDist;
			auto MajorAxis {Ellipse->majorAxis()};
			auto MinorAxis {Ellipse->minorAxis()};
			auto StartAngle {Ellipse->startAngle()};
			auto EndAngle {Ellipse->endAngle()};
			auto SwapMajorMinor {false};
			double RadiusRatio;
			if (Index < 3 && Dist1 > Dist2 || Index >= 3 && Dist1 < Dist2) {
				RadiusRatio = MinorAxis.length() / newDist;
				MajorAxis.setLength(newDist);
				if (RadiusRatio > 1) {
					RadiusRatio = 1.0 / RadiusRatio;
					MajorAxis = MinorAxis;
					SwapMajorMinor = true;
				}
			} else {
				RadiusRatio = newDist / MajorAxis.length();
				if (RadiusRatio > 1) {
					RadiusRatio = MajorAxis.length() / newDist;
					MajorAxis = MinorAxis;
					MajorAxis.setLength(newDist);
					SwapMajorMinor = true;
				}
			}
			try {
				if (SwapMajorMinor) {
					StartAngle = StartAngle - OdaPI2;
					EndAngle = EndAngle - OdaPI2;
					if (StartAngle < 0) {
						StartAngle = StartAngle + Oda2PI;
						EndAngle = EndAngle + Oda2PI;
					}
				}
				Ellipse->set(Center, Normal, MajorAxis, RadiusRatio, StartAngle, EndAngle);
			} catch (...) { }
		}
	}
	return eOk;
}

// Cannot be stretched
OdResult OdDbEllipseGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDbEllipsePtr Ellipse {entity};
	stretchPoints.append(Ellipse->center());                             // center
	//if (!OdZero(circle->thickness()))                                   // next center
	//{
	//  stretchPoints.append(circle->center() + circle->normal()*circle->thickness());
	//}
	return eOk;
}

OdResult OdDbEllipseGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& /*indices_*/, const OdGeVector3d& offset) {
	OdDbEllipsePtr Ellipse {entity};
	Ellipse->setCenter(Ellipse->center() + offset);
	return eOk;
}

/**
 * \brief Return snap Points into snapPoints, depending on type objectSnapMode
 * \param entity 
 * \param objectSnapMode 
 * \param 
 * \param pickPoint         Point, which moves
 * \param lastPoint         Point, from which draw line
 * \param 
 * \param snapPoints 
 * \return 
 */
OdResult OdDbEllipseGripPointsPE::getOsnapPoints(const OdDbEntity* entity, const OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdGePoint3dArray gripPoints;
	const auto Result {getGripPoints(entity, gripPoints)};
	if (Result != eOk || gripPoints.size() < 5) {
		return Result;
	}
	OdDbEllipsePtr Ellipse {entity};
	const auto PickPointInPlane {GetPlanePoint(Ellipse, pickPoint)}; // recalculated pickPoint and lastPoint in plane of circle
	const auto LastPointInPlane {GetPlanePoint(Ellipse, lastPoint)};
	auto Center {Ellipse->center()};
	const auto Radius {0.0}; // circle->radius();
	const auto ptPick {PickPointInPlane - Center.asVector()};
	const auto rdPick {PickPointInPlane.distanceTo(Center)};
	const auto ptLast {LastPointInPlane - Center.asVector()};
	const auto rdLast {LastPointInPlane.distanceTo(Center)};
	const auto ThicknessNotZero {false};
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
				if (ThicknessNotZero) {
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
					if (ThicknessNotZero) {
						snapPoints.append(Center + OdGeVector3d(x, y, 0) + vThickness);
					}
					x = (-b - sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);
					y = (pow(Radius, 2) - ptLast.x * x) / ptLast.y;
					snapPoints.append(Center + OdGeVector3d(x, y, 0));
					if (ThicknessNotZero) {
						snapPoints.append(Center + OdGeVector3d(x, y, 0) + vThickness);
					}
				}
			} else {
				auto x {pow(Radius, 2) / ptLast.x};
				auto y {sqrt(pow(Radius, 2) - pow(x, 2))};
				snapPoints.append(Center + OdGeVector3d(x, y, 0));
				snapPoints.append(Center + OdGeVector3d(x, -y, 0));
				if (ThicknessNotZero) {
					snapPoints.append(Center + OdGeVector3d(x, y, 0) + vThickness);
					snapPoints.append(Center + OdGeVector3d(x, -y, 0) + vThickness);
				}
			}
			break;
		case OdDb::kOsModeNear:                  // Nearest: cursor ~ hourglasses
			if (rdPick > 0) {
				auto Point {ptPick * Radius / rdPick};
				snapPoints.append(Center + Point.asVector());
				if (ThicknessNotZero) {
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
