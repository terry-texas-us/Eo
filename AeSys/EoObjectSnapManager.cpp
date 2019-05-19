#include "stdafx.h"

#include "EoObjectSnapManager.h"
#include "Gi/GiPathNode.h"
#include "Gi/GiViewportDraw.h"
#include "Gi/GiViewportGeometry.h"

#include "OdRound.h"
#define STL_USING_LIMITS
#include "OdaSTL.h"

#define snapPtSize 5

EoObjectSnapManager::EoObjectSnapManager()
	: m_pView(0)
	, m_pPickPoint(0)
	, m_pLastPoint(0)
	, m_nSnapModes(0xFFFFFFFF)
	, m_dWorldToDevice(0.)
	, m_dNearDist(0.)
	, m_mode(OdDb::OsnapMode(0))
	, m_bRedraw(false) {
}

EoObjectSnapManager::SubentId::SubentId(const OdGiPathNode& pathNode) {
	m_gsMarker = pathNode.selectionMarker();
	const OdGiPathNode* pGiPath = &pathNode;
	do {
		m_path.append(pGiPath->persistentDrawableId());
		pGiPath = pGiPath->parent();
	} while (pGiPath);
}
bool EoObjectSnapManager::SubentId::operator==(const SubentId& other) const {
	if (m_gsMarker != other.m_gsMarker) {
		return false;
	}
	if (m_path.size() != other.m_path.size()) {
		return false;
	}
	OdGePoint3dArray::size_type i;
	for (i = 0; i < m_path.size(); ++i) {
		if (m_path[i] != other.m_path[i]) {
			return false;
		}
	}
	return true;
}

