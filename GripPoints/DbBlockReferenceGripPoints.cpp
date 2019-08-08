#include <OdaCommon.h>
#include "DbEntityGripPoints.h"
#include <DbBlockReference.h>
#include <DbAttribute.h>
#include <DbDictionary.h>
#include <Gi/GiClipBoundary.h>
#include <DbSpatialFilter.h>
#include "DbBlockReferenceGripPoints.h"
#include <Gi/GiViewportDraw.h>
ODRX_CONS_DEFINE_MEMBERS(OdDbBlockGripAppData, OdRxObject, RXIMPL_CONSTR);

static OdDbSpatialFilterPtr getBlockReferenceSpatialFilter(OdDbBlockReference* pBR, OdDb::OpenMode openMode) {
	auto pDict {OdDbDictionary::cast(pBR->extensionDictionary().openObject())};
	if (pDict.isNull()) {
		return OdDbSpatialFilterPtr();
	}
	auto pFDict {OdDbDictionary::cast(pDict->getAt(L"ACAD_FILTER", OdDb::kForRead))};
	if (pFDict.isNull()) {
		return OdDbSpatialFilterPtr();
	}
	return OdDbSpatialFilter::cast(pFDict->getAt(L"SPATIAL", openMode));
}

void OdDbBlockGripOpStatus(OdDbGripData* gripData, OdDbStub* /*stub*/, OdDbGripOperations::GripStatus status) {
	if (gripData->appDataOdRxClass() == OdDbBlockGripAppData::desc() && (status == OdDbGripOperations::kGripEnd || status == OdDbGripOperations::kGripAbort)) {
		((OdDbBlockGripAppData*)gripData->appData())->release();
		gripData->setAppData(nullptr);
	}
}

static double getGripSize(OdGiViewportDraw* pWd, const OdGePoint3d& eyePt, int gripSize) {
	OdGePoint2d ptDim;
	auto wcsPt {eyePt};
	wcsPt.transformBy(pWd->viewport().getEyeToWorldTransform());
	pWd->viewport().getNumPixelsInUnitSquare(wcsPt, ptDim);
	OdGeVector3d v(gripSize / ptDim.x, 0, 0);
	v.transformBy(pWd->viewport().getWorldToEyeTransform());
	return v.length();
}

static void drawFlipArrow(OdGiViewportDraw* pWd, const OdGePoint3d& p, int gripSize, const OdGeVector3d& orient) {
	auto v {orient};
	v.normalize();
	const auto GripSize {getGripSize(pWd, p, gripSize)};
	v *= GripSize;
	auto n {v};
	n.rotateBy(OdaPI2, OdGeVector3d::kZAxis);
	n *= 0.75;
	OdGePoint3d pp[7];
	pp[0] = p + n * 0.5;
	pp[1] = p + n;
	pp[3] = p - n;
	pp[4] = p - n * 0.5;
	pp[2] = p + v;
	pp[5] = p - v - n * 0.5;
	pp[6] = p - v + n * 0.5;
	pWd->geometry().polygonEye(7, pp);
}

static void OdDbBlockGripViewportDraw(OdDbGripData* gripData, OdGiViewportDraw* viewportDraw, OdDbStub* /*entId*/, OdDbGripOperations::DrawType type, OdGePoint3d* imageGripPoint, int gripSize) {
	ODA_ASSERT(gripData->appDataOdRxClass() == OdDbBlockGripAppData::desc());
	auto p {imageGripPoint == nullptr ? gripData->gripPoint() : *imageGripPoint};
	p.transformBy(viewportDraw->viewport().getWorldToEyeTransform());
	switch (type) {
		case OdDbGripOperations::kWarmGrip:
			viewportDraw->subEntityTraits().setColor(OdCmEntityColor::kACICyan);
			break;
		case OdDbGripOperations::kHoverGrip:
			viewportDraw->subEntityTraits().setColor(OdCmEntityColor::kACIRed);
			break;
		case OdDbGripOperations::kHotGrip:
			viewportDraw->subEntityTraits().setColor(OdCmEntityColor::kACIMagenta);
			break;
		case OdDbGripOperations::kDragImageGrip:
			viewportDraw->subEntityTraits().setColor(OdCmEntityColor::kACIBlue);
			break;
	}
	viewportDraw->subEntityTraits().setFillType(kOdGiFillAlways);
	drawFlipArrow(viewportDraw, p, gripSize, ((OdDbBlockGripAppData*)gripData->appData())->m_vClipInvertOrientation);
}

