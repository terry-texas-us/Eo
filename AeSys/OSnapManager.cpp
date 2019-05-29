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
	m_Marker = pathNode.selectionMarker();
	const OdGiPathNode* PathNode = &pathNode;
	do {
		m_Path.append(PathNode->persistentDrawableId());
		PathNode = PathNode->parent();
	} while (PathNode);
}

bool OdBaseSnapManager::SubentId::operator==(const SubentId& other) const {
	if (m_Marker != other.m_Marker) { return false; }

	if (m_Path.size() != other.m_Path.size()) { return false; }

	for (unsigned i = 0; i < m_Path.size(); ++i) {

		if (m_Path[i] != other.m_Path[i]) { return false; }
	}
	return true;
}

#define hitradius 15

//#define maxhistory 7

OdBaseSnapManager::OdBaseSnapManager() noexcept
	: m_InputTracker(nullptr)
	, m_SnapMode(OdDb::OsnapMode(0))
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
	m_Centers.clear();
}

long OdBaseSnapManager::GetAperture(OdDbDatabase* database) const {
	return database->appServices()->getAPERTURE();
}

void OdBaseSnapManager::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	OdGePoint3d Points[4];
	OdGiViewportGeometry& ViewportGeometry = viewportDraw->geometry();
	const OdGiViewport& Viewport = viewportDraw->viewport();
	const auto WorldToEyeTransform {Viewport.getWorldToEyeTransform()};
	Viewport.getNumPixelsInUnitSquare(Viewport.getCameraTarget(), (OdGePoint2d&) Points[0]);
	const double pix = 1. / Points[0].x;
	const double s = snapPtSize * pix;

	OdGiSubEntityTraits& SubEntityTraits = viewportDraw->subEntityTraits();
	OdGiDrawFlagsHelper DrawFlagsHelper(SubEntityTraits, OdGiSubEntityTraits::kDrawNoPlotstyle);
	if ((m_SnapMode > 0) && ((unsigned long)m_SnapMode < 100)) {
		SubEntityTraits.setTrueColor(SnapTrueColor());

		SubEntityTraits.setFillType(kOdGiFillNever);
		SubEntityTraits.setSelectionMarker(kNullSubentIndex);
		Points[0] = WorldToEyeTransform * m_SnapPoint;
		Viewport.doPerspective(Points[0]);

		switch (m_SnapMode) {
			case OdDb::kOsModeEnd:
				Points[1].set(Points[0].x - s, Points[0].y + s, 0.0);
				Points[2].set(Points[0].x - s, Points[0].y - s, 0.0);
				Points[3].set(Points[0].x + s, Points[0].y - s, 0.0);
				Points[0].set(Points[0].x + s, Points[0].y + s, 0.0);
				ViewportGeometry.polygonEye(4, Points);
				Points[1].set(Points[1].x - pix, Points[1].y + pix, 0.0);
				Points[2].set(Points[2].x - pix, Points[2].y - pix, 0.0);
				Points[3].set(Points[3].x + pix, Points[3].y - pix, 0.0);
				Points[0].set(Points[0].x + pix, Points[0].y + pix, 0.0);
				ViewportGeometry.polygonEye(4, Points);
				break;

			case OdDb::kOsModeMid:
				Points[1].set(Points[0].x - s * 1.2, Points[0].y - s * 0.6, 0.0);
				Points[2].set(Points[0].x, Points[0].y + s * 1.4, 0.0);
				Points[3].set(Points[0].x + s * 1.2, Points[0].y - s * 0.6, 0.0);
				ViewportGeometry.polygonEye(3, Points + 1);

				Points[1].set(Points[1].x - pix, Points[1].y - pix, 0.0);
				Points[2].set(Points[2].x, Points[2].y + pix, 0.0);
				Points[3].set(Points[3].x + pix, Points[3].y - pix, 0.0);
				ViewportGeometry.polygonEye(3, Points + 1);
				break;

			case OdDb::kOsModeCen:
			{
				OdGiModelTransformSaver mt(ViewportGeometry, Viewport.getEyeToWorldTransform());
				ViewportGeometry.circle(Points[0], s * 1.4, OdGeVector3d::kZAxis);
			}
			break;

			case OdDb::kOsModeQuad:
				Points[1].set(Points[0].x - s, Points[0].y, 0.0);
				Points[2].set(Points[0].x, Points[0].y + s, 0.0);
				Points[3].set(Points[0].x + s, Points[0].y, 0.0);
				Points[0].set(Points[0].x, Points[0].y - s, 0.0);
				ViewportGeometry.polygonEye(4, Points);
				Points[1].set(Points[1].x - pix, Points[1].y, 0.0);
				Points[2].set(Points[2].x, Points[2].y + pix, 0.0);
				Points[3].set(Points[3].x + pix, Points[3].y, 0.0);
				Points[0].set(Points[0].x, Points[0].y - pix, 0.0);
				ViewportGeometry.polygonEye(4, Points);
				break;

			case OdDb::kOsModePerp:
				Points[1].set(Points[0].x - s, Points[0].y + s + pix, 0.0);
				Points[2].set(Points[0].x - s, Points[0].y - s, 0.0);
				Points[3].set(Points[0].x + s + pix, Points[0].y - s, 0.0);
				ViewportGeometry.polylineEye(3, Points + 1);
				Points[1].set(Points[1].x - pix, Points[1].y, 0.0);
				Points[2].set(Points[2].x - pix, Points[2].y - pix, 0.0);
				Points[3].set(Points[3].x, Points[3].y - pix, 0.0);
				ViewportGeometry.polylineEye(3, Points + 1);

				Points[1].set(Points[0].x - s, Points[0].y, 0.0);
				Points[2].set(Points[0].x, Points[0].y, 0.0);
				Points[3].set(Points[0].x, Points[0].y - s, 0.0);
				ViewportGeometry.polylineEye(3, Points + 1);
				Points[1].set(Points[1].x - pix, Points[1].y + pix, 0.0);
				Points[2].set(Points[2].x + pix, Points[2].y + pix, 0.0);
				Points[3].set(Points[3].x + pix, Points[3].y, 0.0);
				ViewportGeometry.polylineEye(3, Points + 1);
				break;

			case OdDb::kOsModeTan:
			{
				OdGiModelTransformSaver mt(ViewportGeometry, Viewport.getEyeToWorldTransform());
				ViewportGeometry.circle(Points[0], s, OdGeVector3d::kZAxis);
			}
			Points[1].set(Points[0].x - s, Points[0].y + s, 0.0);
			Points[2].set(Points[0].x + s, Points[0].y + s, 0.0);
			ViewportGeometry.polylineEye(2, Points + 1);
			Points[1].set(Points[1].x, Points[1].y + pix, 0.0);
			Points[2].set(Points[2].x, Points[2].y + pix, 0.0);
			ViewportGeometry.polylineEye(2, Points + 1);
			break;

			case OdDb::kOsModeNear:
				Points[1].set(Points[0].x - s, Points[0].y + s, 0.0);
				Points[2].set(Points[0].x + s, Points[0].y - s, 0.0);
				Points[3].set(Points[0].x - s, Points[0].y - s, 0.0);
				Points[0].set(Points[0].x + s, Points[0].y + s, 0.0);
				ViewportGeometry.polygonEye(4, Points);
				Points[1].set(Points[1].x - pix, Points[1].y + pix, 0.0);
				Points[2].set(Points[2].x + pix, Points[2].y - pix, 0.0);
				Points[3].set(Points[3].x - pix, Points[3].y - pix, 0.0);
				Points[0].set(Points[0].x + pix, Points[0].y + pix, 0.0);
				ViewportGeometry.polygonEye(4, Points);
				break;

			default:
				Points[1].set(Points[0].x - s, Points[0].y + s, 0.0);
				Points[2].set(Points[0].x + s, Points[0].y - s, 0.0);
				ViewportGeometry.polygonEye(2, Points + 1);
				Points[1].set(Points[0].x - s, Points[0].y - s, 0.0);
				Points[2].set(Points[0].x + s, Points[0].y + s, 0.0);
				ViewportGeometry.polygonEye(2, Points + 1);
				Points[1].set(Points[0].x - s - pix, Points[0].y + s + pix, 0.0);
				Points[2].set(Points[0].x - s - pix, Points[0].y - s - pix, 0.0);
				Points[3].set(Points[0].x + s + pix, Points[0].y - s - pix, 0.0);
				Points[0].set(Points[0].x + s + pix, Points[0].y + s + pix, 0.0);
				ViewportGeometry.polygonEye(4, Points);
				break;
		}
	}
	OdGsMarker Marker = 0;
	if (m_Centers.size()) {
		viewportDraw->subEntityTraits().setTrueColor(CenterTrueColor());

		for (unsigned i = 0; i < m_Centers.size(); i++, Marker++) {
			SubEntityTraits.setSelectionMarker(Marker);

			const OdGePoint3d& Center = WorldToEyeTransform * m_Centers[i].m_Point;
			Points[0].set(Center.x, Center.y + s, 0.0);
			Points[1].set(Center.x, Center.y - s, 0.0);
			ViewportGeometry.polygonEye(2, Points);
			Points[0].set(Center.x + s, Center.y, 0.0);
			Points[1].set(Center.x - s, Center.y, 0.0);
			ViewportGeometry.polygonEye(2, Points);
		}
	}
}

