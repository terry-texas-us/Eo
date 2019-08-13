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

static OdDbSpatialFilterPtr GetBlockReferenceSpatialFilter(OdDbBlockReference* blockReference, const OdDb::OpenMode openMode) {
	auto Dictionary {OdDbDictionary::cast(blockReference->extensionDictionary().openObject())};
	if (Dictionary.isNull()) {
		return OdDbSpatialFilterPtr();
	}
	auto AcadFilterDictionary {OdDbDictionary::cast(Dictionary->getAt(L"ACAD_FILTER", OdDb::kForRead))};
	if (AcadFilterDictionary.isNull()) {
		return OdDbSpatialFilterPtr();
	}
	return OdDbSpatialFilter::cast(AcadFilterDictionary->getAt(L"SPATIAL", openMode));
}

void OdDbBlockGripOpStatus(OdDbGripData* gripData, OdDbStub* /*stub*/, const OdDbGripOperations::GripStatus status) {
	if (gripData->appDataOdRxClass() == OdDbBlockGripAppData::desc() && (status == OdDbGripOperations::kGripEnd || status == OdDbGripOperations::kGripAbort)) {
		((OdDbBlockGripAppData*)gripData->appData())->release();
		gripData->setAppData(nullptr);
	}
}

static double GetGripSize(OdGiViewportDraw* viewportDraw, const OdGePoint3d& eyePoint, const int gripSize) {
	OdGePoint2d PixelDensity;
	auto wcsPt {eyePoint};
	wcsPt.transformBy(viewportDraw->viewport().getEyeToWorldTransform());
	viewportDraw->viewport().getNumPixelsInUnitSquare(wcsPt, PixelDensity);
	OdGeVector3d v(gripSize / PixelDensity.x, 0.0, 0.0);
	v.transformBy(viewportDraw->viewport().getWorldToEyeTransform());
	return v.length();
}

static void DrawFlipArrow(OdGiViewportDraw* viewportDraw, const OdGePoint3d& point, const int gripSize, const OdGeVector3d& orient) {
	auto v {orient};
	v.normalize();
	const auto GripSize {GetGripSize(viewportDraw, point, gripSize)};
	v *= GripSize;
	auto n {v};
	n.rotateBy(OdaPI2, OdGeVector3d::kZAxis);
	n *= 0.75;
	OdGePoint3d pp[7];
	pp[0] = point + n * 0.5;
	pp[1] = point + n;
	pp[3] = point - n;
	pp[4] = point - n * 0.5;
	pp[2] = point + v;
	pp[5] = point - v - n * 0.5;
	pp[6] = point - v + n * 0.5;
	viewportDraw->geometry().polygonEye(7, pp);
}

static void OdDbBlockGripViewportDraw(OdDbGripData* gripData, OdGiViewportDraw* viewportDraw, OdDbStub* /*entityId*/, const OdDbGripOperations::DrawType type, OdGePoint3d* imageGripPoint, const int gripSize) {
	ODA_ASSERT(gripData->appDataOdRxClass() == OdDbBlockGripAppData::desc());
	auto Point {imageGripPoint == nullptr ? gripData->gripPoint() : *imageGripPoint};
	Point.transformBy(viewportDraw->viewport().getWorldToEyeTransform());
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
	DrawFlipArrow(viewportDraw, Point, gripSize, ((OdDbBlockGripAppData*)gripData->appData())->m_vClipInvertOrientation);
}

OdResult OdDbBlockHotGrip(OdDbGripData* gripData, OdDbStub* entityId, const int status) {
	if ((status & OdDbGripOperations::kSharedGrip) != 0) {
		return eOk;
	}
	auto AppData {(OdDbBlockGripAppData*) gripData->appData()};
	if (!AppData->m_bClipInvertGrip) {
		return eOk;
	}
	AppData->m_vClipInvertOrientation.negate();
	OdDbBlockReferencePtr BlockReference {OdDbObjectId(entityId).safeOpenObject(OdDb::kForWrite)};
	auto SpatialFilter {GetBlockReferenceSpatialFilter(BlockReference, OdDb::kForWrite)};
	SpatialFilter->setFilterInverted(!SpatialFilter->isFilterInverted());
	BlockReference->assertWriteEnabled();
	return eOk;
}