OdResult OdDbBlockHotGrip(OdDbGripData* gripData, OdDbStub* entId, int status) {
	if ((status & OdDbGripOperations::kSharedGrip) != 0) {
		return eOk;
	}
	auto appdata {(OdDbBlockGripAppData*) gripData->appData()};
	if (!appdata->m_bClipInvertGrip) {
		return eOk;
	}
	appdata->m_vClipInvertOrientation.negate();
	OdDbBlockReferencePtr pBtr = OdDbObjectId(entId).safeOpenObject(OdDb::kForWrite);
	auto pSpatialFilter {getBlockReferenceSpatialFilter(pBtr, OdDb::kForWrite)};
	pSpatialFilter->setFilterInverted(!pSpatialFilter->isFilterInverted());
	pBtr->assertWriteEnabled();
	return eOk;
}

OdResult OdDbBlockReferenceGripPointsPE::getGripPoints(const OdDbEntity* entity, OdDbGripDataPtrArray& grips, const double /*currentViewUnitSize*/, const int /*gripSize*/, const OdGeVector3d& currentViewDirection, const int /*bitFlags*/) const {
	OdDbBlockReferencePtr pBtr(entity);
	{
		// Origin
		OdDbGripDataPtr originGrip(new OdDbGripData());
		originGrip->setGripPoint(pBtr->position());
		originGrip->setGripOpStatFunc(OdDbBlockGripOpStatus);
		originGrip->setAppDataOdRxClass(OdDbBlockGripAppData::desc());
		auto originAppData {OdRxObjectImpl<OdDbBlockGripAppData>::createObject()};
		originGrip->setAppData(originAppData.detach());
		grips.append(originGrip);
	}
	{
		// XCLIP boundary
		auto pSpatialFilter {getBlockReferenceSpatialFilter(pBtr, OdDb::kForRead)};
		OdGePoint2dArray clipPoints;
		if (!pSpatialFilter.isNull() && pSpatialFilter->isEnabled()) {
			pSpatialFilter->boundary(clipPoints);
			OdGeMatrix3d xClipSpace = pSpatialFilter->getClipSpaceToWCSMatrix(xClipSpace);
			xClipSpace.invert();
			OdGeMatrix3d xClipInvBlock = pSpatialFilter->getOriginalInverseBlockXform(xClipInvBlock);
			const auto xBlockXForm {pBtr->blockTransform()};
			const OdGeMatrix3d xFullXForm = xBlockXForm * xClipInvBlock * xClipSpace;
			for (OdUInt32 nClipPoint = 0; nClipPoint < clipPoints.size(); nClipPoint++) {
				OdGePoint3d clipPoint(clipPoints[nClipPoint].x, clipPoints[nClipPoint].y, 0.0);
				clipPoint.transformBy(xFullXForm);
				OdDbGripDataPtr aGrip(new OdDbGripData());
				aGrip->setGripPoint(clipPoint);
				aGrip->setGripOpStatFunc(OdDbBlockGripOpStatus);
				aGrip->setAppDataOdRxClass(OdDbBlockGripAppData::desc());
				auto appData {OdRxObjectImpl<OdDbBlockGripAppData>::createObject()};
				appData->m_nClipGripGripIndex = nClipPoint;
				aGrip->setAppData(appData.detach());
				grips.append(aGrip);
			} // XCLIP inversion
			if (clipPoints.size() > 1) {
				OdGePoint3d p1(clipPoints[0].x, clipPoints[0].y, 0.0);
				OdGePoint3d p2(clipPoints[1].x, clipPoints[1].y, 0.0);
				p1.transformBy(xFullXForm);
				p2.transformBy(xFullXForm);
				auto v {p2 - p1};
				const auto MidPoint {p1 + v / 2};
				OdDbGripDataPtr aGrip(new OdDbGripData());
				aGrip->setGripPoint(MidPoint);
				aGrip->setGripOpStatFunc(OdDbBlockGripOpStatus);
				aGrip->setViewportDraw(OdDbBlockGripViewportDraw);
				aGrip->setHotGripFunc(OdDbBlockHotGrip);
				aGrip->setAppDataOdRxClass(OdDbBlockGripAppData::desc());
				auto appData {OdRxObjectImpl<OdDbBlockGripAppData>::createObject()};
				appData->m_bClipInvertGrip = true;
				auto flag {OdGe::kOk};
				v.normalize(OdGeContext::gTol, flag);
				if (flag == OdGe::kOk) {
					v.rotateBy(OdaPI2, currentViewDirection);
					if (pSpatialFilter->isFilterInverted()) {
						v.negate();
					}
					appData->m_vClipInvertOrientation = v;
				}
				aGrip->setAppData(appData.detach());
				grips.append(aGrip);
			}
		} // Attributes
		auto attributeIndex {0};
		for (auto i = pBtr->attributeIterator(); !i->done(); i->step()) {
			OdDbAttributePtr attr(i->entity());
			if (attr->lockPositionInBlock()) {
				continue;
			}
			OdDbGripDataPtr aGrip(new OdDbGripData());
			if (attr->isMTextAttribute()) {
				aGrip->setGripPoint(attr->getMTextAttribute()->location());
			} else {
				if (attr->horizontalMode() == OdDb::kTextLeft && attr->verticalMode() == OdDb::kTextBase) {
					aGrip->setGripPoint(attr->position());
				} else {
					aGrip->setGripPoint(attr->alignmentPoint());
				}
			}
			aGrip->setGripOpStatFunc(OdDbBlockGripOpStatus);
			aGrip->setAppDataOdRxClass(OdDbBlockGripAppData::desc());
			auto appData {OdRxObjectImpl<OdDbBlockGripAppData>::createObject()};
			appData->m_nAttributeIndex = attributeIndex++;
			aGrip->setAppData(appData.detach());
			grips.append(aGrip);
		}
	}
	return eOk;
}