void OdBaseSnapManager::InvalidateViewport(const OdBaseSnapManager::HistEntryArray & centers) const {
	OdGePoint3d Point;
	const auto WorldToDeviceTransform {m_View->worldToDeviceMatrix()};

	OdGsDCRect DcRectangle;

	for (unsigned i = 0; i < centers.size(); ++i) {
		Point = WorldToDeviceTransform * centers[i].m_Point;

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
	if (m_SnapMode) {
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
		switch(m_SnapMode)
		{
		case OdDb::kOsModeEnd:
		  DcRectangle.m_min.x -= snapPtSize;
		  DcRectangle.m_min.y -= snapPtSize;
		  DcRectangle.m_max.x += snapPtSize;
		  DcRectangle.m_max.y += snapPtSize;
		  break;

		case OdDb::kOsModeMid:
		  Points[1].set(Points[0].x - s * 1.2, Points[0].y - s * 0.6, 0.0);
		  Points[2].set(Points[0].x,           Points[0].y + s * 1.4, 0.0);
		  Points[3].set(Points[0].x + s * 1.2, Points[0].y - s * 0.6, 0.0);
		  geom.polygonEye(3, Points+1);

		  Points[1].set(Points[1].x - pix, Points[1].y - pix, 0.0);
		  Points[2].set(Points[2].x,       Points[2].y + pix, 0.0);
		  Points[3].set(Points[3].x + pix, Points[3].y - pix, 0.0);
		  geom.polygonEye(3, Points+1);
		  break;

		case OdDb::kOsModeCen:
		  geom.pushModelTransform(vp.getEyeToWorldTransform());
		  geom.circle(Points[0], s * 1.4, OdGeVector3d::kZAxis);
		  geom.popModelTransform();
		  break;

		case OdDb::kOsModeQuad:
		  Points[1].set(Points[0].x - s,  Points[0].y,     0.0);
		  Points[2].set(Points[0].x,      Points[0].y + s, 0.0);
		  Points[3].set(Points[0].x + s,  Points[0].y,     0.0);
		  Points[0].set(Points[0].x,      Points[0].y - s, 0.0);
		  geom.polygonEye(4, Points);
		  Points[1].set(Points[1].x - pix,  Points[1].y,       0.0);
		  Points[2].set(Points[2].x,        Points[2].y + pix, 0.0);
		  Points[3].set(Points[3].x + pix,  Points[3].y,       0.0);
		  Points[0].set(Points[0].x,        Points[0].y - pix, 0.0);
		  geom.polygonEye(4, Points);
		  break;

		case OdDb::kOsModePerp:
		  Points[1].set(Points[0].x - s,       Points[0].y + s + pix, 0.0);
		  Points[2].set(Points[0].x - s,       Points[0].y - s, 0.0);
		  Points[3].set(Points[0].x + s + pix, Points[0].y - s, 0.0);
		  geom.polylineEye(3, Points+1);
		  Points[1].set(Points[1].x - pix, Points[1].y,       0.0);
		  Points[2].set(Points[2].x - pix, Points[2].y - pix, 0.0);
		  Points[3].set(Points[3].x,       Points[3].y - pix, 0.0);
		  geom.polylineEye(3, Points+1);

		  Points[1].set(Points[0].x - s,  Points[0].y, 0.0);
		  Points[2].set(Points[0].x,      Points[0].y, 0.0);
		  Points[3].set(Points[0].x,      Points[0].y - s, 0.0);
		  geom.polylineEye(3, Points+1);
		  Points[1].set(Points[1].x - pix, Points[1].y + pix, 0.0);
		  Points[2].set(Points[2].x + pix, Points[2].y + pix, 0.0);
		  Points[3].set(Points[3].x + pix, Points[3].y,       0.0);
		  geom.polylineEye(3, Points+1);
		  break;

		case OdDb::kOsModeTan:
		  geom.pushModelTransform(vp.getEyeToWorldTransform());
		  geom.circle(Points[0], s, OdGeVector3d::kZAxis);
		  geom.popModelTransform();
		  Points[1].set(Points[0].x - s, Points[0].y + s, 0.0);
		  Points[2].set(Points[0].x + s, Points[0].y + s, 0.0);
		  geom.polylineEye(2, Points+1);
		  Points[1].set(Points[1].x, Points[1].y + pix, 0.0);
		  Points[2].set(Points[2].x, Points[2].y + pix, 0.0);
		  geom.polylineEye(2, Points+1);
		  break;

		case OdDb::kOsModeNear:
		  Points[1].set(Points[0].x - s, Points[0].y + s, 0.0);
		  Points[2].set(Points[0].x + s, Points[0].y - s, 0.0);
		  Points[3].set(Points[0].x - s, Points[0].y - s, 0.0);
		  Points[0].set(Points[0].x + s, Points[0].y + s, 0.0);
		  geom.polygonEye(4, Points);
		  Points[1].set(Points[1].x - pix, Points[1].y + pix, 0.0);
		  Points[2].set(Points[2].x + pix, Points[2].y - pix, 0.0);
		  Points[3].set(Points[3].x - pix, Points[3].y - pix, 0.0);
		  Points[0].set(Points[0].x + pix, Points[0].y + pix, 0.0);
		  geom.polygonEye(4, Points);
		  break;

		default:
		  Points[1].set(Points[0].x - s, Points[0].y + s, 0.0);
		  Points[2].set(Points[0].x + s, Points[0].y - s, 0.0);
		  geom.polygonEye(2, Points + 1);
		  Points[1].set(Points[0].x - s, Points[0].y - s, 0.0);
		  Points[2].set(Points[0].x + s, Points[0].y + s, 0.0);
		  geom.polygonEye(2, Points + 1);
		  Points[1].set(Points[0].x - s - pix, Points[0].y + s + pix, 0.0);
		  Points[2].set(Points[0].x - s - pix, Points[0].y - s - pix, 0.0);
		  Points[3].set(Points[0].x + s + pix, Points[0].y - s - pix, 0.0);
		  Points[0].set(Points[0].x + s + pix, Points[0].y + s + pix, 0.0);
		  geom.polygonEye(4, Points);
		  break;
		}
		*/
		m_View->invalidate(DcRectangle);
	}
}

unsigned long OdBaseSnapManager::subSetAttributes(OdGiDrawableTraits* drawableTraits) const {
	return kDrawableNone;
}

bool OdBaseSnapManager::subWorldDraw(OdGiWorldDraw* worldDraw) const {
	return false;
}

bool OdBaseSnapManager::Snap(OdGsView* view, OdGePoint3d& point, const OdGePoint3d* lastPoint) {
	OdEdPointTrackerWithSnapInfo* TrackerSnapInfo = dynamic_cast<OdEdPointTrackerWithSnapInfo*>(m_InputTracker);

	if (TrackerSnapInfo) { TrackerSnapInfo->m_SnapContext.mValid = false; }

	m_Redraw = false;
	m_SnapPoints.clear();
	m_View = view;
	m_PickPoint = &point;
	m_LastPoint = lastPoint;

	HistEntryArray PreviousCenters(m_Centers);
	const OdGePoint3d PreviousPoint(m_SnapPoint);
	const OdDb::OsnapMode PreviousMode(m_SnapMode);

	if (m_SnapMode == 0 || !Checkpoint(m_SnapMode, m_SnapPoint)) {
		m_NearDist = std::numeric_limits<double>::max();
		m_SnapPoint = OdGePoint3d(1e100, 1e100, 1e100);
		m_SnapMode = OdDb::OsnapMode(100);
	}

	const auto pt {(view->worldToDeviceMatrix() * point).convert2d()};
	OdGsDCPoint DcPoints[2];
	const auto Aperture {GetAperture(dynamic_cast<OdDbDatabase*>(view->userGiContext()->database()))};

	DcPoints[0].x = OdRoundToLong(pt.x) - Aperture;
	DcPoints[1].x = DcPoints[0].x + Aperture * 2;
	DcPoints[0].y = OdRoundToLong(pt.y) - Aperture;
	DcPoints[1].y = DcPoints[0].y + Aperture * 2;
	m_HitRadius = static_cast<double>(Aperture);
	m_WorldToDevice = view->worldToDeviceMatrix().getCsXAxis().length();

	auto pViewImpl {dynamic_cast<OdGsViewImpl*>(view)};
	
	if (pViewImpl) { pViewImpl->setSnapping(true); }

	m_SelectedEntityData.clear();

	view->select(DcPoints, 2, this);

	if (m_SelectedEntityData.size()) { // dna: only one can be selected currently
		CheckSnapPoints(m_SelectedEntityData[0], pViewImpl->worldToEyeMatrix());
	}

	if (pViewImpl) { pViewImpl->setSnapping(false); }

	if (m_SnapMode > 0 && (unsigned long)m_SnapMode < 100) {
		point = m_SnapPoint;
	} else {
		if (PreviousMode > 0 && (unsigned long)PreviousMode < 100) { InvalidateViewport(PreviousPoint); }

		m_SnapMode = OdDb::OsnapMode(0);
	}
	bool bRes = true;
	if (m_SnapPoint == PreviousPoint) {
		bRes = false;
	} else {
		if (PreviousPoint.x < 1e100) { InvalidateViewport(PreviousPoint); }
		
		if (m_SnapPoint.x < 1e100) { InvalidateViewport(m_SnapPoint); }
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
			m_SnapPoint = point;
			m_SnapMode = objectSnapMode;

			if (TrackerSnapInfo) {
				TrackerSnapInfo->m_SnapContext.mPoint = point;
				TrackerSnapInfo->m_SnapContext.mMode = objectSnapMode;
				TrackerSnapInfo->m_SnapContext.mValid = true;
			}
			return true;
		} else if (dist == m_NearDist)
			return true;
	}
	return false;
}

