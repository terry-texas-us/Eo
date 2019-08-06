#include <OdaCommon.h>
#include "Db2dPolylineGripPoints.h"
#include <Db2dPolyline.h>
#include <Ge/GeMatrix3d.h>
#include <Ge/GeCircArc2d.h>

OdResult OdDb2dPolylineGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDb2dPolylinePtr pPoly = entity;
	auto pIt {pPoly->vertexIterator()};
	while (!pIt->done()) {
		auto pVertex {OdDb2dVertex::cast(pIt->entity())};
		ODA_ASSERT_ONCE(pVertex->vertexType() == OdDb::k2dVertex);
		if (pVertex->vertexType() == OdDb::k2dVertex) {
			gripPoints.append(pVertex->position());
		}
		pIt->step();
	}
	return eOk;
}

OdResult OdDb2dPolylineGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto size {indices.size()};
	if (size == 0) {
		return eOk;
	}
	OdDb2dPolylinePtr pPoly = entity;
	const auto prevType {pPoly->polyType()};
	auto pIt {pPoly->vertexIterator()};
	auto counter {0};
	while (!pIt->done()) {
		auto pVertex {OdDb2dVertex::cast(pIt->entity())};
		if (pVertex->vertexType() == OdDb::k2dVertex) {
			for (unsigned i = 0; i < size; i++) {
				if (indices[i] == counter) {
					pVertex->upgradeOpen();
					pVertex->setPosition(pVertex->position() + offset);
					break;
				}
			}
			counter++;
		}
		pIt->step();
	}
	if (prevType != OdDb::k2dSimplePoly) { // Force re-computation of spline curve
		pPoly->convertToPolyType(prevType);
	}
	return eOk;
}

OdResult OdDb2dPolylineGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDb2dPolylinePtr pPoly = entity;
	for (auto i = pPoly->vertexIterator(); !i->done(); i->step()) {
		auto v {OdDb2dVertex::cast(i->entity())};
		if (!v.isNull() && v->vertexType() == OdDb::k2dVertex) {
			stretchPoints.append(v->position());
		}
	}
	return eOk;
}

OdResult OdDb2dPolylineGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDb2dPolylinePtr pPoly = entity;
	auto index {0};
	for (auto i = pPoly->vertexIterator(); !i->done(); i->step(), ++index) {
		auto v {OdDb2dVertex::cast(i->entity())};
		if (!v.isNull() && v->vertexType() == OdDb::k2dVertex) {
			if (indices.contains(index)) {
				v->upgradeOpen();
				v->setPosition(v->position() + offset);
			}
			++index;
		}
	}
	return eOk;
}

OdResult OdDb2dPolylineGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*xWorldToEye*/, OdGePoint3dArray& snapPoints) const {
	OdDb2dPolylinePtr Polyline = entity;
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd: case OdDb::kOsModeCen: case OdDb::kOsModeQuad: case OdDb::kOsModePerp: case OdDb::kOsModeTan: {
			auto pIt {Polyline->vertexIterator()};
			while (!pIt->done()) {
				auto Vertex {OdDb2dVertex::cast(pIt->entity())};
				if (Vertex->vertexType() == OdDb::k2dVertex || Vertex->vertexType() == OdDb::k2dCurveFitVertex) {
					snapPoints.append(Vertex->position());
				}
				pIt->step();
			}
		}
			break;
		default:
			break;
	}
	return eOk;
}
