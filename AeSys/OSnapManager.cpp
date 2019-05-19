// From Examples\Editor\OSnapManager.cpp (last compare 19.12)

#include "OdaCommon.h"
#include "OSnapManager.h"
#include "Gi/GiPathNode.h"
#include "OdRound.h"
#include "SaveState.h"
#define STL_USING_LIMITS
#include "OdaSTL.h"

#include "DbBlockTableRecord.h"
#include "DbBlockReference.h"
#include "DbLayout.h"
#include "DbHostAppServices.h"
#include "Gi/GiLocalDrawableDesc.h"
#include "Gs/GsViewImpl.h"

#define snapPtSize 5

OdBaseSnapManager::SubentId::SubentId(const OdGiPathNode& pathNode) {
	m_gsMarker = pathNode.selectionMarker();
	const OdGiPathNode* pGiPath = &pathNode;
	do {
		m_path.append(pGiPath->persistentDrawableId());
		pGiPath = pGiPath->parent();
	} while (pGiPath);
}

bool OdBaseSnapManager::SubentId::operator==(const SubentId& other) const {
	if (m_gsMarker != other.m_gsMarker) { return false; }

	if (m_path.size() != other.m_path.size()) { return false; }

	for (unsigned i = 0; i < m_path.size(); ++i) {

		if (m_path[i] != other.m_path[i]) { return false; }
	}
	return true;
}

#define hitradius 15

//#define maxhistory 7

OdBaseSnapManager::OdBaseSnapManager() noexcept
	: m_InputTracker(nullptr)
	, m_mode(OdDb::OsnapMode(0))
	, m_View(nullptr)
	, m_HitRadius(hitradius)
	, m_PickPoint(nullptr)
	, m_LastPoint(nullptr)
	, m_WorldToDevice(0.0)
	, m_NearDist(std::numeric_limits<double>::max())
	, m_Redraw(m_Redraw) {
}

OSnapManager::OSnapManager() noexcept
	: m_SnapModes(0xFFFFFFFF) {
}

unsigned OSnapManager::SnapModes() const noexcept{
	return m_SnapModes;
}

void OSnapManager::SetSnapModes(unsigned snapModes) noexcept {
	m_SnapModes = snapModes;
}

void OdBaseSnapManager::Reset() {
	m_centers.clear();
}

OdInt32 OdBaseSnapManager::GetAperture(OdDbDatabase* database) const {
	return database->appServices()->getAPERTURE();
}