const int nMaxHist = 7;

bool OdBaseSnapManager::AppendToQueue(OdBaseSnapManager::HistEntryArray& histEntries, const HistEntry& histEntry) {
	if (!histEntries.contains(histEntry)) {
		if (histEntries.size() > nMaxHist) {
			histEntries.erase(histEntries.begin());
		}
		histEntries.append(histEntry);
		return true;
	}
	return false;
}

void OdBaseSnapManager::CheckSnapPoints(const SelectedEntityData& selectedEntityData, const OdGeMatrix3d& worldToEyeTransform) {
	const auto ModelToWorldTransform {selectedEntityData.ModelToWorldTransform};
	const bool InsertionMatrix = (ModelToWorldTransform != OdGeMatrix3d::kIdentity);
	const auto ModelPickPoint {ModelToWorldTransform * *m_PickPoint};
	OdGePoint3d ModelLastPoint;
	unsigned nSnapModes = SnapModes();
	
	if (m_LastPoint) {
		SETBIT(nSnapModes, ToSnapModes(OdDb::kOsModePerp) | ToSnapModes(OdDb::kOsModeTan) | ToSnapModes(OdDb::kOsModePerp), 0);
		ModelLastPoint = ModelToWorldTransform * *m_LastPoint;
	}
	OdEdPointTrackerWithSnapInfo* PointTrackerWithSnapInfo = dynamic_cast<OdEdPointTrackerWithSnapInfo*>(m_InputTracker);

	OdDbEntityPtr Entity {selectedEntityData.SubentId.m_Path.first().safeOpenObject()};
	const auto Marker {selectedEntityData.SubentId.m_Marker};
	
	if (!PointTrackerWithSnapInfo) {
		for (OdDb::OsnapMode ObjectSnapMode = OdDb::kOsModeEnd; ObjectSnapMode <= OdDb::kOsModeNear; ObjectSnapMode = OdDb::OsnapMode(ObjectSnapMode + 1)) {
	
			if (nSnapModes & ToSnapModes(ObjectSnapMode)) // so not all types are tested
			{
				OdResult Result;
				if (InsertionMatrix) {
					Result = Entity->getOsnapPoints(ObjectSnapMode, Marker, ModelPickPoint, ModelLastPoint, worldToEyeTransform, m_SnapPoints, ModelToWorldTransform);
				} else {
					Result = Entity->getOsnapPoints(ObjectSnapMode, Marker, ModelPickPoint, ModelLastPoint, worldToEyeTransform, m_SnapPoints);
				}
				if (Result == eOk) {

					for (unsigned i = 0; i < m_SnapPoints.size(); ++i) {
						OdGePoint3d& point = m_SnapPoints[i];
						point.transformBy(ModelToWorldTransform);
						Checkpoint(ObjectSnapMode, point);
						switch (ObjectSnapMode) {
							case OdDb::kOsModeCen:
								AppendToQueue(m_Centers, HistEntry(selectedEntityData.SubentId, point));
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
	} else {
		if (!PointTrackerWithSnapInfo->IsTargetEntity(Entity)) { return; }

		OdSaveState<double> ssHitRadius(m_HitRadius, 1500.);

		OdArray<OdDb::OsnapMode> snapModes;

		PointTrackerWithSnapInfo->GetSnapModes(Entity, snapModes);

		OdArray<OdDb::OsnapMode>::iterator it;

		for (it = snapModes.begin(); it != snapModes.end(); it++) {

			if (Entity->getOsnapPoints(*it, Marker, ModelPickPoint, ModelLastPoint, worldToEyeTransform, m_SnapPoints) == eOk) {
				PointTrackerWithSnapInfo->m_SnapContext.mEntityObjectId = Entity->objectId();
				PointTrackerWithSnapInfo->m_SnapContext.mMarker = Marker;

				for (unsigned i = 0; i < m_SnapPoints.size(); ++i) {
					OdGePoint3d& point = m_SnapPoints[i];
					point.transformBy(ModelToWorldTransform);
					Checkpoint(*it, point);
				}

			}
		}
	}
}

unsigned long OdBaseSnapManager::selected(const OdGiPathNode & pathNode, const OdGiViewport & viewInfo) {
	if (pathNode.transientDrawable() == this) {
		const auto Marker {pathNode.selectionMarker()};

		if (Marker > -1) {
			if ((SnapModes() & ToSnapModes(OdDb::kOsModeCen)) && (OdGsMarker) m_Centers.size() > Marker) {
				Checkpoint(OdDb::kOsModeCen, m_Centers[Marker].m_Point);
			}
		}
		return unsigned long(kContinue);
	}

	auto Entity {OdDbEntity::cast(OdDbObjectId(pathNode.persistentDrawableId()).openObject())};

	if (Entity.isNull()) { return unsigned long(kSkipDrawable); }

	m_SelectedEntityData.append()->set(pathNode);

	return unsigned long(kSkipDrawable);
}

void OdBaseSnapManager::RecalculateEntityCenters() {
	for (int i = m_Centers.size() - 1; i >= 0; --i) {
		SubentId SubentId = m_Centers[i].m_SubentId;

		if (SubentId.m_Path.size() <= 0) continue;

		auto Entity {OdDbEntity::cast(SubentId.m_Path[0].openObject())};

		if (Entity.isNull()) {
			m_Centers.erase(m_Centers.begin() + i);
			continue;
		}

		OdGePoint3dArray SnapPoints;
		Entity->getOsnapPoints(OdDb::kOsModeCen, OdGsMarker(), OdGePoint3d(), OdGePoint3d(), OdGeMatrix3d(), SnapPoints);

		if (SnapPoints.size() > 0) {
			m_Centers[i].m_Point = SnapPoints[0]; // recalculation center
		}

	}
}

bool OdBaseSnapManager::SetEntityCenters(OdRxObject* rxObject) {
	m_Centers.clear();

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
	OdGiDrawableDesc* dd0 = nullptr;
	OdGiLocalDrawableDesc dd(dd0); // need for build OdGiPathNode

	for (OdDbObjectIteratorPtr pIter = blockTableRecord->newIterator(); !pIter->done() && m_Centers.size() < nMaxHist; pIter->step()) {
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

		for (unsigned long i = 0; i < snapPoints.size() && m_Centers.size() < nMaxHist; i++) {
			m_Centers.append(HistEntry(dd, snapPoints[i].transformBy(matrix)));
		}
	}
}

void OdBaseSnapManager::Track(OdEdInputTracker* inputTracker) {
	m_InputTracker = inputTracker;
}
