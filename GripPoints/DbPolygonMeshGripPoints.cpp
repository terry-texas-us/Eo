#include <OdaCommon.h>
#include "DbPolygonMeshGripPoints.h"
#include <DbPolygonMesh.h>
#include <DbPolygonMeshVertex.h>
#include <DbLine.h>

OdResult OdDbPolygonMeshGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbPolygonMesh* Mesh = OdDbPolygonMesh::cast(entity);
	auto VertexIterator {Mesh->vertexIterator()};
	while (!VertexIterator->done()) {
		OdDbPolygonMeshVertexPtr Vertex = VertexIterator->entity();
		gripPoints.append(Vertex->position());
		VertexIterator->step();
	}
	return eOk;
}

OdResult OdDbPolygonMeshGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbPolygonMesh* Mesh = OdDbPolygonMesh::cast(entity);
	for (auto Index : indices) {
		OdDbObjectIteratorPtr VertexIterator {Mesh->vertexIterator()};
		auto CurrentIndex {0};
		while (!VertexIterator->done()) {
			if (CurrentIndex == Index) {
				OdDbPolygonMeshVertexPtr Vertex = VertexIterator->entity();
				Mesh->openVertex(Vertex->id(), OdDb::kForWrite);
				Vertex->setPosition(Vertex->position() + offset);
				Mesh->subClose();
				break;
			}
			CurrentIndex++;
			VertexIterator->step();
		}
	}
	return eOk;
}

OdResult OdDbPolygonMeshGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbPolygonMeshGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbPolygonMeshGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const {
	OdDbPolygonMesh* Mesh {OdDbPolygonMesh::cast(entity)};
	auto VertexIterator {Mesh->vertexIterator()};
	OdGePoint3dArray pPosArr;
	while (!VertexIterator->done()) {
		OdDbPolygonMeshVertexPtr Vertex = VertexIterator->entity();
		pPosArr.append(Vertex->position());
		VertexIterator->step();
	}
	const OdInt32 NumberOfColumns {Mesh->nSize()};
	const OdInt32 NumberOfRows {Mesh->mSize()};
	const auto ColumnsClosed {Mesh->isNClosed()};
	const auto RowsClosed {Mesh->isMClosed()};
	for (auto i = 0; i < NumberOfRows; i++) {
		for (auto j = 0; j < NumberOfColumns; j++) {
			if (j < NumberOfColumns - 1) {
				auto Line {OdDbLine::createObject()};
				Line->setStartPoint(pPosArr[i * NumberOfColumns + j]);
				Line->setEndPoint(pPosArr[i * NumberOfColumns + j + 1]);
				Line->getOsnapPoints(objectSnapMode, 0, pickPoint, lastPoint, worldToEyeTransform, snapPoints);
			} else if (j == NumberOfColumns - 1 && ColumnsClosed) {
				auto Line {OdDbLine::createObject()};
				Line->setStartPoint(pPosArr[i * NumberOfColumns + j]);
				Line->setEndPoint(pPosArr[i * NumberOfColumns]);
				Line->getOsnapPoints(objectSnapMode, 0, pickPoint, lastPoint, worldToEyeTransform, snapPoints);
			}
			if (i < NumberOfRows - 1) {
				auto Line {OdDbLine::createObject()};
				Line->setStartPoint(pPosArr[i * NumberOfColumns + j]);
				Line->setEndPoint(pPosArr[(i + 1) * NumberOfColumns + j]);
				Line->getOsnapPoints(objectSnapMode, 0, pickPoint, lastPoint, worldToEyeTransform, snapPoints);
			} else if (i == NumberOfRows - 1 && RowsClosed) {
				auto Line {OdDbLine::createObject()};
				Line->setStartPoint(pPosArr[i * NumberOfColumns + j]);
				Line->setEndPoint(pPosArr[j]);
				Line->getOsnapPoints(objectSnapMode, 0, pickPoint, lastPoint, worldToEyeTransform, snapPoints);
			}
		}
	}
	return eOk;
}

OdResult OdDbPolygonMeshGripPointsPE::getGripPointsAtSubentPath(const OdDbEntity* /*entity*/, const OdDbFullSubentPath& /*path*/, OdDbGripDataPtrArray& /*grips*/, const double /*currentViewUnitSize*/, const int /*gripSize*/, const OdGeVector3d& /*currentViewDirection*/, const OdUInt32 /*bitFlags*/) const {
	return eNotApplicable;
}

OdResult OdDbPolygonMeshGripPointsPE::moveGripPointsAtSubentPaths(OdDbEntity* /*entity*/, const OdDbFullSubentPathArray& /*paths*/, const OdDbVoidPtrArray& /*gripAppData*/, const OdGeVector3d& /*offset*/, const OdUInt32 /*bitFlags*/) {
	return eNotApplicable;
}