void EoObjectSnapManager::reset() {
	m_centers.clear();
}
void EoObjectSnapManager::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	OdGePoint3d pts[4];
	OdGiViewportGeometry& ViewportGeometry = viewportDraw->geometry();
	const OdGiViewport& Viewport = viewportDraw->viewport();
	const OdGeMatrix3d xWorldToEye = Viewport.getWorldToEyeTransform();
	Viewport.getNumPixelsInUnitSquare(Viewport.getCameraTarget(), (OdGePoint2d&) pts[0]);
	const double pix = 1. / pts[0].x;
	const double s = snapPtSize * pix;

	OdGiSubEntityTraits& traits = viewportDraw->subEntityTraits();
	OdGiDrawFlagsHelper _dfh(traits, OdGiSubEntityTraits::kDrawNoPlotstyle);
	if (m_mode) {
		traits.setColor(2);
		traits.setFillType(kOdGiFillNever);
		traits.setSelectionMarker(kNullSubentIndex);
		pts[0] = xWorldToEye * m_snapPoint;
		Viewport.doPerspective(pts[0]);

		switch (m_mode) {
			case OdDb::kOsModeEnd:
				pts[1].set(pts[0].x - s, pts[0].y + s, 0.0);
				pts[2].set(pts[0].x - s, pts[0].y - s, 0.0);
				pts[3].set(pts[0].x + s, pts[0].y - s, 0.0);
				pts[0].set(pts[0].x + s, pts[0].y + s, 0.0);
				ViewportGeometry.polygonEye(4, pts);
				pts[1].set(pts[1].x - pix, pts[1].y + pix, 0.0);
				pts[2].set(pts[2].x - pix, pts[2].y - pix, 0.0);
				pts[3].set(pts[3].x + pix, pts[3].y - pix, 0.0);
				pts[0].set(pts[0].x + pix, pts[0].y + pix, 0.0);
				ViewportGeometry.polygonEye(4, pts);
				break;

			case OdDb::kOsModeMid:
				pts[1].set(pts[0].x - s * 1.2, pts[0].y - s * 0.6, 0.0);
				pts[2].set(pts[0].x, pts[0].y + s * 1.4, 0.0);
				pts[3].set(pts[0].x + s * 1.2, pts[0].y - s * 0.6, 0.0);
				ViewportGeometry.polygonEye(3, pts + 1);

				pts[1].set(pts[1].x - pix, pts[1].y - pix, 0.0);
				pts[2].set(pts[2].x, pts[2].y + pix, 0.0);
				pts[3].set(pts[3].x + pix, pts[3].y - pix, 0.0);
				ViewportGeometry.polygonEye(3, pts + 1);
				break;

			case OdDb::kOsModeCen:
			{
				OdGiModelTransformSaver mt(ViewportGeometry, Viewport.getEyeToWorldTransform());
				ViewportGeometry.circle(pts[0], s * 1.4, OdGeVector3d::kZAxis);
			}
			break;

			case OdDb::kOsModeQuad:
				pts[1].set(pts[0].x - s, pts[0].y, 0.0);
				pts[2].set(pts[0].x, pts[0].y + s, 0.0);
				pts[3].set(pts[0].x + s, pts[0].y, 0.0);
				pts[0].set(pts[0].x, pts[0].y - s, 0.0);
				ViewportGeometry.polygonEye(4, pts);
				pts[1].set(pts[1].x - pix, pts[1].y, 0.0);
				pts[2].set(pts[2].x, pts[2].y + pix, 0.0);
				pts[3].set(pts[3].x + pix, pts[3].y, 0.0);
				pts[0].set(pts[0].x, pts[0].y - pix, 0.0);
				ViewportGeometry.polygonEye(4, pts);
				break;

			case OdDb::kOsModePerp:
				pts[1].set(pts[0].x - s, pts[0].y + s + pix, 0.0);
				pts[2].set(pts[0].x - s, pts[0].y - s, 0.0);
				pts[3].set(pts[0].x + s + pix, pts[0].y - s, 0.0);
				ViewportGeometry.polylineEye(3, pts + 1);
				pts[1].set(pts[1].x - pix, pts[1].y, 0.0);
				pts[2].set(pts[2].x - pix, pts[2].y - pix, 0.0);
				pts[3].set(pts[3].x, pts[3].y - pix, 0.0);
				ViewportGeometry.polylineEye(3, pts + 1);

				pts[1].set(pts[0].x - s, pts[0].y, 0.0);
				pts[2].set(pts[0].x, pts[0].y, 0.0);
				pts[3].set(pts[0].x, pts[0].y - s, 0.0);
				ViewportGeometry.polylineEye(3, pts + 1);
				pts[1].set(pts[1].x - pix, pts[1].y + pix, 0.0);
				pts[2].set(pts[2].x + pix, pts[2].y + pix, 0.0);
				pts[3].set(pts[3].x + pix, pts[3].y, 0.0);
				ViewportGeometry.polylineEye(3, pts + 1);
				break;

			case OdDb::kOsModeTan:
			{
				OdGiModelTransformSaver mt(ViewportGeometry, Viewport.getEyeToWorldTransform());
				ViewportGeometry.circle(pts[0], s, OdGeVector3d::kZAxis);
			}
			pts[1].set(pts[0].x - s, pts[0].y + s, 0.0);
			pts[2].set(pts[0].x + s, pts[0].y + s, 0.0);
			ViewportGeometry.polylineEye(2, pts + 1);
			pts[1].set(pts[1].x, pts[1].y + pix, 0.0);
			pts[2].set(pts[2].x, pts[2].y + pix, 0.0);
			ViewportGeometry.polylineEye(2, pts + 1);
			break;

			case OdDb::kOsModeNear:
				pts[1].set(pts[0].x - s, pts[0].y + s, 0.0);
				pts[2].set(pts[0].x + s, pts[0].y - s, 0.0);
				pts[3].set(pts[0].x - s, pts[0].y - s, 0.0);
				pts[0].set(pts[0].x + s, pts[0].y + s, 0.0);
				ViewportGeometry.polygonEye(4, pts);
				pts[1].set(pts[1].x - pix, pts[1].y + pix, 0.0);
				pts[2].set(pts[2].x + pix, pts[2].y - pix, 0.0);
				pts[3].set(pts[3].x - pix, pts[3].y - pix, 0.0);
				pts[0].set(pts[0].x + pix, pts[0].y + pix, 0.0);
				ViewportGeometry.polygonEye(4, pts);
				break;

			default:
				pts[1].set(pts[0].x - s, pts[0].y + s, 0.0);
				pts[2].set(pts[0].x + s, pts[0].y - s, 0.0);
				ViewportGeometry.polygonEye(2, pts + 1);
				pts[1].set(pts[0].x - s, pts[0].y - s, 0.0);
				pts[2].set(pts[0].x + s, pts[0].y + s, 0.0);
				ViewportGeometry.polygonEye(2, pts + 1);
				pts[1].set(pts[0].x - s - pix, pts[0].y + s + pix, 0.0);
				pts[2].set(pts[0].x - s - pix, pts[0].y - s - pix, 0.0);
				pts[3].set(pts[0].x + s + pix, pts[0].y - s - pix, 0.0);
				pts[0].set(pts[0].x + s + pix, pts[0].y + s + pix, 0.0);
				ViewportGeometry.polygonEye(4, pts);
				break;
		}
	}
	OdGsMarker gsMarker = 0;
	if (m_centers.size()) {
		OdGePoint3dArray::size_type i;
		viewportDraw->subEntityTraits().setColor(7);
		for (i = 0; i < m_centers.size(); ++i) {
			traits.setSelectionMarker(++gsMarker);
			const OdGePoint3d& cntr = xWorldToEye * m_centers[i].m_point;
			pts[0].set(cntr.x, cntr.y + s, 0.0);
			pts[1].set(cntr.x, cntr.y - s, 0.0);
			ViewportGeometry.polygonEye(2, pts);
			pts[0].set(cntr.x + s, cntr.y, 0.0);
			pts[1].set(cntr.x - s, cntr.y, 0.0);
			ViewportGeometry.polygonEye(2, pts);
		}
	}
}

