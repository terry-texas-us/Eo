#include <OdaCommon.h>
#include "DbRasterImageGripPoints.h"
#include <DbRasterImage.h>
#include <DbWipeout.h>
#include <Ge/GeLine3d.h>

OdResult OdDbRasterImageGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbRasterImagePtr pImg = entity;
	OdGeExtents3d exts;
	const auto Result {pImg->getGeomExtents(exts)};
	if (eOk == Result) {
		const auto size {gripPoints.size()};
		if (!pImg->isClipped() || !pImg->isSetDisplayOpt(OdDbRasterImage::kClip)) {
			gripPoints.resize(size + 5);
			gripPoints[size] = exts.minPoint() + (exts.maxPoint() - exts.minPoint()) / 2.;
			OdGePoint3d origin;
			OdGeVector3d u, v;
			pImg->getOrientation(origin, u, v);
			gripPoints[size + 1] = origin;
			gripPoints[size + 2] = origin + v;
			gripPoints[size + 3] = origin + u + v;
			gripPoints[size + 4] = origin + u;
		} else {
			OdGePoint3dArray clipPoints;
			pImg->getVertices(clipPoints);
			if (clipPoints.last() == clipPoints.first()) {
				clipPoints.removeLast();
			}
			gripPoints.append(clipPoints);
		}
	}
	return Result;
}

struct Coeff {
	int m_iUOrg;
	int m_iVOrg;
	int m_iUCf;
	int m_iVCf;
};

OdResult OdDbRasterImageGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto size {indices.size()};
	if (size == 0) {
		return eOk;
	}
	OdDbRasterImagePtr pImg = entity;
	if (!pImg->isClipped() || !pImg->isSetDisplayOpt(OdDbRasterImage::kClip)) {
		if (indices[0] == 0) {
			return entity->transformBy(OdGeMatrix3d::translation(offset));
		}
		OdGePoint3d origin;
		OdGeVector3d u, v;
		pImg->getOrientation(origin, u, v);
		OdGeVector3d xNorm = u, yNorm = v;
		const auto xNLen {xNorm.normalizeGetLength()};
		const auto yNLen {yNorm.normalizeGetLength()}; //Default bottom - left corner
		Coeff cfs = {1, 1, 1, 1};
		auto dX {xNorm.dotProduct(offset)};
		auto dY {yNorm.dotProduct(offset)};
		const auto dKoeffXY {xNLen / yNLen};
		switch (indices[0]) {
			case 2: //top - left corner
				cfs.m_iVOrg = 0;
				cfs.m_iVCf = -1;
				break;
			case 3: //top - right corner
				cfs.m_iVOrg = 0;
				cfs.m_iVCf = -1;
				cfs.m_iUOrg = 0;
				cfs.m_iUCf = -1;
				break;
			case 4: //bottom - right corner
				cfs.m_iUOrg = 0;
				cfs.m_iUCf = -1;
				break;
		}
		dY = cfs.m_iVCf * dY * dKoeffXY;
		dX = cfs.m_iUCf * dX;
		if (dX > xNLen) {
			dX = 2 * xNLen - dX;
		}
		if (dY > yNLen) { // For Ortho mode, either dX or dY is always 0.
			dY = 2 * yNLen - dY;
		}
		const auto dMov {fabs(dX) > fabs(dY) ? dX : dY};
		const auto vecU {xNorm * dMov};
		const auto vecV {yNorm * dMov * yNLen / xNLen};
		const auto pOrg {origin + cfs.m_iUOrg * vecU + cfs.m_iVOrg * vecV};
		if (!(u - vecU).isZeroLength() && !(v - vecV).isZeroLength()) {
			pImg->setOrientation(pOrg, u - vecU, v - vecV);
		}
	} else {
		OdGePoint3dArray clipPoints;
		pImg->getVertices(clipPoints);
		for (unsigned i = 0; i < indices.size(); ++i) {
			if (indices[i] >= 0 && (unsigned)indices[i] < clipPoints.size()) {
				clipPoints[indices[i]] += offset;
			}
		}
		clipPoints.last() = clipPoints.first();
		OdGePoint2dArray outBry;
		outBry.resize(clipPoints.size());
		const auto w2p {pImg->getPixelToModelTransform().invert()};
		const auto isize {pImg->imageSize()};
		const auto isWipeout {entity->isKindOf(OdDbWipeout::desc())};
		for (OdUInt32 setPt = 0; setPt < clipPoints.size(); setPt++) {
			outBry[setPt] = clipPoints[setPt].transformBy(w2p).convert2d();
			if (!isWipeout) {
				if (outBry[setPt].x < 0.0) {
					outBry[setPt].x = 0.0;
				} else if (outBry[setPt].x > isize.x) {
					outBry[setPt].x = isize.x;
				}
				if (outBry[setPt].y < 0.0) {
					outBry[setPt].y = 0.0;
				} else if (outBry[setPt].y > isize.y) {
					outBry[setPt].y = isize.y;
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

OdResult OdDbRasterImageGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*viewTransform*/, OdGePoint3dArray& snapPoints) const {
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd: case OdDb::kOsModeCen: {
			OdGePoint3dArray gripPoints;
			getGripPoints(entity, gripPoints);
			if (gripPoints.size() > 0) {
				if (objectSnapMode == OdDb::kOsModeEnd) {
					gripPoints.erase(gripPoints.begin());
				} else {
					gripPoints.erase(gripPoints.begin() + 1, gripPoints.end());
				}
				snapPoints.append(gripPoints);
			}
		}
			break;
		default:
			break;
	}
	return eOk;
}