OdResult OdDbBlockReferenceGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdDbVoidPtrArray& grips, const OdGeVector3d& offset, int /*bitFlags*/) {
	OdDbBlockReferencePtr pBtr(entity);
	const OdGeMatrix3d Transform(offset); // XCLIP boundary
	auto pSpatialFilter {getBlockReferenceSpatialFilter(pBtr, OdDb::kForWrite)};
	OdGePoint2dArray clipPoints;
	OdGeVector3d cbOffset;
	auto BoundaryChanged {false};
	if (!pSpatialFilter.isNull() && pSpatialFilter->isEnabled()) {
		pSpatialFilter->boundary(clipPoints);
		OdGeMatrix3d xClipSpace = pSpatialFilter->getClipSpaceToWCSMatrix(xClipSpace);
		xClipSpace.invert();
		OdGeMatrix3d xClipInvBlock = pSpatialFilter->getOriginalInverseBlockXform(xClipInvBlock);
		const auto xBlockXForm {pBtr->blockTransform()};
		const auto xFullXForm {(xBlockXForm * xClipInvBlock * xClipSpace).invert()};
		cbOffset = offset;
		cbOffset.transformBy(xFullXForm);
	}
	for (unsigned k = 0; k < grips.size(); ++k) {
		if (grips[k] == nullptr || ((OdRxObject*)grips[k])->isA() != OdDbBlockGripAppData::desc()) { // not our grip (maybe overruled)
			continue;
		}
		const auto pAppData {(OdDbBlockGripAppData*) grips[k]};
		if (pAppData->m_nClipGripGripIndex >= 0) {
			// XCLIP boundary
			clipPoints[pAppData->m_nClipGripGripIndex] += OdGeVector2d(cbOffset.x, cbOffset.y);
			BoundaryChanged = true;
		} else if (pAppData->m_nAttributeIndex >= 0) {
			auto n {0};
			for (auto i = pBtr->attributeIterator(); !i->done(); i->step()) {
				OdDbAttributePtr attr(i->entity(OdDb::kForWrite));
				if (attr->lockPositionInBlock()) {
					continue;
				}
				if (n == pAppData->m_nAttributeIndex) {
					// We need to care about annotation contexts and alignment
					// attr->setPosition(attr->position() + offset);
					attr->transformBy(Transform);
				}
				++n;
			}
		} else if (!pAppData->m_bClipInvertGrip) {
			pBtr->setPosition(pBtr->position() + offset);
			for (auto i = pBtr->attributeIterator(); !i->done(); i->step()) {
				OdDbAttributePtr attr(i->entity(OdDb::kForWrite));
				// We need to care about annotation contexts and alignment
				// attr->setPosition(attr->position() + offset);
				attr->transformBy(Transform);
			}
		}
	}
	if (BoundaryChanged) {
		OdGiClipBoundary clipBnd;
		bool Enabled;
		pSpatialFilter->getDefinition(clipBnd, Enabled);
		const auto Inverted {pSpatialFilter->isFilterInverted()};
		clipBnd.m_Points = clipPoints;
		pSpatialFilter->setDefinition(clipBnd, Enabled);
		if (Inverted) {
			pSpatialFilter->setFilterInverted(Inverted);
		}
		pBtr->assertWriteEnabled();
		// Force modification in case if boundary was changed
	}
	return eOk;
}

OdResult OdDbBlockReferenceGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDbBlockReferencePtr pBtr(entity);
	stretchPoints.append(pBtr->position());
	// basic block reference has only one stretch point
	return eOk;
}

OdResult OdDbBlockReferenceGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& /*indices*/, const OdGeVector3d& offset) {
	OdDbBlockReferencePtr pBtr(entity);
	pBtr->transformBy(OdGeMatrix3d::translation(offset));
	return eOk;
}

OdResult OdDbBlockReferenceGripPointsPE::getOsnapPoints(const OdDbEntity* /*entity*/, OdDb::OsnapMode /*objectSnapMode*/, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& /*snapPoints*/) const {
	// Should return:
	// Osnap points for block contents transformed by own transformation.
	// Plus for INS - own position and INS should be called for attributes
	//
	// Note selectionMarker also needs to be taken care of
	return eNotImplemented;
}
