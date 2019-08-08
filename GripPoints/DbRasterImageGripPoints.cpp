#include <OdaCommon.h>
#include "DbRasterImageGripPoints.h"
#include <DbRasterImage.h>
#include <DbWipeout.h>
#include <Ge/GeLine3d.h>

OdResult OdDbRasterImageGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbRasterImagePtr pImg = entity;
	OdGeExtents3d Extents;
	const auto Result {pImg->getGeomExtents(Extents)};
	if (eOk == Result) {
		const auto Size {gripPoints.size()};
		if (!pImg->isClipped() || !pImg->isSetDisplayOpt(OdDbRasterImage::kClip)) {
			gripPoints.resize(Size + 5);
			gripPoints[Size] = Extents.minPoint() + (Extents.maxPoint() - Extents.minPoint()) / 2.;
			OdGePoint3d Origin;
			OdGeVector3d u;
			OdGeVector3d v;
			pImg->getOrientation(Origin, u, v);
			gripPoints[Size + 1] = Origin;
			gripPoints[Size + 2] = Origin + v;
			gripPoints[Size + 3] = Origin + u + v;
			gripPoints[Size + 4] = Origin + u;
		} else {
			OdGePoint3dArray ClipPoints;
			pImg->getVertices(ClipPoints);
			if (ClipPoints.last() == ClipPoints.first()) {
				ClipPoints.removeLast();
			}
			gripPoints.append(ClipPoints);
		}
	}
	return Result;
}

struct Coefficients {
	int iUOrg;
	int iVOrg;
	int iUCf;
	int iVCf;
};

OdResult OdDbRasterImageGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto IndicesSize {indices.size()};
	if (IndicesSize == 0) {
		return eOk;
	}
	OdDbRasterImagePtr pImg = entity;
	if (!pImg->isClipped() || !pImg->isSetDisplayOpt(OdDbRasterImage::kClip)) {
		if (indices[0] == 0) {
			return entity->transformBy(OdGeMatrix3d::translation(offset));
		}
		OdGePoint3d origin;
		OdGeVector3d u;
		OdGeVector3d v;
		pImg->getOrientation(origin, u, v);
		auto xNorm = u;
		auto yNorm = v;
		const auto xNLen {xNorm.normalizeGetLength()};
		const auto yNLen {yNorm.normalizeGetLength()}; //Default bottom - left corner
		Coefficients cfs = {1, 1, 1, 1};
		auto dX {xNorm.dotProduct(offset)};
		auto dY {yNorm.dotProduct(offset)};
		const auto CoefficientXy {xNLen / yNLen};
		switch (indices[0]) {
			case 2: //top - left corner
				cfs.iVOrg = 0;
				cfs.iVCf = -1;
				break;
			case 3: //top - right corner
				cfs.iVOrg = 0;
				cfs.iVCf = -1;
				cfs.iUOrg = 0;
				cfs.iUCf = -1;
				break;
			case 4: //bottom - right corner
				cfs.iUOrg = 0;
				cfs.iUCf = -1;
				break;
			default: ;
		}
		dY = cfs.iVCf * dY * CoefficientXy;
		dX = cfs.iUCf * dX;
		if (dX > xNLen) {
			dX = 2 * xNLen - dX;
		}
		if (dY > yNLen) { // For Ortho mode, either dX or dY is always 0.
			dY = 2 * yNLen - dY;
		}
		const auto dMov {fabs(dX) > fabs(dY) ? dX : dY};
		const auto vecU {xNorm * dMov};
		const auto vecV {yNorm * dMov * yNLen / xNLen};
		const auto pOrg {origin + cfs.iUOrg * vecU + cfs.iVOrg * vecV};
		if (!(u - vecU).isZeroLength() && !(v - vecV).isZeroLength()) {
			pImg->setOrientation(pOrg, u - vecU, v - vecV);
		}
	} else {
		OdGePoint3dArray ClipPoints;
		pImg->getVertices(ClipPoints);
		for (unsigned i = 0; i < indices.size(); ++i) {
			if (indices[i] >= 0 && static_cast<unsigned>(indices[i]) < ClipPoints.size()) {
				ClipPoints[indices[i]] += offset;
			}
		}
		ClipPoints.last() = ClipPoints.first();
		OdGePoint2dArray outBry;
		outBry.resize(ClipPoints.size());
		const auto w2p {pImg->getPixelToModelTransform().invert()};
		const auto ImageSize {pImg->imageSize()};
		const auto IsWipeout {entity->isKindOf(OdDbWipeout::desc())};
		for (OdUInt32 setPt = 0; setPt < ClipPoints.size(); setPt++) {
			outBry[setPt] = ClipPoints[setPt].transformBy(w2p).convert2d();
			if (!IsWipeout) {
				if (outBry[setPt].x < 0.0) {
					outBry[setPt].x = 0.0;
				} else if (outBry[setPt].x > ImageSize.x) {
					outBry[setPt].x = ImageSize.x;
				}
				if (outBry[setPt].y < 0.0) {
					outBry[setPt].y = 0.0;
				} else if (outBry[setPt].y > ImageSize.y) {
					outBry[setPt].y = ImageSize.y;
				}
			}
		}
		pImg->setClipBoundary(outBry);
	}
	return eOk;
}

OdResult OdDbRasterImageGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbRasterImageGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbRasterImageGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd: case OdDb::kOsModeCen: {
			OdGePoint3dArray GripPoints;
			getGripPoints(entity, GripPoints);
			if (!GripPoints.empty()) {
				if (objectSnapMode == OdDb::kOsModeEnd) {
					GripPoints.erase(GripPoints.begin());
				} else {
					GripPoints.erase(GripPoints.begin() + 1, GripPoints.end());
				}
				snapPoints.append(GripPoints);
			}
		}
			break;
		default:
			break;
	}
	return eOk;
}
