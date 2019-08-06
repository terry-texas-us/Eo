#include <OdaCommon.h>
#include "DbArcGripPoints.h"
#include <DbArc.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "OdGripPointsModule.h"

OdResult OdDbArcGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto size {gripPoints.size()};
	OdDbArcPtr arc = entity;
	const auto dThickness {arc->thickness()};
	const auto nNumPoints {OdZero(dThickness) ? 4 : 8};
	gripPoints.resize(size + nNumPoints);
	auto pStart {gripPoints.asArrayPtr() + size};
	auto pCur {pStart};
	arc->getStartPoint(*pCur++);
	arc->getEndPoint(*pCur++);
	double start;
	double end;
	arc->getStartParam(start);
	arc->getEndParam(end);
	arc->getPointAtParam((start + end) / 2, *pCur++);
	*pCur++ = arc->center();
	if (nNumPoints == 8) {
		const auto Extrusion {arc->normal() * dThickness};
		*pCur++ = *pStart++ + Extrusion;
		*pCur++ = *pStart++ + Extrusion;
		*pCur++ = *pStart++ + Extrusion;
		*pCur++ = *pStart++ + Extrusion;
	}
	return eOk;
}

OdResult OdDbArcGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& vOffset) {
	const auto indicesSize {indices.size()};
	if (indicesSize == 0) {
		return eOk;
	}
	OdDbArcPtr pArc = entity;
	auto offset {vOffset};
	// Project offset on entity's plane in view direction
	if (!projectOffset(pArc->database(), pArc->normal(), offset)) { // View direction is perpendicular to normal. Move the arc
		pArc->setCenter(pArc->center() + offset);
	} else {
		auto flags {0};
		OdGePoint3dArray pts;
		getGripPoints(entity, pts);
		for (unsigned i = 0; i < indicesSize; ++i) {
			unsigned ind = indices[i];
			if (ind < pts.size()) {
				if (ind >= 4) {
					ind -= 4;
				}
				const auto nMask {1 << ind};
				if (!(flags & nMask)) {
					pts[ind] += offset;
					flags |= nMask;
				}
			}
		}
		try {
			if ((flags & 8) || ((flags & 7) == 7)) { // // Center moved (8) or all 3 arc points moved
				pArc->setCenter(pts[3]);
			} else {
				const auto pP1 {pts.asArrayPtr()};
				const auto pP2 {pP1 + 2};
				const auto pP3 {pP1 + 1};
				OdGeCircArc3d geArc;
				// Check if arc changed direction. Normal should not be flipped.
				const auto v1 {*pP2 - *pP1};
				const auto v2 {*pP3 - *pP2};
				const auto vNewNormal {v1.crossProduct(v2)};
				if (vNewNormal.isCodirectionalTo(pArc->normal())) {
					// OK
					geArc.set(*pP1, *pP2, *pP3);
				} else {
					geArc.set(*pP3, *pP2, *pP1);
				}
				pArc->setFromOdGeCurve(geArc);
			}
		} catch (const OdError& e) {
			return e.code();
		}
	}
	return eOk;
}

OdResult OdDbArcGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	const auto OldSize {stretchPoints.size()};
	const auto Result {getGripPoints(entity, stretchPoints)};
	if (Result == eOk) {
		stretchPoints.resize(OldSize + 2);
	}
	// remove mid and center and thickness grips
	return Result;
}

OdResult OdDbArcGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& vOffset) {
	const auto indicesSize {indices.size()};
	if (indicesSize == 0) {
		return eOk;
	}
	OdDbArcPtr pArc = entity;
	auto offset {vOffset};
	// Project offset on entity's plane in view direction
	if (!projectOffset(pArc->database(), pArc->normal(), offset)) {
		// View direction is perpendicular to normal
		// Do nothing
		return eOk;
	}
	if (indicesSize >= 2) {
		return entity->transformBy(OdGeMatrix3d::translation(offset));
	}
	try {
		OdGePoint3d pt;
		switch (indices[0]) {
			case 0: {
				pArc->getStartPoint(pt);
				const auto v0 {pt - pArc->center()};
				pt += offset;
				const auto v1 {pt - pArc->center()};
				const auto angle {v0.angleTo(v1, pArc->normal())};
				pArc->setStartAngle(pArc->startAngle() + angle);
			}
			break;
			case 1: {
				pArc->getEndPoint(pt);
				const auto v0 {pt - pArc->center()};
				pt += offset;
				const auto v1 {pt - pArc->center()};
				const auto angle {v0.angleTo(v1, pArc->normal())};
				pArc->setEndAngle(pArc->endAngle() + angle);
			}
			break;
		}
	} catch (const OdError& e) {
		return e.code();
	}
	return eOk;
}

OdResult OdDbArcGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& xWorldToEye, OdGePoint3dArray& snapPoints) const {
	OdGePoint3dArray gripPoints;
	const auto Result {getGripPoints(entity, gripPoints)};
	if (Result != eOk) {
		return Result;
	}
	OdDbArcPtr arc = entity;
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			snapPoints.append(gripPoints[0]);
			snapPoints.append(gripPoints[1]);
			break;
		case OdDb::kOsModeMid:
			snapPoints.append(gripPoints[2]);
			break;
		case OdDb::kOsModeCen:
			snapPoints.append(gripPoints[3]);
			break;
		case OdDb::kOsModeQuad: {
			const OdDbDatabase* pDb = entity->database();
			const auto xAxis {pDb->getUCSXDIR()};
			const auto yAxis {pDb->getUCSYDIR()};
			const auto zAxis {xAxis.crossProduct(yAxis)};
			if (!arc->normal().isParallelTo(zAxis)) {
				return eInvalidAxis;
			}
			snapPoints.append(arc->center() + xAxis * arc->radius());
			snapPoints.append(arc->center() - xAxis * arc->radius());
			snapPoints.append(arc->center() + yAxis * arc->radius());
			snapPoints.append(arc->center() - yAxis * arc->radius());
		}
		break;
		case OdDb::kOsModePerp: {
			const OdGePlane Plane(arc->center(), arc->normal());
			OdGePoint3d pp;
			if (!Plane.project(lastPoint, pp)) {
				return eNotApplicable;
			}
			auto v {pp - arc->center()};
			if (v.isZeroLength()) {
				return eNotApplicable;
			}
			v.normalize();
			v *= arc->radius();
			snapPoints.append(arc->center() + v);
			snapPoints.append(arc->center() - v);
		}
		break;
		case OdDb::kOsModeTan: {
			const OdGePlane Plane(arc->center(), arc->normal());
			OdGePoint3d pp;
			if (!Plane.project(lastPoint, pp)) {
				return eNotApplicable;
			}
			auto v {pp - arc->center()};
			const auto dLen {v.length()};
			const auto Radius {arc->radius()};
			if (dLen <= Radius) { // dLen may be zero
				return eNotApplicable;
			}
			const auto c {Radius / dLen};
			const auto angle {OD_ACOS(c)};
			auto v1 {v.normalize()};
			v1 *= Radius;
			auto v2 {v1};
			v1.rotateBy(angle, arc->normal());
			v2.rotateBy(-angle, arc->normal());
			snapPoints.append(arc->center() + v1);
			snapPoints.append(arc->center() + v2);
		}
		break;
		case OdDb::kOsModeNear: {
			OdGePoint3d p;
			if (arc->getClosestPointTo(pickPoint, xWorldToEye.inverse() * OdGeVector3d::kZAxis, p) == eOk) {
				snapPoints.append(p);
			}
		}
		break;
		default:
			break;
	}
	return eOk;
}
