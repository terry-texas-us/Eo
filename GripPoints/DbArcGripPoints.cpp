#include <OdaCommon.h>
#include "DbArcGripPoints.h"
#include <DbArc.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "OdGripPointsModule.h"

OdResult OdDbArcGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	OdDbArcPtr Arc {entity};
	const auto Thickness {Arc->thickness()};
	const auto NumberOfPoints {OdZero(Thickness) ? 4 : 8};
	gripPoints.resize(GripPointsSize + NumberOfPoints);
	auto StartPoint {gripPoints.asArrayPtr() + GripPointsSize};
	auto CurrentPoint {StartPoint};
	Arc->getStartPoint(*CurrentPoint++);
	Arc->getEndPoint(*CurrentPoint++);
	double StartParameter;
	double EndParameter;
	Arc->getStartParam(StartParameter);
	Arc->getEndParam(EndParameter);
	Arc->getPointAtParam((StartParameter + EndParameter) / 2.0, *CurrentPoint++);
	*CurrentPoint++ = Arc->center();
	if (NumberOfPoints == 8) {
		const auto Extrusion {Arc->normal() * Thickness};
		*CurrentPoint++ = *StartPoint++ + Extrusion;
		*CurrentPoint++ = *StartPoint++ + Extrusion;
		*CurrentPoint++ = *StartPoint++ + Extrusion;
		*CurrentPoint++ = *StartPoint++ + Extrusion;
	}
	return eOk;
}

OdResult OdDbArcGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto IndicesSize {indices.size()};
	if (IndicesSize == 0) {
		return eOk;
	}
	OdDbArcPtr Arc {entity};
	auto Offset {offset};
	// Project offset on entity's plane in view direction
	if (!ProjectOffset(Arc->database(), Arc->normal(), Offset)) { // View direction is perpendicular to normal. Move the arc
		Arc->setCenter(Arc->center() + Offset);
	} else {
		auto Flags {0U};
		OdGePoint3dArray Points;
		getGripPoints(entity, Points);
		for (unsigned i = 0; i < IndicesSize; ++i) {
			unsigned Index = indices[i];
			if (Index < Points.size()) {
				if (Index >= 4) {
					Index -= 4;
				}
				const auto Mask {1U << Index};
				if ((Flags & Mask) == 0U) {
					Points[Index] += Offset;
					Flags |= Mask;
				}
			}
		}
		try {
			if ((Flags & 8U) != 0U || (Flags & 7U) == 7U) { // // Center moved (8) or all 3 arc points moved
				Arc->setCenter(Points[3]);
			} else {
				const auto pP1 {Points.asArrayPtr()};
				const auto pP2 {pP1 + 2};
				const auto pP3 {pP1 + 1};
				OdGeCircArc3d CircularArc;
				const auto v1 {*pP2 - *pP1};
				const auto v2 {*pP3 - *pP2};
				const auto NewNormal {v1.crossProduct(v2)};
				if (NewNormal.isCodirectionalTo(Arc->normal())) { // OK
					CircularArc.set(*pP1, *pP2, *pP3);
				} else { // Arc changed direction. Normal should not be flipped.
					CircularArc.set(*pP3, *pP2, *pP1);
				}
				Arc->setFromOdGeCurve(CircularArc);
			}
		} catch (const OdError& Error) {
			return Error.code();
		}
	}
	return eOk;
}

OdResult OdDbArcGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	const auto StretchPointsSize {stretchPoints.size()};
	const auto Result {getGripPoints(entity, stretchPoints)};
	if (Result == eOk) {
		stretchPoints.resize(StretchPointsSize + 2);
	}
	// remove mid and center and thickness grips
	return Result;
}

