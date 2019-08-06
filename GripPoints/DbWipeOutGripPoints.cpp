#include <OdaCommon.h>
#include "DbWipeOutGripPoints.h"
#include <Ge/GeMatrix3d.h>
#include <Ge/GeScale3d.h>
#include <Ge/GeCircArc3d.h>
#include <DbWipeout.h>

OdResult OdDbWipeOutGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdGePoint3dArray odGePoint3dArray;
	OdDbWipeoutPtr(entity)->getVertices(odGePoint3dArray);
	// for the closed polyline boundary last coincident point is not returned as a grip
	if (odGePoint3dArray.size() > 2 && odGePoint3dArray.last().isEqualTo(odGePoint3dArray.first())) {
		odGePoint3dArray.removeLast();
	}
	gripPoints.append(odGePoint3dArray);
	return eOk;
}

OdResult OdDbWipeOutGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto size {indices.size()};
	if (size == 0) {
		return eOk;
	}
	OdDbWipeoutPtr pWipeout = entity;
	OdGePoint3dArray odGePoint3dArray;
	pWipeout->getVertices(odGePoint3dArray);
	// for the closed polyline boundary last coincident point is not returned as a grip
	const auto hasClosedPolylineBoundary {odGePoint3dArray.size() > 2 && odGePoint3dArray.last().isEqualTo(odGePoint3dArray.first())};
	for (unsigned i = 0; i < size; ++i) {
		if (indices[i] < (int)odGePoint3dArray.length()) {
			auto pt3D {odGePoint3dArray.getAt(indices[i])};
			pt3D += offset;
			odGePoint3dArray.setAt(indices[i], pt3D);
		}
	}
	if (hasClosedPolylineBoundary) {
		odGePoint3dArray[odGePoint3dArray.length() - 1] = odGePoint3dArray.first();
	}
	pWipeout->setBoundary(odGePoint3dArray);
	return eOk;
}

OdResult OdDbWipeOutGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbWipeOutGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbWipeOutGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& /*xWorldToEye*/, OdGePoint3dArray& snapPoints) const {
	OdDbWipeoutPtr Wipeout = entity;
	OdGePoint3dArray odGePoint3dArray;
	Wipeout->getVertices(odGePoint3dArray);
	const auto Size {snapPoints.size()};
	switch (objectSnapMode) {
		case OdDb::kOsModeEnd:
			getStretchPoints(entity, snapPoints);
			break;
		case OdDb::kOsModeMid: {
			snapPoints.resize(Size + odGePoint3dArray.length() - 1);
			for (unsigned i = 1; i < odGePoint3dArray.length(); ++i) {
				OdGeLine3d l(odGePoint3dArray.getAt(i - 1), odGePoint3dArray.getAt(i));
				snapPoints.append(l.evalPoint(l.paramOf(lastPoint)));
			}
			break;
		}
		case OdDb::kOsModeQuad: // not implemented yet
			break;
		case OdDb::kOsModeNear: {
			snapPoints.resize(Size + odGePoint3dArray.length() - 1);
			for (unsigned i = 1; i < odGePoint3dArray.length(); ++i) {
				auto Start {odGePoint3dArray.getAt(i - 1)};
				auto End {odGePoint3dArray.getAt(i)};
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
	}
	return eOk;
}
