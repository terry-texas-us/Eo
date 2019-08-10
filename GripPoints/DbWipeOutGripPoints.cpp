#include <OdaCommon.h>
#include "DbWipeOutGripPoints.h"
#include <Ge/GeMatrix3d.h>
#include <Ge/GeScale3d.h>
#include <Ge/GeCircArc3d.h>
#include <DbWipeout.h>

OdResult OdDbWipeOutGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdGePoint3dArray Vertices;
	OdDbWipeoutPtr(entity)->getVertices(Vertices);
	// for the closed polyline boundary last coincident point is not returned as a grip
	if (Vertices.size() > 2 && Vertices.last().isEqualTo(Vertices.first())) {
		Vertices.removeLast();
	}
	gripPoints.append(Vertices);
	return eOk;
}

OdResult OdDbWipeOutGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbWipeoutPtr Wipeout {entity};
	OdGePoint3dArray Vertices;
	Wipeout->getVertices(Vertices);
	// for the closed polyline boundary last coincident point is not returned as a grip
	const auto HasClosedPolylineBoundary {Vertices.size() > 2 && Vertices.last().isEqualTo(Vertices.first())};
	for (auto Index : indices) {
		if (Index < static_cast<int>(Vertices.length())) {
			auto Vertex {Vertices.getAt(Index)};
			Vertex += offset;
			Vertices.setAt(Index, Vertex);
		}
	}
	if (HasClosedPolylineBoundary) {
		Vertices[Vertices.length() - 1] = Vertices.first();
	}
	Wipeout->setBoundary(Vertices);
	return eOk;
}

OdResult OdDbWipeOutGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbWipeOutGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbWipeOutGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdDbWipeoutPtr Wipeout {entity};
	OdGePoint3dArray Vertices;
	Wipeout->getVertices(Vertices);
	const auto SnapPointsSize {snapPoints.size()};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			getStretchPoints(entity, snapPoints);
			break;
		case OdDb::kOsModeMid: {
			snapPoints.resize(SnapPointsSize + Vertices.length() - 1);
			for (unsigned i = 1; i < Vertices.length(); ++i) {
				OdGeLine3d l(Vertices.getAt(i - 1), Vertices.getAt(i));
				snapPoints.append(l.evalPoint(l.paramOf(lastPoint)));
			}
			break;
		}
		case OdDb::kOsModeQuad: // not implemented yet
			break;
		case OdDb::kOsModeNear: {
			snapPoints.resize(SnapPointsSize + Vertices.length() - 1);
			for (unsigned i = 1; i < Vertices.length(); ++i) {
				auto Start {Vertices.getAt(i - 1)};
				auto End {Vertices.getAt(i)};
				if (!Start.isEqualTo(End)) {
					OdGeLine3d l(Start, End);
					const auto p {l.paramOf(pickPoint)};
					if (p > 1) {
						snapPoints.append(End);
					} else if (p < 0) {
						snapPoints.append(Start);
					} else {
						snapPoints.append(l.evalPoint(p));
					}
				} else {
					snapPoints.append(Start);
				}
			}
			break;
		}
		case OdDb::kOsModeCen:
			break;
		case OdDb::kOsModeNode:
			break;
		case OdDb::kOsModeIntersec:
			break;
		case OdDb::kOsModeIns:
			break;
		case OdDb::kOsModePerp:
			break;
		case OdDb::kOsModeTan:
			break;
		case OdDb::kOsModeApint:
			break;
		case OdDb::kOsModePar:
			break;
		case OdDb::kOsModeStart:
			break;
		default: ;
	}
	return eOk;
}
