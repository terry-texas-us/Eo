#include <OdaCommon.h>
#include "DbPolylineGripPoints.h"
#include <DbPolyline.h>
#include <Ge/GeMatrix3d.h>
#include <Ge/GeCircArc2d.h>
#include <Ge/GeCircArc3d.h>

OdResult OdDbPolylineGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto size {gripPoints.size()};
	OdDbPolylinePtr pPoly = entity;
	gripPoints.resize(size + (pPoly->numVerts() * 2 - 1));
	if (pPoly->numVerts() > 1) {
		pPoly->getPointAt(0, gripPoints[size]);
	}
	auto iIndAdd {size + 1};
	for (unsigned i = 1; i < pPoly->numVerts(); ++i) {
		OdGePoint3d p;
		pPoly->getPointAtParam(i - 0.5, p);
		gripPoints[iIndAdd++] = p;
		pPoly->getPointAt(i, gripPoints[iIndAdd++]);
	}
	if (pPoly->isClosed()) {
		OdGePoint3d p;
		pPoly->getPointAtParam(pPoly->numVerts() - 0.5, p);
		gripPoints.append(p);
	}
	return eOk;
}

OdResult OdDbPolylineGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto IndicesSize {indices.size()};
	if (IndicesSize == 0) {
		return eOk;
	}
	OdDbPolylinePtr Polyline {entity};
	const auto x {OdGeMatrix3d::worldToPlane(Polyline->normal())};
	const auto NumberOfVertices {Polyline->numVerts()};
	auto PolyLineIndex {0};
	unsigned iUStart = 0;
	unsigned i = 0;
	for (i = 0; i < IndicesSize; ++i) {
		PolyLineIndex = indices[i] / 2;
		if (indices[i] % 2 == 0) { // "Vertex. Check if near middle grip point presents. If yes skip this vertex
			auto PreviousIndex {indices[i] - 1};
			if (PreviousIndex < 0) {
				if (Polyline->isClosed()) {
					PreviousIndex = NumberOfVertices * 2 - 1;
				}
			}
			if (PreviousIndex >= 0 && indices.find(PreviousIndex, iUStart)) {
				continue;
			}
			auto NextIndex {indices[i] + 1};
			if (NextIndex > (static_cast<int>(NumberOfVertices) - 1) * 2) {
				NextIndex = Polyline->isClosed() ? 1 : -1;
			}
			if (NextIndex >= 0 && indices.find(NextIndex, iUStart)) {
				continue;
			}
			OdGePoint3d pt;
			Polyline->getPointAt(PolyLineIndex, pt);
			pt += offset;
			Polyline->setPointAt(PolyLineIndex, (x * pt).convert2d());
		} else { // Middle of segment point
			const auto PolylineIndex1 {PolyLineIndex != static_cast<int>(Polyline->numVerts()) - 1 ? PolyLineIndex + 1 : 0};
			if (!OdZero(Polyline->getBulgeAt(PolyLineIndex))) { // Arc segment
				OdGePoint2d p0;
				Polyline->getPointAt(PolyLineIndex, p0);
				OdGePoint2d p1;
				Polyline->getPointAt(PolylineIndex1, p1);
				OdGePoint3d pt;
				Polyline->getPointAtParam(PolyLineIndex + 0.5, pt);
				pt += offset;
				auto p {(x * pt).convert2d()};
				try {
					OdGeCircArc2d CircularArc(p0, p, p1);
					auto bulge {tan((CircularArc.endAng() - CircularArc.startAng()) / 4)};
					if (CircularArc.isClockWise()) {
						bulge = -bulge;
					}
					Polyline->setBulgeAt(PolyLineIndex, bulge);
				} catch (OdError& e) {
					return e.code();
				}
			} else { // Line segment
				OdGePoint3d pt;
				Polyline->getPointAt(PolyLineIndex, pt);
				pt += offset;
				Polyline->setPointAt(PolyLineIndex, (x * pt).convert2d());
				Polyline->getPointAt(PolylineIndex1, pt);
				pt += offset;
				Polyline->setPointAt(PolylineIndex1, (x * pt).convert2d());
			}
		}
	}
	return eOk;
}