void EoObjectSnapManager::invalidateViewport(const EoObjectSnapManager::HistEntryArray & centers) const {
	OdGePoint3d pt;
	const OdGeMatrix3d xWorldToScr = m_pView->worldToDeviceMatrix();

	OdGsDCRect rc;
	OdGePoint3dArray::size_type i;
	for (i = 0; i < centers.size(); ++i) {
		pt = xWorldToScr * centers[i].m_point;

		rc.m_min.x = OdRoundToLong(pt.x);
		rc.m_min.y = OdRoundToLong(pt.y);
		rc.m_max = rc.m_min;

		rc.m_min.x -= snapPtSize;
		rc.m_min.y -= snapPtSize;
		rc.m_max.x += snapPtSize;
		rc.m_max.y += snapPtSize;

		m_pView->invalidate(rc);
	}
}

void EoObjectSnapManager::invalidateViewport(const OdGePoint3d & point) const {
	OdGePoint3d pt;
	const OdGeMatrix3d xWorldToScr = m_pView->worldToDeviceMatrix();

	OdGsDCRect rc;
	if (m_mode) {
		pt = xWorldToScr * point;
		rc.m_min.x = OdRoundToLong(pt.x);
		rc.m_min.y = OdRoundToLong(pt.y);
		rc.m_max = rc.m_min;

		rc.m_min.x -= (snapPtSize * 2);
		rc.m_min.y -= (snapPtSize * 2);
		rc.m_max.x += (snapPtSize * 2);
		rc.m_max.y += (snapPtSize * 2);

		/*
		switch(m_mode)
		{
		case OdDb::kOsModeEnd:
		rc.m_min.x -= snapPtSize;
		rc.m_min.y -= snapPtSize;
		rc.m_max.x += snapPtSize;
		rc.m_max.y += snapPtSize;
		break;

		case OdDb::kOsModeMid:
		pts[1].set(pts[0].x - s * 1.2, pts[0].y - s * 0.6, 0.0);
		pts[2].set(pts[0].x, pts[0].y + s * 1.4, 0.0);
		pts[3].set(pts[0].x + s * 1.2, pts[0].y - s * 0.6, 0.0);
		geom.polygonEye(3, pts+1);

		pts[1].set(pts[1].x - pix, pts[1].y - pix, 0.0);
		pts[2].set(pts[2].x, pts[2].y + pix, 0.0);
		pts[3].set(pts[3].x + pix, pts[3].y - pix, 0.0);
		geom.polygonEye(3, pts+1);
		break;

		case OdDb::kOsModeCen:
		geom.pushModelTransform(vp.getEyeToWorldTransform());
		geom.circle(pts[0], s * 1.4, OdGeVector3d::kZAxis);
		geom.popModelTransform();
		break;

		case OdDb::kOsModeQuad:
		pts[1].set(pts[0].x - s, pts[0].y, 0.0);
		pts[2].set(pts[0].x, pts[0].y + s, 0.0);
		pts[3].set(pts[0].x + s,  pts[0].y, 0.0);
		pts[0].set(pts[0].x, pts[0].y - s, 0.0);
		geom.polygonEye(4, pts);
		pts[1].set(pts[1].x - pix,  pts[1].y, 0.0);
		pts[2].set(pts[2].x, pts[2].y + pix, 0.0);
		pts[3].set(pts[3].x + pix,  pts[3].y, 0.0);
		pts[0].set(pts[0].x, pts[0].y - pix, 0.0);
		geom.polygonEye(4, pts);
		break;

		case OdDb::kOsModePerp:
		pts[1].set(pts[0].x - s, pts[0].y + s + pix, 0.0);
		pts[2].set(pts[0].x - s, pts[0].y - s, 0.0);
		pts[3].set(pts[0].x + s + pix, pts[0].y - s, 0.0);
		geom.polylineEye(3, pts+1);
		pts[1].set(pts[1].x - pix, pts[1].y, 0.0);
		pts[2].set(pts[2].x - pix, pts[2].y - pix, 0.0);
		pts[3].set(pts[3].x, pts[3].y - pix, 0.0);
		geom.polylineEye(3, pts+1);

		pts[1].set(pts[0].x - s,  pts[0].y, 0.0);
		pts[2].set(pts[0].x, pts[0].y, 0.0);
		pts[3].set(pts[0].x, pts[0].y - s, 0.0);
		geom.polylineEye(3, pts+1);
		pts[1].set(pts[1].x - pix, pts[1].y + pix, 0.0);
		pts[2].set(pts[2].x + pix, pts[2].y + pix, 0.0);
		pts[3].set(pts[3].x + pix, pts[3].y, 0.0);
		geom.polylineEye(3, pts+1);
		break;

		case OdDb::kOsModeTan:
		geom.pushModelTransform(vp.getEyeToWorldTransform());
		geom.circle(pts[0], s, OdGeVector3d::kZAxis);
		geom.popModelTransform();
		pts[1].set(pts[0].x - s, pts[0].y + s, 0.0);
		pts[2].set(pts[0].x + s, pts[0].y + s, 0.0);
		geom.polylineEye(2, pts+1);
		pts[1].set(pts[1].x, pts[1].y + pix, 0.0);
		pts[2].set(pts[2].x, pts[2].y + pix, 0.0);
		geom.polylineEye(2, pts+1);
		break;

		case OdDb::kOsModeNear:
		pts[1].set(pts[0].x - s, pts[0].y + s, 0.0);
		pts[2].set(pts[0].x + s, pts[0].y - s, 0.0);
		pts[3].set(pts[0].x - s, pts[0].y - s, 0.0);
		pts[0].set(pts[0].x + s, pts[0].y + s, 0.0);
		geom.polygonEye(4, pts);
		pts[1].set(pts[1].x - pix, pts[1].y + pix, 0.0);
		pts[2].set(pts[2].x + pix, pts[2].y - pix, 0.0);
		pts[3].set(pts[3].x - pix, pts[3].y - pix, 0.0);
		pts[0].set(pts[0].x + pix, pts[0].y + pix, 0.0);
		geom.polygonEye(4, pts);
		break;

		default:
		pts[1].set(pts[0].x - s, pts[0].y + s, 0.0);
		pts[2].set(pts[0].x + s, pts[0].y - s, 0.0);
		geom.polygonEye(2, pts + 1);
		pts[1].set(pts[0].x - s, pts[0].y - s, 0.0);
		pts[2].set(pts[0].x + s, pts[0].y + s, 0.0);
		geom.polygonEye(2, pts + 1);
		pts[1].set(pts[0].x - s - pix, pts[0].y + s + pix, 0.0);
		pts[2].set(pts[0].x - s - pix, pts[0].y - s - pix, 0.0);
		pts[3].set(pts[0].x + s + pix, pts[0].y - s - pix, 0.0);
		pts[0].set(pts[0].x + s + pix, pts[0].y + s + pix, 0.0);
		geom.polygonEye(4, pts);
		break;
		}
		*/
		m_pView->invalidate(rc);
	}
}
OdUInt32 EoObjectSnapManager::subSetAttributes(OdGiDrawableTraits * drawableTraits) const noexcept {
	return kDrawableNone;
}
bool EoObjectSnapManager::subWorldDraw(OdGiWorldDraw * worldDraw) const noexcept {
	return false;
}

