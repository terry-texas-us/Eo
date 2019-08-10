#include <OdaCommon.h>
#include "Db3dPolylineGripPoints.h"
#include <Db3dPolyline.h>
#include <Db3dPolylineVertex.h>

OdResult OdDb3dPolylineGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDb3dPolylinePtr Polyline {entity};
	auto VertexIterator {Polyline->vertexIterator()};
	while (!VertexIterator->done()) {
		auto Vertex {OdDb3dPolylineVertex::cast(VertexIterator->entity())};
		if (Vertex->vertexType() == OdDb::k3dSimpleVertex || Vertex->vertexType() == OdDb::k3dControlVertex) {
			gripPoints.append(Vertex->position());
		}
		VertexIterator->step();
	}
	return eOk;
}

OdResult OdDb3dPolylineGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	OdDb3dPolylinePtr Polyline {entity};
	const auto PreviousType {Polyline->polyType()};
	auto VertexIterator {Polyline->vertexIterator()};
	auto Counter {0};
	while (!VertexIterator->done()) {
		auto Vertex {OdDb3dPolylineVertex::cast(VertexIterator->entity())};
		if (Vertex->vertexType() == OdDb::k3dSimpleVertex || Vertex->vertexType() == OdDb::k3dControlVertex) {
			for (auto Index : indices) {
				if (Index == Counter) {
					Vertex->upgradeOpen();
					Vertex->setPosition(Vertex->position() + offset);
					break;
				}
			}
			Counter++;
		}
		VertexIterator->step();
	}
	if (PreviousType != OdDb::k3dSimplePoly) { // Force re-computation of spline curve
		Polyline->convertToPolyType(PreviousType);
	}
	return eOk;
}

OdResult OdDb3dPolylineGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDb3dPolylineGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDb3dPolylineGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdDb3dPolylinePtr Polyline {entity};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd: {
			auto VertexIterator {Polyline->vertexIterator()};
			while (!VertexIterator->done()) {
				auto Vertex {OdDb3dPolylineVertex::cast(VertexIterator->entity())};
				if (Vertex->vertexType() == OdDb::k3dSimpleVertex || Vertex->vertexType() == OdDb::k3dFitVertex) {
					snapPoints.append(Vertex->position());
				}
				VertexIterator->step();
			}
			break;
		}
		case OdDb::kOsModeMid:
			break;
		case OdDb::kOsModeCen:
			break;
		case OdDb::kOsModeNode:
			break;
		case OdDb::kOsModeQuad:
			break;
		case OdDb::kOsModeIntersec:
			break;
		case OdDb::kOsModeIns:
			break;
		case OdDb::kOsModePerp:
			break;
		case OdDb::kOsModeTan:
			break;
		case OdDb::kOsModeNear:
			break;
		case OdDb::kOsModeApint:
			break;
		case OdDb::kOsModePar:
			break;
		case OdDb::kOsModeStart:
			break;
		default:
			break;
	}
	return eOk;
}