OdResult OdDbBlockReferenceGripPointsPE::getGripPoints(const OdDbEntity* entity, OdDbGripDataPtrArray& grips, const double /*currentViewUnitSize*/, const int /*gripSize*/, const OdGeVector3d& currentViewDirection, const int /*bitFlags*/) const {
	OdDbBlockReferencePtr BlockReference(entity);
	// Origin
	{
		OdDbGripDataPtr OriginGripData(new OdDbGripData());
		OriginGripData->setGripPoint(BlockReference->position());
		OriginGripData->setGripOpStatFunc(OdDbBlockGripOpStatus);
		OriginGripData->setAppDataOdRxClass(OdDbBlockGripAppData::desc());
		auto OriginAppData {OdRxObjectImpl<OdDbBlockGripAppData>::createObject()};
		OriginGripData->setAppData(OriginAppData.detach());
		grips.append(OriginGripData);
	}
	// XCLIP boundary
	{
		auto SpatialFilter {GetBlockReferenceSpatialFilter(BlockReference, OdDb::kForRead)};
		OdGePoint2dArray ClipPoints;
		if (!SpatialFilter.isNull() && SpatialFilter->isEnabled()) {
			SpatialFilter->boundary(ClipPoints);
			OdGeMatrix3d ClipSpaceTransform {SpatialFilter->getClipSpaceToWCSMatrix(ClipSpaceTransform)};
			ClipSpaceTransform.invert();
			OdGeMatrix3d ClipInverseBlockTransform {SpatialFilter->getOriginalInverseBlockXform(ClipInverseBlockTransform)};
			const auto BlockTransform {BlockReference->blockTransform()};
			const auto FullTransform {BlockTransform * ClipInverseBlockTransform * ClipSpaceTransform};
			for (unsigned ClipPointIndex = 0; ClipPointIndex < ClipPoints.size(); ClipPointIndex++) {
				OdGePoint3d ClipPoint(ClipPoints[ClipPointIndex].x, ClipPoints[ClipPointIndex].y, 0.0);
				ClipPoint.transformBy(FullTransform);
				OdDbGripDataPtr GripData(new OdDbGripData());
				GripData->setGripPoint(ClipPoint);
				GripData->setGripOpStatFunc(OdDbBlockGripOpStatus);
				GripData->setAppDataOdRxClass(OdDbBlockGripAppData::desc());
				auto appData {OdRxObjectImpl<OdDbBlockGripAppData>::createObject()};
				appData->m_nClipGripGripIndex = ClipPointIndex;
				GripData->setAppData(appData.detach());
				grips.append(GripData);
			}
			// XCLIP inversion
			if (ClipPoints.size() > 1) {
				OdGePoint3d p1(ClipPoints[0].x, ClipPoints[0].y, 0.0);
				OdGePoint3d p2(ClipPoints[1].x, ClipPoints[1].y, 0.0);
				p1.transformBy(FullTransform);
				p2.transformBy(FullTransform);
				auto v {p2 - p1};
				const auto MidPoint {p1 + v / 2.0};
				OdDbGripDataPtr GripData(new OdDbGripData());
				GripData->setGripPoint(MidPoint);
				GripData->setGripOpStatFunc(OdDbBlockGripOpStatus);
				GripData->setViewportDraw(OdDbBlockGripViewportDraw);
				GripData->setHotGripFunc(OdDbBlockHotGrip);
				GripData->setAppDataOdRxClass(OdDbBlockGripAppData::desc());
				auto AppData {OdRxObjectImpl<OdDbBlockGripAppData>::createObject()};
				AppData->m_bClipInvertGrip = true;
				auto Flag {OdGe::kOk};
				v.normalize(OdGeContext::gTol, Flag);
				if (Flag == OdGe::kOk) {
					v.rotateBy(OdaPI2, currentViewDirection);
					if (SpatialFilter->isFilterInverted()) {
						v.negate();
					}
					AppData->m_vClipInvertOrientation = v;
				}
				GripData->setAppData(AppData.detach());
				grips.append(GripData);
			}
		} 
		// Attributes
		auto AttributeIndex {0};
		for (auto Iterator = BlockReference->attributeIterator(); !Iterator->done(); Iterator->step()) {
			OdDbAttributePtr Attribute(Iterator->entity());
			if (Attribute->lockPositionInBlock()) {
				continue;
			}
			OdDbGripDataPtr GripData(new OdDbGripData());
			if (Attribute->isMTextAttribute()) {
				GripData->setGripPoint(Attribute->getMTextAttribute()->location());
			} else {
				if (Attribute->horizontalMode() == OdDb::kTextLeft && Attribute->verticalMode() == OdDb::kTextBase) {
					GripData->setGripPoint(Attribute->position());
				} else {
					GripData->setGripPoint(Attribute->alignmentPoint());
				}
			}
			GripData->setGripOpStatFunc(OdDbBlockGripOpStatus);
			GripData->setAppDataOdRxClass(OdDbBlockGripAppData::desc());
			auto AppData {OdRxObjectImpl<OdDbBlockGripAppData>::createObject()};
			AppData->m_nAttributeIndex = AttributeIndex++;
			GripData->setAppData(AppData.detach());
			grips.append(GripData);
		}
	}
	return eOk;
}

