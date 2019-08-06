#include <OdaCommon.h>
#include "DbPolygonMeshGripPoints.h"
#include <DbPolygonMesh.h>
#include <DbPolygonMeshVertex.h>
#include <DbLine.h>

OdResult OdDbPolygonMeshGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbPolygonMesh* pMesh = OdDbPolygonMesh::cast(entity);
	auto pIter {pMesh->vertexIterator()};
	while (!pIter->done()) {
		OdDbPolygonMeshVertexPtr pVertex = pIter->entity();
		gripPoints.append(pVertex->position());
		pIter->step();
	}
	return eOk;
}

OdResult OdDbPolygonMeshGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbPolygonMesh* pMesh = OdDbPolygonMesh::cast(entity);
	OdDbObjectIteratorPtr pIter;
	for (unsigned iPt = 0; iPt < indices.size(); iPt++) {
		const auto iIndex {indices[iPt]};
		pIter = pMesh->vertexIterator();
		auto iCurIndex {0};
		while (!pIter->done()) {
			if (iCurIndex == iIndex) {
				OdDbPolygonMeshVertexPtr pVertex = pIter->entity();
				pMesh->openVertex(pVertex->id(), OdDb::kForWrite);
				pVertex->setPosition(pVertex->position() + offset);
				pMesh->subClose();
				break;
			}
			iCurIndex++;
			pIter->step();
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

OdResult OdDbPolygonMeshGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& xWorldToEye, OdGePoint3dArray& snapPoints) const {
	OdDbPolygonMesh* pMesh = OdDbPolygonMesh::cast(entity);
	auto pIter {pMesh->vertexIterator()};
	OdGePoint3dArray pPosArr;
	while (!pIter->done()) {
		OdDbPolygonMeshVertexPtr pVertex = pIter->entity();
		pPosArr.append(pVertex->position());
		pIter->step();
	}
	const OdInt32 nCols {pMesh->nSize()};
	const OdInt32 nRows {pMesh->mSize()};
	const auto bColsClosed {pMesh->isNClosed()};
	const auto bRowsClosed {pMesh->isMClosed()};
	for (auto i = 0; i < nRows; i++) {
		for (auto j = 0; j < nCols; j++) {
			if (j < nCols - 1) {
				auto pLine {OdDbLine::createObject()};
				pLine->setStartPoint(pPosArr[i * nCols + j]);
				pLine->setEndPoint(pPosArr[i * nCols + j + 1]);
				pLine->getOsnapPoints(objectSnapMode, 0, pickPoint, lastPoint, xWorldToEye, snapPoints);
			} else if (j == nCols - 1 && bColsClosed) {
				auto pLine {OdDbLine::createObject()};
				pLine->setStartPoint(pPosArr[i * nCols + j]);
				pLine->setEndPoint(pPosArr[i * nCols]);
				pLine->getOsnapPoints(objectSnapMode, 0, pickPoint, lastPoint, xWorldToEye, snapPoints);
			}
			if (i < nRows - 1) {
				auto pLine {OdDbLine::createObject()};
				pLine->setStartPoint(pPosArr[i * nCols + j]);
				pLine->setEndPoint(pPosArr[(i + 1) * nCols + j]);
				pLine->getOsnapPoints(objectSnapMode, 0, pickPoint, lastPoint, xWorldToEye, snapPoints);
			} else if (i == nRows - 1 && bRowsClosed) {
				auto pLine {OdDbLine::createObject()};
				pLine->setStartPoint(pPosArr[i * nCols + j]);
				pLine->setEndPoint(pPosArr[j]);
				pLine->getOsnapPoints(objectSnapMode, 0, pickPoint, lastPoint, xWorldToEye, snapPoints);
			}
		}
	}
	return eOk;
}

OdResult OdDbPolygonMeshGripPointsPE::getGripPointsAtSubentPath(const OdDbEntity* /*entity*/, const OdDbFullSubentPath& /*path*/, OdDbGripDataPtrArray& /*grips*/, const double /*curViewUnitSize*/, const int /*gripSize*/, const OdGeVector3d& /*curViewDir*/, const OdUInt32 /*bitFlags*/) const {
	return eNotApplicable;
}

OdResult OdDbPolygonMeshGripPointsPE::moveGripPointsAtSubentPaths(OdDbEntity* /*entity*/, const OdDbFullSubentPathArray& /*paths*/, const OdDbVoidPtrArray& /*gripAppData*/, const OdGeVector3d& /*offset*/, const OdUInt32 /*bitFlags*/) {
	return eNotApplicable;
}
