#include <OdaCommon.h>
#include "DbCircleGripPoints.h"
#include "OdGripPointsModule.h"

// Returns 5 Points: center + 4 points on Circle
OdResult OdDbCircleGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbCirclePtr Circle {entity};
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 5);
	gripPoints[GripPointsSize + 0] = Circle->center();
	Circle->getPointAtParam(0, gripPoints[GripPointsSize + 1]);
	Circle->getPointAtParam(OdaPI, gripPoints[GripPointsSize + 2]);
	Circle->getPointAtParam(OdaPI2, gripPoints[GripPointsSize + 3]);
	Circle->getPointAtParam(OdaPI + OdaPI2, gripPoints[GripPointsSize + 4]);
	if (!OdZero(Circle->thickness())) {
		const auto Extrusion {Circle->normal() * Circle->thickness()};
		for (auto i = 0; i < 5; i++) {
			gripPoints.append(gripPoints[GripPointsSize + i] + Extrusion);
		}
	}
	return eOk;
}

// Move circle or change radius
OdResult OdDbCircleGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) { // indices[0] - defines for what point we pull:
		return eOk;
	} 
	OdDbCirclePtr Circle {entity};
	auto Center {true};
	auto Offset {offset};
	if (!ProjectOffset(Circle->database(), Circle->normal(), Offset)) { // Project offset on entity's plane in view direction. View direction is perpendicular to normal
		Circle->setCenter(Circle->center() + Offset);
		return eOk;
	}
	for (auto Index : indices) {
		if (Index % 5 == 0) { // point center - move circle
			if (Center) { // move center only one time
				Circle->setCenter(Circle->center() + Offset);
				Center = false;
			}
		} else { // change radius
			OdGePoint3dArray GripPoints;
			Circle->getGripPoints(GripPoints);
			auto Point {GripPoints[Index % 5] + Offset};
			Circle->setRadius(Circle->center().distanceTo(Point));
		}
	}
	return eOk;
}

// Cannot be stretched
OdResult OdDbCircleGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDbCirclePtr Circle {entity};
	stretchPoints.append(Circle->center());
	if (!OdZero(Circle->thickness())) { // next center
		stretchPoints.append(Circle->center() + Circle->normal() * Circle->thickness());
	}
	return eOk;
}

OdResult OdDbCircleGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& /*indices*/, const OdGeVector3d& offset) {
	OdDbCirclePtr Circle {entity};
	Circle->setCenter(Circle->center() + offset);
	return eOk;
}

/**
 * \brief Return snap Points into snapPoints, depending on type objectSnapMode
 * \param entity 
 * \param objectSnapMode 
 * \param pickPoint  Point, which moves
 * \param lastPoint  Point, from which draw line
 * \param
 * \param snapPoints 
 * \return 
 */
OdResult OdDbCircleGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdGePoint3dArray GripPoints;
	const auto Result {getGripPoints(entity, GripPoints)};
	if (Result != eOk || GripPoints.size() < 5) {
		return Result;
	}
	OdDbCirclePtr Circle {entity};
	const auto PickPointInPlane {GetPlanePoint(Circle, pickPoint)};
	const auto LastPointInPlane {GetPlanePoint(Circle, lastPoint)};
	auto Center {Circle->center()};
	const auto Radius {Circle->radius()};
	const auto PickPoint {PickPointInPlane - Center.asVector()};
	const auto PickRadius {PickPointInPlane.distanceTo(Center)};
	const auto LastPoint {LastPointInPlane - Center.asVector()};
	const auto LastRadius {LastPointInPlane.distanceTo(Center)};
	const auto ThicknessNotZero {!OdZero(Circle->thickness())};
	const auto Thickness {Circle->normal() * Circle->thickness()};
	switch (objectSnapMode) {
		case OdDb::kOsModeCen: // draw cross
			for (unsigned i = 0; i < GripPoints.size(); i += 5) {
				snapPoints.append(GripPoints[i]);
			}
			break;
		case OdDb::kOsModeQuad: // cursor as square turned on 45 degrees
			for (unsigned i = 1; i < GripPoints.size(); i++) {
				if (i == 5) {
					continue;
				}
				snapPoints.append(GripPoints[i]);
			}
			break;
		case OdDb::kOsModePerp: // Perpendicular: cursor as angle 90
			if (LastRadius > 0) {
				auto Point {LastPoint * Radius / LastRadius};
				snapPoints.append(Center + Point.asVector());
				snapPoints.append(Center - Point.asVector());
				if (ThicknessNotZero) {
					snapPoints.append(Center + Point.asVector() + Thickness);
					snapPoints.append(Center - Point.asVector() + Thickness);
				}
			}
			break;
		case OdDb::kOsModeTan: // cursor as circle with tangent
			if (LastRadius <= Radius) {
				break;
			}
			if (LastPoint.y != 0) {
				const auto a {1 + pow(LastPoint.x, 2) / pow(LastPoint.y, 2)};
				const auto b {-2 * pow(Radius, 2) * LastPoint.x / pow(LastPoint.y, 2)};
				const auto c {pow(Radius, 4) / pow(LastPoint.y, 2) - pow(Radius, 2)};
				if (pow(b, 2) - 4 * a * c >= 0) {
					auto x {(-b + sqrt(pow(b, 2) - 4 * a * c)) / (2 * a)};
					auto y {(pow(Radius, 2) - LastPoint.x * x) / LastPoint.y};
					snapPoints.append(Center + OdGeVector3d(x, y, 0));
					if (ThicknessNotZero) {
						snapPoints.append(Center + OdGeVector3d(x, y, 0) + Thickness);
					}
					x = (-b - sqrt(pow(b, 2) - 4 * a * c)) / (2 * a);
					y = (pow(Radius, 2) - LastPoint.x * x) / LastPoint.y;
					snapPoints.append(Center + OdGeVector3d(x, y, 0));
					if (ThicknessNotZero) {
						snapPoints.append(Center + OdGeVector3d(x, y, 0) + Thickness);
					}
				}
			} else {
				const auto x {pow(Radius, 2) / LastPoint.x};
				const auto y {sqrt(pow(Radius, 2) - pow(x, 2))};
				snapPoints.append(Center + OdGeVector3d(x, y, 0));
				snapPoints.append(Center + OdGeVector3d(x, -y, 0));
				if (ThicknessNotZero) {
					snapPoints.append(Center + OdGeVector3d(x, y, 0) + Thickness);
					snapPoints.append(Center + OdGeVector3d(x, -y, 0) + Thickness);
				}
			}
			break;
		case OdDb::kOsModeNear: // cursor ~ hourglass
			if (PickRadius > 0) {
				auto Point {PickPoint * Radius / PickRadius};
				snapPoints.append(Center + Point.asVector());
				if (ThicknessNotZero) {
					snapPoints.append(Center + Point.asVector() + Thickness);
				}
			}
			break;
		case OdDb::kOsModeEnd: // cursor as square
		case OdDb::kOsModeNode: // cursor as cross in a square
		case OdDb::kOsModeMid: // cursor as triangle
		case OdDb::kOsModeIntersec: // cursor as intersection in a square
		case OdDb::kOsModeIns:
		case OdDb::kOsModePar:
		case OdDb::kOsModeApint:
			break;
		default:
			break;
	}
	return eOk;
}
