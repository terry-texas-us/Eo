#include <OdaCommon.h>
#include "DbRasterImageGripPoints.h"
#include <DbRasterImage.h>
#include <DbWipeout.h>
#include <Ge/GeLine3d.h>

OdResult OdDbRasterImageGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbRasterImagePtr RasterImage {entity};
	OdGeExtents3d Extents;
	const auto Result {RasterImage->getGeomExtents(Extents)};
	if (eOk == Result) {
		const auto GripPointsSize {gripPoints.size()};
		if (!RasterImage->isClipped() || !RasterImage->isSetDisplayOpt(OdDbRasterImage::kClip)) {
			gripPoints.resize(GripPointsSize + 5);
			gripPoints[GripPointsSize] = Extents.minPoint() + (Extents.maxPoint() - Extents.minPoint()) / 2.0;
			OdGePoint3d Origin;
			OdGeVector3d u;
			OdGeVector3d v;
			RasterImage->getOrientation(Origin, u, v);
			gripPoints[GripPointsSize + 1] = Origin;
			gripPoints[GripPointsSize + 2] = Origin + v;
			gripPoints[GripPointsSize + 3] = Origin + u + v;
			gripPoints[GripPointsSize + 4] = Origin + u;
		} else {
			OdGePoint3dArray ClipPoints;
			RasterImage->getVertices(ClipPoints);
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
	if (indices.empty()) {
		return eOk;
	}
	OdDbRasterImagePtr RasterImage {entity};
	if (!RasterImage->isClipped() || !RasterImage->isSetDisplayOpt(OdDbRasterImage::kClip)) {
		if (indices[0] == 0) {
			return entity->transformBy(OdGeMatrix3d::translation(offset));
		}
		OdGePoint3d Origin;
		OdGeVector3d U;
		OdGeVector3d V;
		RasterImage->getOrientation(Origin, U, V);
		auto xNorm = U;
		auto yNorm = V;
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
		const auto pOrg {Origin + cfs.iUOrg * vecU + cfs.iVOrg * vecV};
		if (!(U - vecU).isZeroLength() && !(V - vecV).isZeroLength()) {
			RasterImage->setOrientation(pOrg, U - vecU, V - vecV);
		}
	} else {
		OdGePoint3dArray ClipPoints;
		RasterImage->getVertices(ClipPoints);
		for (auto Index : indices) {
			if (Index >= 0 && static_cast<unsigned>(Index) < ClipPoints.size()) {
				ClipPoints[Index] += offset;
			}
		}
		ClipPoints.last() = ClipPoints.first();
		OdGePoint2dArray outBry;
		outBry.resize(ClipPoints.size());
		const auto w2p {RasterImage->getPixelToModelTransform().invert()};
		const auto ImageSize {RasterImage->imageSize()};
		const auto IsWipeout {entity->isKindOf(OdDbWipeout::desc())};
		for (unsigned setPt = 0; setPt < ClipPoints.size(); setPt++) {
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
		RasterImage->setClipBoundary(outBry);
	}
	return eOk;
}

OdResult OdDbRasterImageGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbRasterImageGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbRasterImageGripPointsPE::getOsnapPoints(const OdDbEntity* entity, const OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
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