OdResult OdDbBlockReferenceGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdDbVoidPtrArray& grips, const OdGeVector3d& offset, int /*bitFlags*/) {
	OdDbBlockReferencePtr BlockReference(entity);
	const OdGeMatrix3d Transform(offset); // XCLIP boundary
	auto SpatialFilter {GetBlockReferenceSpatialFilter(BlockReference, OdDb::kForWrite)};
	OdGePoint2dArray ClipPoints;
	OdGeVector3d cbOffset;
	auto BoundaryChanged {false};
	if (!SpatialFilter.isNull() && SpatialFilter->isEnabled()) {
		SpatialFilter->boundary(ClipPoints);
		OdGeMatrix3d ClipSpaceTransform {SpatialFilter->getClipSpaceToWCSMatrix(ClipSpaceTransform)};
		ClipSpaceTransform.invert();
		OdGeMatrix3d ClipInverseBlockTransform = SpatialFilter->getOriginalInverseBlockXform(ClipInverseBlockTransform);
		const auto BlockTransform {BlockReference->blockTransform()};
		const auto FullTransform {(BlockTransform * ClipInverseBlockTransform * ClipSpaceTransform).invert()};
		cbOffset = offset;
		cbOffset.transformBy(FullTransform);
	}
	for (auto Grip : grips) {
		if (Grip == nullptr || ((OdRxObject*)Grip)->isA() != OdDbBlockGripAppData::desc()) { // not our grip (maybe overruled)
			continue;
		}
		const auto AppData {(OdDbBlockGripAppData*)Grip};
		if (AppData->m_nClipGripGripIndex >= 0) { // XCLIP boundary
			ClipPoints[AppData->m_nClipGripGripIndex] += OdGeVector2d(cbOffset.x, cbOffset.y);
			BoundaryChanged = true;
		} else if (AppData->m_nAttributeIndex >= 0) {
			auto n {0};
			for (auto Iterator = BlockReference->attributeIterator(); !Iterator->done(); Iterator->step()) {
				OdDbAttributePtr Attribute(Iterator->entity(OdDb::kForWrite));
				if (Attribute->lockPositionInBlock()) {
					continue;
				}
				if (n == AppData->m_nAttributeIndex) {
					Attribute->transformBy(Transform);
				}
				++n;
			}
		} else if (!AppData->m_bClipInvertGrip) {
			BlockReference->setPosition(BlockReference->position() + offset);
			for (auto Iterator = BlockReference->attributeIterator(); !Iterator->done(); Iterator->step()) {
				OdDbAttributePtr Attribute(Iterator->entity(OdDb::kForWrite));
				Attribute->transformBy(Transform);
			}
		}
	}
	if (BoundaryChanged) {
		OdGiClipBoundary ClipBoundary;
		bool Enabled;
		SpatialFilter->getDefinition(ClipBoundary, Enabled);
		const auto Inverted {SpatialFilter->isFilterInverted()};
		ClipBoundary.m_Points = ClipPoints;
		SpatialFilter->setDefinition(ClipBoundary, Enabled);
		if (Inverted) {
			SpatialFilter->setFilterInverted(Inverted);
		}
		BlockReference->assertWriteEnabled();
		// Force modification in case if boundary was changed
	}
	return eOk;
}

OdResult OdDbBlockReferenceGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDbBlockReferencePtr BlockReference(entity);
	stretchPoints.append(BlockReference->position());
	// basic block reference has only one stretch point
	return eOk;
}

OdResult OdDbBlockReferenceGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& /*indices*/, const OdGeVector3d& offset) {
	OdDbBlockReferencePtr BlockReference(entity);
	BlockReference->transformBy(OdGeMatrix3d::translation(offset));
	return eOk;
}

// Should return:
// Osnap points for block contents transformed by own transformation.
// Plus for INS - own position and INS should be called for attributes
//
// Note selectionMarker also needs to be taken care of
OdResult OdDbBlockReferenceGripPointsPE::getOsnapPoints(const OdDbEntity* /*entity*/, OdDb::OsnapMode /*objectSnapMode*/, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& /*snapPoints*/) const {
	return eNotImplemented;
}