#define hitradius 15

#define maxhistory 7

bool EoObjectSnapManager::snap(OdGsView * view, OdGePoint3d & point, const OdGePoint3d * lastPoint) {
	m_bRedraw = false;
	m_snapPointsBuff.clear();
	m_pView = view;
	m_pPickPoint = &point;
	m_pLastPoint = lastPoint;

	HistEntryArray prevCenters(m_centers);
	const OdGePoint3d prevPoint(m_snapPoint);
	const OdDb::OsnapMode prevMode(m_mode);

	if (m_mode == 0 || !checkpoint(m_mode, m_snapPoint)) {
		m_dNearDist = std::numeric_limits<double>::max();
		m_snapPoint = OdGePoint3d(1e100, 1e100, 1e100);
		m_mode = OdDb::OsnapMode(100);
	}
	const OdGePoint2d pt = (view->worldToDeviceMatrix() * point).convert2d();
	OdGsDCPoint pts[2];
	pts[0].x = OdRoundToLong(pt.x) - hitradius;
	pts[1].x = pts[0].x + hitradius * 2;
	pts[0].y = OdRoundToLong(pt.y) - 5;
	pts[1].y = pts[0].y + hitradius * 2;
	m_dWorldToDevice = view->worldToDeviceMatrix().getCsXAxis().length();
	view->select(pts, 2, this);

	/*
	unsigned SnapModes = m_nSnapModes & (OdDb::kOsModeCen | OdDb::kOsModeIns | OdDb::kOsModePerp | OdDb::kOsModeTan | OdDb::kOsModePar);
	OdGeMatrix3d xWorldToEye = pView->viewingMatrix();

	for (unsigned i = 0; i < m_history.size(); ++i) {
		const HistEntry& entry = m_history[i];
		OdDbEntityPtr Entity = entry.m_objectId.safeOpenObject();
		checkSnapPoints(Entity, SnapModes, entry.m_gsMarker, entry.m_xModelToWorld, entry.m_xModelToWorld.inverse(), xWorldToEye);
	}
	*/
	if (m_mode > 0 && m_mode < 100) {
		point = m_snapPoint;
		/*
		if (m_history.size() > maxhistory)
		m_history.erase(m_history.begin());
		HistEntry& histEntry = *m_history.append();
		*/
	} else {
		if (prevMode > 0 && prevMode < 100)
			invalidateViewport(prevPoint);
		m_mode = OdDb::OsnapMode(0);
	}
	bool bRes = true;
	if (m_snapPoint == prevPoint)
		bRes = false;
	else {
		if (prevPoint.x < 1e100)
			invalidateViewport(prevPoint);
		if (m_snapPoint.x < 1e100)
			invalidateViewport(m_snapPoint);
	}
	return bRes | m_bRedraw;
}

