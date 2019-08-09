#include <OdaCommon.h>
#include "Db2dPolylineGripPoints.h"
#include <Db2dPolyline.h>
#include <Ge/GeMatrix3d.h>
#include <Ge/GeCircArc2d.h>

OdResult OdDb2dPolylineGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDb2dPolylinePtr Polyline {entity};
	auto VertexIterator {Polyline->vertexIterator()};
	while (!VertexIterator->done()) {
		auto Vertex {OdDb2dVertex::cast(VertexIterator->entity())};
		ODA_ASSERT_ONCE(Vertex->vertexType() == OdDb::k2dVertex);
		if (Vertex->vertexType() == OdDb::k2dVertex) {
			gripPoints.append(Vertex->position());
		}
		VertexIterator->step();
	}
	return eOk;
}

OdResult OdDb2dPolylineGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto IndicesSize {indices.size()};
	if (IndicesSize == 0) {
		return eOk;
	}
	OdDb2dPolylinePtr Polyline {entity};
	const auto PreviousType {Polyline->polyType()};
	auto VertexIterator {Polyline->vertexIterator()};
	auto Counter {0};
	while (!VertexIterator->done()) {
		auto Vertex {OdDb2dVertex::cast(VertexIterator->entity())};
		if (Vertex->vertexType() == OdDb::k2dVertex) {
			for (unsigned i = 0; i < IndicesSize; i++) {
				if (indices[i] == Counter) {
					Vertex->upgradeOpen();
					Vertex->setPosition(Vertex->position() + offset);
					break;
				}
			}
			Counter++;
		}
		VertexIterator->step();
	}
	if (PreviousType != OdDb::k2dSimplePoly) { // Force re-computation of spline curve
		Polyline->convertToPolyType(PreviousType);
	}
	return eOk;
}

OdResult OdDb2dPolylineGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDb2dPolylinePtr Polyline {entity};
	for (auto i = Polyline->vertexIterator(); !i->done(); i->step()) {
		auto v {OdDb2dVertex::cast(i->entity())};
		if (!v.isNull() && v->vertexType() == OdDb::k2dVertex) {
			stretchPoints.append(v->position());
		}
	}
	return eOk;
}

OdResult OdDb2dPolylineGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDb2dPolylinePtr Polyline {entity};
	auto Index {0};
	for (auto i = Polyline->vertexIterator(); !i->done(); i->step(), ++Index) {
		auto v {OdDb2dVertex::cast(i->entity())};
		if (!v.isNull() && v->vertexType() == OdDb::k2dVertex) {
			if (indices.contains(Index)) {
				v->upgradeOpen();
				v->setPosition(v->position() + offset);
			}
			++Index;
		}
	}
	return eOk;
}

OdResult OdDb2dPolylineGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdDb2dPolylinePtr Polyline {entity};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd: case OdDb::kOsModeCen: case OdDb::kOsModeQuad: case OdDb::kOsModePerp: case OdDb::kOsModeTan: {
			auto VertexIterator {Polyline->vertexIterator()};
			while (!VertexIterator->done()) {
				auto Vertex {OdDb2dVertex::cast(VertexIterator->entity())};
				if (Vertex->vertexType() == OdDb::k2dVertex || Vertex->vertexType() == OdDb::k2dCurveFitVertex) {
					snapPoints.append(Vertex->position());
				}
				VertexIterator->step();
			}
		}
			break;
		default:
			break;
	}
	return eOk;
}