void OdBaseSnapManager::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	OdGePoint3d pts[4];
	OdGiViewportGeometry& ViewportGeometry = viewportDraw->geometry();
	const OdGiViewport& Viewport = viewportDraw->viewport();
	const OdGeMatrix3d xWorldToEye = Viewport.getWorldToEyeTransform();
	Viewport.getNumPixelsInUnitSquare(Viewport.getCameraTarget(), (OdGePoint2d&) pts[0]);
	const double pix = 1. / pts[0].x;
	const double s = snapPtSize * pix;

	OdGiSubEntityTraits& traits = viewportDraw->subEntityTraits();
	OdGiDrawFlagsHelper _dfh(traits, OdGiSubEntityTraits::kDrawNoPlotstyle);
	if ((m_mode > 0) && ((OdUInt32) m_mode < 100)) {
		traits.setTrueColor(SnapTrueColor());

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
		viewportDraw->subEntityTraits().setTrueColor(CenterTrueColor());

		for (OdUInt32 i = 0; i < m_centers.size(); i++, gsMarker++) {
			traits.setSelectionMarker(gsMarker);

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

void OdBaseSnapManager::InvalidateViewport(const OdBaseSnapManager::HistEntryArray & centers) const {
	OdGePoint3d Point;
	const auto WorldToDeviceTransform {m_View->worldToDeviceMatrix()};

	OdGsDCRect DcRectangle;

	for (unsigned i = 0; i < centers.size(); ++i) {
		Point = WorldToDeviceTransform * centers[i].m_point;

		DcRectangle.m_min.x = OdRoundToLong(Point.x);
		DcRectangle.m_min.y = OdRoundToLong(Point.y);
		DcRectangle.m_max = DcRectangle.m_min;

		DcRectangle.m_min.x -= snapPtSize;
		DcRectangle.m_min.y -= snapPtSize;
		DcRectangle.m_max.x += snapPtSize;
		DcRectangle.m_max.y += snapPtSize;

		m_View->invalidate(DcRectangle);
	}
}

void OdBaseSnapManager::InvalidateViewport(const OdGePoint3d& point) const {

	OdGsDCRect DcRectangle;
	if (m_mode) {
		const auto WorldToDeviceTransform {m_View->worldToDeviceMatrix()};
		auto Point {WorldToDeviceTransform * point};
		DcRectangle.m_min.x = OdRoundToLong(Point.x);
		DcRectangle.m_min.y = OdRoundToLong(Point.y);
		DcRectangle.m_max = DcRectangle.m_min;

		DcRectangle.m_min.x -= (snapPtSize * 2);
		DcRectangle.m_min.y -= (snapPtSize * 2);
		DcRectangle.m_max.x += (snapPtSize * 2);
		DcRectangle.m_max.y += (snapPtSize * 2);

		  /*
		switch(m_mode)
		{
		case OdDb::kOsModeEnd:
		  DcRectangle.m_min.x -= snapPtSize;
		  DcRectangle.m_min.y -= snapPtSize;
		  DcRectangle.m_max.x += snapPtSize;
		  DcRectangle.m_max.y += snapPtSize;
		  break;

		case OdDb::kOsModeMid:
		  pts[1].set(pts[0].x - s * 1.2, pts[0].y - s * 0.6, 0.0);
		  pts[2].set(pts[0].x,           pts[0].y + s * 1.4, 0.0);
		  pts[3].set(pts[0].x + s * 1.2, pts[0].y - s * 0.6, 0.0);
		  geom.polygonEye(3, pts+1);

		  pts[1].set(pts[1].x - pix, pts[1].y - pix, 0.0);
		  pts[2].set(pts[2].x,       pts[2].y + pix, 0.0);
		  pts[3].set(pts[3].x + pix, pts[3].y - pix, 0.0);
		  geom.polygonEye(3, pts+1);
		  break;

		case OdDb::kOsModeCen:
		  geom.pushModelTransform(vp.getEyeToWorldTransform());
		  geom.circle(pts[0], s * 1.4, OdGeVector3d::kZAxis);
		  geom.popModelTransform();
		  break;

		case OdDb::kOsModeQuad:
		  pts[1].set(pts[0].x - s,  pts[0].y,     0.0);
		  pts[2].set(pts[0].x,      pts[0].y + s, 0.0);
		  pts[3].set(pts[0].x + s,  pts[0].y,     0.0);
		  pts[0].set(pts[0].x,      pts[0].y - s, 0.0);
		  geom.polygonEye(4, pts);
		  pts[1].set(pts[1].x - pix,  pts[1].y,       0.0);
		  pts[2].set(pts[2].x,        pts[2].y + pix, 0.0);
		  pts[3].set(pts[3].x + pix,  pts[3].y,       0.0);
		  pts[0].set(pts[0].x,        pts[0].y - pix, 0.0);
		  geom.polygonEye(4, pts);
		  break;

		case OdDb::kOsModePerp:
		  pts[1].set(pts[0].x - s,       pts[0].y + s + pix, 0.0);
		  pts[2].set(pts[0].x - s,       pts[0].y - s, 0.0);
		  pts[3].set(pts[0].x + s + pix, pts[0].y - s, 0.0);
		  geom.polylineEye(3, pts+1);
		  pts[1].set(pts[1].x - pix, pts[1].y,       0.0);
		  pts[2].set(pts[2].x - pix, pts[2].y - pix, 0.0);
		  pts[3].set(pts[3].x,       pts[3].y - pix, 0.0);
		  geom.polylineEye(3, pts+1);

		  pts[1].set(pts[0].x - s,  pts[0].y, 0.0);
		  pts[2].set(pts[0].x,      pts[0].y, 0.0);
		  pts[3].set(pts[0].x,      pts[0].y - s, 0.0);
		  geom.polylineEye(3, pts+1);
		  pts[1].set(pts[1].x - pix, pts[1].y + pix, 0.0);
		  pts[2].set(pts[2].x + pix, pts[2].y + pix, 0.0);
		  pts[3].set(pts[3].x + pix, pts[3].y,       0.0);
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
		m_View->invalidate(DcRectangle);
	}
}

OdUInt32 OdBaseSnapManager::subSetAttributes(OdGiDrawableTraits* drawableTraits) const {
	return kDrawableNone;
}

bool OdBaseSnapManager::subWorldDraw(OdGiWorldDraw* worldDraw) const {
	return false;
}

bool OdBaseSnapManager::Snap(OdGsView* view, OdGePoint3d& point, const OdGePoint3d* lastPoint) {
	OdEdPointTrackerWithSnapInfo* TrackerSnapInfo = dynamic_cast<OdEdPointTrackerWithSnapInfo*>(m_InputTracker);

	if (TrackerSnapInfo) { TrackerSnapInfo->m_SnapContext.bValid = false; }

	m_Redraw = false;
	m_SnapPoints.clear();
	m_View = view;
	m_PickPoint = &point;
	m_LastPoint = lastPoint;

	HistEntryArray prevCenters(m_centers);
	const OdGePoint3d prevPoint(m_snapPoint);
	const OdDb::OsnapMode prevMode(m_mode);

	if (m_mode == 0 || !Checkpoint(m_mode, m_snapPoint)) {
		m_NearDist = std::numeric_limits<double>::max();
		m_snapPoint = OdGePoint3d(1e100, 1e100, 1e100);
		m_mode = OdDb::OsnapMode(100);
	}

	const auto pt {(view->worldToDeviceMatrix() * point).convert2d()};
	OdGsDCPoint pts[2];
	const auto aperture {GetAperture(static_cast<OdDbDatabase*>(view->userGiContext()->database()))};

	pts[0].x = OdRoundToLong(pt.x) - aperture;
	pts[1].x = pts[0].x + aperture * 2;
	pts[0].y = OdRoundToLong(pt.y) - aperture;
	pts[1].y = pts[0].y + aperture * 2;
	m_HitRadius = (double) aperture;
	m_WorldToDevice = view->worldToDeviceMatrix().getCsXAxis().length();

	OdGsViewImpl* pViewImpl = dynamic_cast<OdGsViewImpl*>(view);
	
	if (pViewImpl) { pViewImpl->setSnapping(true); }

	m_selectedEntityDataArray.clear();

	view->select(pts, 2, this);

	if (m_selectedEntityDataArray.size()) { // dna: only one can be selected currently
		CheckSnapPoints(m_selectedEntityDataArray[0], pViewImpl->worldToEyeMatrix());
	}

	if (pViewImpl) { pViewImpl->setSnapping(false); }

	if (m_mode > 0 && (OdUInt32) m_mode < 100) {
		point = m_snapPoint;
	} else {
		if (prevMode > 0 && (OdUInt32)prevMode < 100) { InvalidateViewport(prevPoint); }

		m_mode = OdDb::OsnapMode(0);
	}
	bool bRes = true;
	if (m_snapPoint == prevPoint) {
		bRes = false;
	} else {
		if (prevPoint.x < 1e100) { InvalidateViewport(prevPoint); }
		
		if (m_snapPoint.x < 1e100) { InvalidateViewport(m_snapPoint); }
	}
	return bRes | m_Redraw;
}

bool OdBaseSnapManager::selected(const OdGiDrawableDesc&) {
	return false;
}

inline bool OdBaseSnapManager::Checkpoint(OdDb::OsnapMode objectSnapMode, const OdGePoint3d & point) {

	const auto WorldToDeviceTransform {m_View->worldToDeviceMatrix()};
	const auto p1((WorldToDeviceTransform * *m_PickPoint).convert2d());
	const auto p2((WorldToDeviceTransform * point).convert2d());
	const double dist {(p1 - p2).length()};

	auto TrackerSnapInfo {dynamic_cast<OdEdPointTrackerWithSnapInfo*>(m_InputTracker)};

	if (dist < m_HitRadius) {
		if (dist < m_NearDist) {
			m_NearDist = dist;
			m_snapPoint = point;
			m_mode = objectSnapMode;

			if (TrackerSnapInfo) {
				TrackerSnapInfo->m_SnapContext.point = point;
				TrackerSnapInfo->m_SnapContext.mode = objectSnapMode;
				TrackerSnapInfo->m_SnapContext.bValid = true;
			}
			return true;
		} else if (dist == m_NearDist)
			return true;
	}
	return false;
}

const int nMaxHist = 7;

bool OdBaseSnapManager::AppendToQueue(OdBaseSnapManager::HistEntryArray & array, const HistEntry & entry) {
	if (!array.contains(entry)) {
		if (array.size() > nMaxHist) {
			array.erase(array.begin());
		}
		array.append(entry);
		return true;
	}
	return false;
}

void OdBaseSnapManager::CheckSnapPoints(const SelectedEntityData& data, const OdGeMatrix3d& xWorldToEye) {
	const auto xModelToWorld {data.xModelToWorld};
	const bool insertionMatrix = (xModelToWorld != OdGeMatrix3d::kIdentity);
	const auto modelPickPt {xModelToWorld * *m_PickPoint};
	OdGePoint3d modelLastPt;
	unsigned nSnapModes = SnapModes();
	
	if (m_LastPoint) {
		SETBIT(nSnapModes, ToSnapModes(OdDb::kOsModePerp) | ToSnapModes(OdDb::kOsModeTan) | ToSnapModes(OdDb::kOsModePerp), 0);
		modelLastPt = xModelToWorld * *m_LastPoint;
	}
	OdEdPointTrackerWithSnapInfo* pTrackerSnapInfo = dynamic_cast<OdEdPointTrackerWithSnapInfo*>(m_InputTracker);

	OdDbEntityPtr Entity = data.subentId.m_path.first().safeOpenObject();
	const OdGsMarker gsMarker = data.subentId.m_gsMarker;
	
	if (!pTrackerSnapInfo) {
		for (OdDb::OsnapMode ObjectSnapMode = OdDb::kOsModeEnd; ObjectSnapMode <= OdDb::kOsModeNear; ObjectSnapMode = OdDb::OsnapMode(ObjectSnapMode + 1)) {
	
			if (nSnapModes & ToSnapModes(ObjectSnapMode)) // so not all types are tested
			{
				OdResult res;
				if (insertionMatrix) {
					res = Entity->getOsnapPoints(ObjectSnapMode, gsMarker, modelPickPt, modelLastPt, xWorldToEye, m_SnapPoints, xModelToWorld);
				} else {
					res = Entity->getOsnapPoints(ObjectSnapMode, gsMarker, modelPickPt, modelLastPt, xWorldToEye, m_SnapPoints);
				}
				if (res == eOk) {

					for (unsigned i = 0; i < m_SnapPoints.size(); ++i) {
						OdGePoint3d& point = m_SnapPoints[i];
						point.transformBy(xModelToWorld);
						Checkpoint(ObjectSnapMode, point);
						switch (ObjectSnapMode) {
							case OdDb::kOsModeCen:
								AppendToQueue(m_centers, HistEntry(data.subentId, point));
								m_Redraw = true;
								break;
							default:
							  // no op
								break;
						}
					}
					m_SnapPoints.clear();
				}
			}
		}
	}
	else {
		if (!pTrackerSnapInfo->isTargetEntity(Entity)) { return; }

		OdSaveState<double> ssHitRadius(m_HitRadius, 1500.);

		OdArray<OdDb::OsnapMode> snapModes;

		pTrackerSnapInfo->getSnapModes(Entity, snapModes);

		OdArray<OdDb::OsnapMode>::iterator it;

		for (it = snapModes.begin(); it != snapModes.end(); it++) {

			if (Entity->getOsnapPoints(*it, gsMarker, modelPickPt, modelLastPt, xWorldToEye, m_SnapPoints) == eOk) {
				pTrackerSnapInfo->m_SnapContext.entId = Entity->objectId();
				pTrackerSnapInfo->m_SnapContext.marker = gsMarker;

				for (unsigned i = 0; i < m_SnapPoints.size(); ++i) {
					OdGePoint3d& point = m_SnapPoints[i];
					point.transformBy(xModelToWorld);
					Checkpoint(*it, point);
				}

			}
		}
	}
}

OdUInt32 OdBaseSnapManager::selected(const OdGiPathNode & pathNode, const OdGiViewport & viewInfo) {
	if (pathNode.transientDrawable() == this) {
		const OdGsMarker gsMarker = pathNode.selectionMarker();

		if (gsMarker > -1) {
			if ((SnapModes() & ToSnapModes(OdDb::kOsModeCen)) && (OdGsMarker) m_centers.size() > gsMarker) {
				Checkpoint(OdDb::kOsModeCen, m_centers[gsMarker].m_point);
			}
		}
		return OdUInt32(kContinue);
	}

	auto Entity {OdDbEntity::cast(OdDbObjectId(pathNode.persistentDrawableId()).openObject())};

	if (Entity.isNull()) { return OdUInt32(kSkipDrawable); }

	m_selectedEntityDataArray.append()->set(pathNode);

	return OdUInt32(kSkipDrawable);
}

void OdBaseSnapManager::RecalculateEntityCenters() {
	for (int i = m_centers.size() - 1; i >= 0; --i) {
		SubentId sub_id = m_centers[i].m_subentId;

		if (sub_id.m_path.size() <= 0) continue;

		auto Entity {OdDbEntity::cast(sub_id.m_path[0].openObject())};

		if (Entity.isNull()) {
			m_centers.erase(m_centers.begin() + i);
			continue;
		}

		OdGePoint3dArray snapPoints;
		Entity->getOsnapPoints(OdDb::kOsModeCen, OdGsMarker(), OdGePoint3d(), OdGePoint3d(), OdGeMatrix3d(), snapPoints);

		if (snapPoints.size() > 0) {
			m_centers[i].m_point = snapPoints[0]; // recalculation center
		}

	}
}

bool OdBaseSnapManager::SetEntityCenters(OdRxObject* rxObject) {
	m_centers.clear();

	OdDbDatabase* Database = OdDbDatabase::cast(rxObject).get();
	
	if (!Database) { return false; }

	OdDbBlockTableRecordPtr BlockTableRecord = Database->getActiveLayoutBTRId().safeOpenObject();  // Layout table

	if (Database->getModelSpaceId() != BlockTableRecord->objectId()) { // it's not ModelSpace, it's PaperSpace which can have many ModelSpace
		OdDbLayoutPtr pLayout = BlockTableRecord->getLayoutId().safeOpenObject();

		if (pLayout->overallVportId() != OdDbObjectId(Database->activeViewportId())) {
			BlockTableRecord = Database->getModelSpaceId().safeOpenObject(); // get active ModelSpace for PaperSpace
		}
	}
	SetEntityCenters(BlockTableRecord);
	return true;
}

void OdBaseSnapManager::SetEntityCenters(OdDbBlockTableRecord* blockTableRecord, const OdGeMatrix3d& matrix) {
	OdGiDrawableDesc* dd0 = NULL;
	OdGiLocalDrawableDesc dd(dd0); // need for build OdGiPathNode

	for (OdDbObjectIteratorPtr pIter = blockTableRecord->newIterator(); !pIter->done() && m_centers.size() < nMaxHist; pIter->step()) {
		OdDbEntityPtr Entity = pIter->entity();

		auto BlockReference {OdDbBlockReference::cast(Entity)};

		if (!BlockReference.isNull()) {
			auto BlockTableRecord {OdDbBlockTableRecord::cast(BlockReference->blockTableRecord().openObject())};

			if (!BlockTableRecord.isNull()) {
				SetEntityCenters(BlockTableRecord, BlockReference->blockTransform());
			}
			continue;
		}
		if (Entity.isNull()) continue;

		dd.persistId = Entity->objectId();

		OdGePoint3dArray snapPoints;
		Entity->getOsnapPoints(OdDb::kOsModeCen, OdGsMarker(), OdGePoint3d::kOrigin, OdGePoint3d::kOrigin, OdGeMatrix3d(), snapPoints);

		for (OdUInt32 i = 0; i < snapPoints.size() && m_centers.size() < nMaxHist; i++) {
			m_centers.append(HistEntry(dd, snapPoints[i].transformBy(matrix)));
		}
	}
}

void OdBaseSnapManager::Track(OdEdInputTracker* inputTracker) {
	m_InputTracker = inputTracker;
}