unsigned EoObjectSnapManager::snapModes() const noexcept {
	return m_nSnapModes;
}

void EoObjectSnapManager::SetSnapModes(unsigned snapModes) noexcept {
	m_nSnapModes = snapModes;
}

bool EoObjectSnapManager::selected(const OdGiDrawableDesc&) noexcept {
	return false;
}

bool EoObjectSnapManager::checkpoint(OdDb::OsnapMode osm, const OdGePoint3d & point) {
	//double dist = (point - *m_pPickPoint).length() * m_dWorldToDevice;
	const OdGeMatrix3d xWorldToScr = m_pView->worldToDeviceMatrix();
	const OdGePoint2d p1((xWorldToScr * *m_pPickPoint).convert2d());
	const OdGePoint2d p2((xWorldToScr * point).convert2d());
	const double dist = (p1 - p2).length();

	if (dist < hitradius) {
		if (dist < m_dNearDist && osm <= m_mode) {
			m_dNearDist = dist;
			m_snapPoint = point;
			m_mode = osm;
			return true;
		}
	}
	return false;
}

const int nMaxHist = 7;

//template <class A, class Size>
bool EoObjectSnapManager::appendToQueue(EoObjectSnapManager::HistEntryArray & array, const HistEntry & entry) {
	if (!array.contains(entry)) {
		if (array.size() > nMaxHist) {
			array.erase(array.begin());
		}
		array.append(entry);
		return true;
	}
	return false;
}