OdResult OdDbArcGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbArcPtr Arc {entity};
	auto Offset {offset};
	// Project offset on entity's plane in view direction
	if (!ProjectOffset(Arc->database(), Arc->normal(), Offset)) {
		// View direction is perpendicular to normal. Do nothing
		return eOk;
	}
	if (indices.size() >= 2) {
		return entity->transformBy(OdGeMatrix3d::translation(Offset));
	}
	try {
		OdGePoint3d Point;
		switch (indices[0]) {
			case 0: {
				Arc->getStartPoint(Point);
				const auto v0 {Point - Arc->center()};
				Point += Offset;
				const auto v1 {Point - Arc->center()};
				const auto Angle {v0.angleTo(v1, Arc->normal())};
				Arc->setStartAngle(Arc->startAngle() + Angle);
				break;
			}
			case 1: {
				Arc->getEndPoint(Point);
				const auto v0 {Point - Arc->center()};
				Point += Offset;
				const auto v1 {Point - Arc->center()};
				const auto Angle {v0.angleTo(v1, Arc->normal())};
				Arc->setEndAngle(Arc->endAngle() + Angle);
				break;
			}
			default: ;
		}
	} catch (const OdError& Error) {
		return Error.code();
	}
	return eOk;
}

OdResult OdDbArcGripPointsPE::getOsnapPoints(const OdDbEntity* entity, const OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const {
	OdGePoint3dArray GripPoints;
	const auto Result {getGripPoints(entity, GripPoints)};
	if (Result != eOk) {
		return Result;
	}
	OdDbArcPtr Arc {entity};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			snapPoints.append(GripPoints[0]);
			snapPoints.append(GripPoints[1]);
			break;
		case OdDb::kOsModeMid:
			snapPoints.append(GripPoints[2]);
			break;
		case OdDb::kOsModeCen:
			snapPoints.append(GripPoints[3]);
			break;
		case OdDb::kOsModeQuad: {
			const OdDbDatabase* Database {entity->database()};
			const auto XAxis {Database->getUCSXDIR()};
			const auto YAxis {Database->getUCSYDIR()};
			const auto ZAxis {XAxis.crossProduct(YAxis)};
			if (!Arc->normal().isParallelTo(ZAxis)) {
				return eInvalidAxis;
			}
			snapPoints.append(Arc->center() + XAxis * Arc->radius());
			snapPoints.append(Arc->center() - XAxis * Arc->radius());
			snapPoints.append(Arc->center() + YAxis * Arc->radius());
			snapPoints.append(Arc->center() - YAxis * Arc->radius());
			break;
		}
		case OdDb::kOsModePerp: {
			const OdGePlane Plane(Arc->center(), Arc->normal());
			OdGePoint3d pp;
			if (!Plane.project(lastPoint, pp)) {
				return eNotApplicable;
			}
			auto v {pp - Arc->center()};
			if (v.isZeroLength()) {
				return eNotApplicable;
			}
			v.normalize();
			v *= Arc->radius();
			snapPoints.append(Arc->center() + v);
			snapPoints.append(Arc->center() - v);
			break;
		}
		case OdDb::kOsModeTan: {
			const OdGePlane Plane(Arc->center(), Arc->normal());
			OdGePoint3d pp;
			if (!Plane.project(lastPoint, pp)) {
				return eNotApplicable;
			}
			auto v {pp - Arc->center()};
			const auto Length {v.length()};
			const auto Radius {Arc->radius()};
			if (Length <= Radius) { // Length may be zero
				return eNotApplicable;
			}
			const auto c {Radius / Length};
			const auto Angle {OD_ACOS(c)};
			auto v1 {v.normalize()};
			v1 *= Radius;
			auto v2 {v1};
			v1.rotateBy(Angle, Arc->normal());
			v2.rotateBy(-Angle, Arc->normal());
			snapPoints.append(Arc->center() + v1);
			snapPoints.append(Arc->center() + v2);
			break;
		}
		case OdDb::kOsModeNear: {
			OdGePoint3d ClosestPoint;
			if (Arc->getClosestPointTo(pickPoint, worldToEyeTransform.inverse() * OdGeVector3d::kZAxis, ClosestPoint) == eOk) {
				snapPoints.append(ClosestPoint);
			}
		}
		break;
		default:
			break;
	}
	return eOk;
}
