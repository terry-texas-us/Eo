#include <OdaCommon.h>
#include "DbPolylineGripPoints.h"
#include <DbPolyline.h>
#include <Ge/GeMatrix3d.h>
#include <Ge/GeCircArc2d.h>
#include <Ge/GeCircArc3d.h>

OdResult OdDbPolylineGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	OdDbPolylinePtr Polyline {entity};
	gripPoints.resize(GripPointsSize + (Polyline->numVerts() * 2 - 1));
	if (Polyline->numVerts() > 1) {
		Polyline->getPointAt(0, gripPoints[GripPointsSize]);
	}
	auto iIndAdd {GripPointsSize + 1};
	for (unsigned i = 1; i < Polyline->numVerts(); ++i) {
		OdGePoint3d p;
		Polyline->getPointAtParam(i - 0.5, p);
		gripPoints[iIndAdd++] = p;
		Polyline->getPointAt(i, gripPoints[iIndAdd++]);
	}
	if (Polyline->isClosed()) {
		OdGePoint3d p;
		Polyline->getPointAtParam(Polyline->numVerts() - 0.5, p);
		gripPoints.append(p);
	}
	return eOk;
}

OdResult OdDbPolylineGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbPolylinePtr Polyline {entity};
	const auto x {OdGeMatrix3d::worldToPlane(Polyline->normal())};
	const auto NumberOfVertices {Polyline->numVerts()};
	unsigned iUStart = 0;
	for (unsigned i = 0; i < indices.size(); ++i) {
		const auto PolyLineIndex {indices[i] / 2};
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
	const auto StretchPointsSize {stretchPoints.size()};
	OdDbPolylinePtr Polyline {entity};
	stretchPoints.resize(StretchPointsSize + Polyline->numVerts());
	unsigned i = 0;
	for (; i < Polyline->numVerts(); ++i) {
		Polyline->getPointAt(i, stretchPoints[StretchPointsSize + i]);
	}
	return eOk;
}

OdResult OdDbPolylineGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbPolylinePtr Polyline {entity};
	auto Offset {offset};
	Offset.transformBy(OdGeMatrix3d::worldToPlane(Polyline->normal()));
	const auto offset2d {offset.convert2d()};
	for (auto Index : indices) {
		OdGePoint2d p;
		Polyline->getPointAt(Index, p);
		Polyline->setPointAt(Index, p + offset2d);
	}
	return eOk;
}

OdResult OdDbPolylineGripPointsPE::getOsnapPoints(const OdDbEntity* entity, const OdDb::OsnapMode objectSnapMode, const OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const {
	OdDbPolylinePtr Polyline {entity};
	if (selectionMarker) {
		const OdDbFullSubentPath subEntPath(OdDb::kEdgeSubentType, selectionMarker);
		auto pSubEnt {Polyline->subentPtr(subEntPath)};
		if (!pSubEnt.isNull()) {
			return pSubEnt->getOsnapPoints(objectSnapMode, 0, pickPoint, lastPoint, worldToEyeTransform, snapPoints);
		}
	}
	const auto SnapPointsSize {snapPoints.size()};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			getStretchPoints(entity, snapPoints);
			break;
		case OdDb::kOsModeMid: {
			snapPoints.resize(SnapPointsSize + Polyline->numVerts() - 1);
			for (unsigned i = 1; i < Polyline->numVerts(); ++i) {
				Polyline->getPointAtParam(i - 0.5, snapPoints[SnapPointsSize + i - 1]);
			}
			break;
		}
		case OdDb::kOsModeCen:
			for (unsigned i = 0; i < Polyline->numVerts(); i++) {
				if (Polyline->segType(i) == OdDbPolyline::kArc) {
					OdGeCircArc3d CircularArc;
					Polyline->getArcSegAt(i, CircularArc);
					snapPoints.append(CircularArc.center());
				}
			}
			break;
		case OdDb::kOsModeQuad: {
			for (unsigned i = 0; i < Polyline->numVerts(); i++) {
				if (Polyline->segType(i) == OdDbPolyline::kArc) {
					OdGeCircArc3d CircularArc;
					Polyline->getArcSegAt(i, CircularArc);
					const OdDbDatabase* Database {entity->database()};
					auto XAxis {Database->getUCSXDIR()};
					auto YAxis {Database->getUCSYDIR()};
					auto ZAxis {XAxis.crossProduct(YAxis)};
					if (!CircularArc.normal().isParallelTo(ZAxis)) {
						return eInvalidAxis;
					}
					OdGeVector3d Start;
					OdGeVector3d End;
					OdGeVector3d vQuad;
					Start = CircularArc.startPoint() - CircularArc.center();
					End = CircularArc.endPoint() - CircularArc.center();
					int k[5] = {0, 1, 0, -1, 0};
					for (auto quad = 0; quad < 4; quad ++) {
						vQuad = XAxis * CircularArc.radius() * k[quad + 1] + YAxis * CircularArc.radius() * k[quad];
						if ((vQuad - Start).crossProduct(End - vQuad).isCodirectionalTo(CircularArc.normal())) {
							snapPoints.append(CircularArc.center() + XAxis * CircularArc.radius() * k[quad + 1] + YAxis * CircularArc.radius() * k[quad]);
						}
					}
				}
			}
		}
		break;
		case OdDb::kOsModeNear: {
			OdGePoint3d Point;
			if (Polyline->getClosestPointTo(pickPoint, worldToEyeTransform.inverse() * OdGeVector3d::kZAxis, Point) == eOk) {
				snapPoints.append(Point);
			}
		}
		break;
		default:
			break;
	}
	return eOk;
}
