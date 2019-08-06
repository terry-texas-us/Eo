#include <OdaCommon.h>
#include "Db3dPolylineGripPoints.h"
#include <Db3dPolyline.h>
#include <Db3dPolylineVertex.h>

OdResult OdDb3dPolylineGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDb3dPolylinePtr Polyline = entity;
	auto pIt {Polyline->vertexIterator()};
	while (!pIt->done()) {
		auto Vertex {OdDb3dPolylineVertex::cast(pIt->entity())};
		if (Vertex->vertexType() == OdDb::k3dSimpleVertex || Vertex->vertexType() == OdDb::k3dControlVertex) {
			gripPoints.append(Vertex->position());
		}
		pIt->step();
	}
	return eOk;
}

OdResult OdDb3dPolylineGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto size {indices.size()};
	if (size == 0) {
		return eOk;
	}
	OdDb3dPolylinePtr Polyline = entity;
	const auto prevType {Polyline->polyType()};
	auto pIt {Polyline->vertexIterator()};
	auto counter {0};
	while (!pIt->done()) {
		auto Vertex {OdDb3dPolylineVertex::cast(pIt->entity())};
		if (Vertex->vertexType() == OdDb::k3dSimpleVertex || Vertex->vertexType() == OdDb::k3dControlVertex) {
			for (unsigned i = 0; i < size; i++) {
				if (indices[i] == counter) {
					Vertex->upgradeOpen();
					Vertex->setPosition(Vertex->position() + offset);
					break;
				}
			}
			counter++;
		}
		pIt->step();
	}
	if (prevType != OdDb::k3dSimplePoly) { // Force re-computation of spline curve
		Polyline->convertToPolyType(prevType);
	}
	return eOk;
}

OdResult OdDb3dPolylineGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDb3dPolylineGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDb3dPolylineGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*xWorldToEye*/, OdGePoint3dArray& snapPoints) const {
	OdDb3dPolylinePtr pPoly = entity;
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd: {
			auto pIt {pPoly->vertexIterator()};
			while (!pIt->done()) {
				auto Vertex {OdDb3dPolylineVertex::cast(pIt->entity())};
				if (Vertex->vertexType() == OdDb::k3dSimpleVertex || Vertex->vertexType() == OdDb::k3dFitVertex) {
					snapPoints.append(Vertex->position());
				}
				pIt->step();
			}
		}
		break;
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