void EoObjectSnapManager::checkSnapPoints(const OdDbEntity * entity, const OdGiPathNode & pathNode, unsigned snapModes, OdGsMarker gsMarker, const OdGeMatrix3d & xModelToWorld, const OdGeMatrix3d & xWorldToModel, const OdGeMatrix3d & xWorldToEye) {
	const OdGePoint3d modelPickPt = xWorldToModel * *m_pPickPoint;
	OdGePoint3d modelLastPt;
	if (m_pLastPoint) {
		modelLastPt = xWorldToModel * *m_pLastPoint;
	}
	for (OdDb::OsnapMode osm = OdDb::kOsModeEnd; osm <= OdDb::kOsModeNear; osm = OdDb::OsnapMode(osm + 1)) {
		if (snapModes & (1 << osm)) {
			if (entity->getOsnapPoints(OdDb::OsnapMode(osm), gsMarker, modelPickPt, modelLastPt, xWorldToEye, m_snapPointsBuff) == eOk) {
				OdGePoint3dArray::size_type i;
				for (i = 0; i < m_snapPointsBuff.size(); ++i) {
					OdGePoint3d& point = m_snapPointsBuff[i];
					point.transformBy(xModelToWorld);
					checkpoint(OdDb::OsnapMode(osm), point);
					switch (osm) {
						case OdDb::kOsModeCen:
							appendToQueue(m_centers, HistEntry(pathNode, point));
							m_bRedraw = true;
							break;
					}
				}
				m_snapPointsBuff.clear();
			}
		}
	}
}
OdUInt32 EoObjectSnapManager::selected(const OdGiPathNode & pathNode, const OdGiViewport & viewport) {
	if (pathNode.transientDrawable() == this) {
		const OdGsMarker gsMarker = pathNode.selectionMarker();
		if (gsMarker > -1) {
			if ((m_nSnapModes & (1 << OdDb::kOsModeCen)) && (OdGsMarker) m_centers.size() > gsMarker) {
				checkpoint(OdDb::kOsModeCen, m_centers[gsMarker].m_point);
			}
		}
		return OdUInt32(kContinue);
	}
	OdDbEntityPtr Entity = OdDbEntity::cast(OdDbObjectId(pathNode.persistentDrawableId()).openObject());
	if (Entity.isNull())
		return OdUInt32(kSkipDrawable);

	const OdGeMatrix3d xModelToWorld = viewport.getEyeToWorldTransform() * viewport.getModelToEyeTransform();
	const OdGeMatrix3d xWorldToModel = viewport.getEyeToModelTransform() * viewport.getWorldToEyeTransform();

	unsigned SnapModes = m_nSnapModes;
	if (!m_pLastPoint) {
		SETBIT(SnapModes, (1 << OdDb::kOsModePerp) | (1 << OdDb::kOsModeTan) | (1 << OdDb::kOsModePerp), 0);
	}
	checkSnapPoints(Entity, pathNode, SnapModes, pathNode.selectionMarker(), xModelToWorld, xWorldToModel, viewport.getWorldToEyeTransform());

	return OdUInt32(kContinue);
}