OdResult OdDbPolylineGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	const auto size {stretchPoints.size()};
	OdDbPolylinePtr pPoly = entity;
	stretchPoints.resize(size + pPoly->numVerts());
	unsigned i = 0;
	for (; i < pPoly->numVerts(); ++i) {
		pPoly->getPointAt(i, stretchPoints[size + i]);
	}
	return eOk;
}

OdResult OdDbPolylineGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbPolylinePtr pPoly = entity;
	auto off {offset};
	off.transformBy(OdGeMatrix3d::worldToPlane(pPoly->normal()));
	const auto offset2d {offset.convert2d()};
	for (unsigned i = 0; i < indices.size(); ++i) {
		OdGePoint2d p;
		pPoly->getPointAt(indices[i], p);
		pPoly->setPointAt(indices[i], p + offset2d);
	}
	return eOk;
}

OdResult OdDbPolylineGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const {
	OdDbPolylinePtr pPoly = entity;
	if (selectionMarker) {
		const OdDbFullSubentPath subEntPath(OdDb::kEdgeSubentType, selectionMarker);
		auto pSubEnt {pPoly->subentPtr(subEntPath)};
		if (!pSubEnt.isNull()) {
			return pSubEnt->getOsnapPoints(objectSnapMode, 0, pickPoint, lastPoint, worldToEyeTransform, snapPoints);
		}
	}
	const auto size {snapPoints.size()};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			getStretchPoints(entity, snapPoints);
			break;
		case OdDb::kOsModeMid: {
			snapPoints.resize(size + pPoly->numVerts() - 1);
			for (unsigned i = 1; i < pPoly->numVerts(); ++i) {
				pPoly->getPointAtParam(i - 0.5, snapPoints[size + i - 1]);
			}
			break;
		}
		case OdDb::kOsModeCen:
			for (unsigned i = 0; i < pPoly->numVerts(); i++) {
				if (pPoly->segType(i) == OdDbPolyline::kArc) {
					OdGeCircArc3d arc;
					pPoly->getArcSegAt(i, arc);
					snapPoints.append(arc.center());
				}
			}
			break;
		case OdDb::kOsModeQuad: {
			for (unsigned i = 0; i < pPoly->numVerts(); i++) {
				if (pPoly->segType(i) == OdDbPolyline::kArc) {
					OdGeCircArc3d arc;
					pPoly->getArcSegAt(i, arc);
					const OdDbDatabase* pDb = entity->database();
					auto xAxis {pDb->getUCSXDIR()};
					auto yAxis {pDb->getUCSYDIR()};
					auto zAxis {xAxis.crossProduct(yAxis)};
					if (!arc.normal().isParallelTo(zAxis)) {
						return eInvalidAxis;
					}
					OdGeVector3d vStart, vEnd, vQuad;
					vStart = arc.startPoint() - arc.center();
					vEnd = arc.endPoint() - arc.center();
					int k[5] = {0, 1, 0, -1, 0};
					for (auto quad = 0; quad < 4; quad ++) {
						vQuad = xAxis * arc.radius() * k[quad + 1] + yAxis * arc.radius() * k[quad];
						if ((vQuad - vStart).crossProduct(vEnd - vQuad).isCodirectionalTo(arc.normal())) {
							snapPoints.append(arc.center() + xAxis * arc.radius() * k[quad + 1] + yAxis * arc.radius() * k[quad]);
						}
					}
				}
			}
		}
		break;
		case OdDb::kOsModeNear: {
			OdGePoint3d p;
			if (pPoly->getClosestPointTo(pickPoint, worldToEyeTransform.inverse() * OdGeVector3d::kZAxis, p) == eOk) {
				snapPoints.append(p);
			}
		}
		break;
		default:
			break;
	}
	return eOk;
}
