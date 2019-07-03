// From Examples\Editor\ExGripManager.cpp (last compare 20.5)
#include "stdafx.h"
#include "OdaCommon.h"
#define STL_USING_MAP
#define STL_USING_ALGORITHM
#include "OdaSTL.h"
#include "UInt32Array.h"
#include "Ge/GePoint3d.h"
#include "Gi/GiDrawableImpl.h"
#include "Gi/GiWorldDraw.h"
#include "Gi/GiViewportDraw.h"
#include "DbHostAppServices.h"
#include "DbCommandContext.h"
#include "DbEntity.h"
#include "DbAbstractViewportData.h"
#include "RxVariantValue.h"
#include "ExGripManager.h"
#include "Gs/GsModel.h"

// Menu animation flags
#if !defined(ODA_UNIXOS)
#ifndef TPM_VERPOSANIMATION
static const unsigned TPM_VERPOSANIMATION = 0x1000L;
#endif
#ifndef TPM_NOANIMATION
static const unsigned TPM_NOANIMATION = 0x4000L;
#endif
#endif // ODA_UNIXOS
//
constexpr unsigned gc_GripManagerPageEachObject = 200;

namespace
{
	OdSelectionSetIteratorPtr searchObjectSSetIterator(OdSelectionSetPtr selectionSet, OdDbStub* id) {
		auto SelectionSetIterator {selectionSet->newIterator()};
		while (!SelectionSetIterator->done()) {
			if (SelectionSetIterator->id() == id) { return SelectionSetIterator; }
			SelectionSetIterator->next();
		}
		return OdSelectionSetIteratorPtr();
	}

	OdBaseGripManager::OdExGripDataSubent& getSubentGripData(OdBaseGripManager::OdExGripDataExt& ext, OdDbBaseFullSubentPath entityPath) {
		for (auto& GripData : ext.m_GripDataSubEntity) {
			if (GripData.subentPath == entityPath) { return GripData; }
		}
		ODA_FAIL();
		return ext.m_GripDataSubEntity.at(0);
	}
}

OdExGripDragPtr OdExGripDrag::CreateObject(OdDbStub* id, OdBaseGripManager* gripManager) {
	auto GripDrag {RXIMPL_CONSTR(OdExGripDrag)};
	GripDrag->m_SubentPath.objectIds().append(id);
	GripDrag->m_GripManager = gripManager;
	return GripDrag;
}

OdExGripDragPtr OdExGripDrag::CreateObject(OdDbBaseFullSubentPath entityPath, OdBaseGripManager* gripManager) {
	auto GripDrag {RXIMPL_CONSTR(OdExGripDrag)};
	GripDrag->m_SubentPath = entityPath;
	GripDrag->m_GripManager = gripManager;
	return GripDrag;
}

OdExGripDrag::OdExGripDrag() noexcept {
	//m_SubentPath = OdDbBaseFullSubentPath();
	m_Clone = nullptr;
	m_GripManager = nullptr;
}

OdDbStub* OdExGripDrag::EntityId() const {
	return m_SubentPath.objectIds().last();
}

bool OdExGripDrag::EntityPath(OdDbBaseFullSubentPath* subentPath) const {
	if (subentPath) { *subentPath = m_SubentPath; }
	return m_SubentPath.subentId() != OdDbSubentId();
}

bool OdExGripDrag::LocateActiveGrips(OdIntArray& indices) {
	const auto& GripData {EntityPath() ? getSubentGripData(m_GripManager->m_GripData[EntityId()], m_SubentPath).m_pSubData : m_GripManager->m_GripData[EntityId()].m_pDataArray};
	auto bExMethod {true};
	indices.clear();
	for (unsigned i = 0; i < GripData.size(); i++) {
		if (GripData[i]->GripData().isNull()) { bExMethod = false; }
		if (OdDbGripOperations::kDragImageGrip == GripData[i]->Status()) { indices.push_back(static_cast<int>(i)); }
	}
	ODA_ASSERT(GripData.size() == 0 || !indices.empty());
	return bExMethod;
}

void OdExGripDrag::CloneEntity() {
	m_Clone = m_GripManager ? m_GripManager->CloneEntity(EntityId()) : OdGiDrawablePtr();
}

void OdExGripDrag::CloneEntity(const OdGePoint3d& ptMoveAt) {
	CloneEntity();
	if (m_Clone.isNull()) { return; }
	OdIntArray aIndices;
	const auto bExMethod {LocateActiveGrips(aIndices)};
	const auto vOffset {ptMoveAt - m_GripManager->m_BasePoint};
	if (bExMethod) {
		OdDbGripDataPtrArray aCloneData;
		if (EntityPath()) {
			m_GripManager->GetGripPointsAtSubentPath(m_Clone, m_SubentPath, aCloneData, m_GripManager->ActiveViewUnitSize(), m_GripManager->m_GRIPSIZE, m_GripManager->ActiveViewDirection(), 0);
		} else {
			m_GripManager->GetGripPoints(m_Clone, aCloneData, m_GripManager->ActiveViewUnitSize(), m_GripManager->m_GRIPSIZE, m_GripManager->ActiveViewDirection(), 0);
		}
		OdDbVoidPtrArray aIds;
		for (auto& GripDataIndex : aIndices) {
			if (gsl::narrow_cast<unsigned>(GripDataIndex) < aCloneData.size()) {
				aIds.push_back(aCloneData[static_cast<unsigned>(GripDataIndex)]->appData());
			} else {
				ODA_ASSERT(0);
			}
		}
		for (auto& GripData : aCloneData) {
			if (GripData->gripOpStatFunc()) {
				GripData->gripOpStatFunc()(GripData, OdDbObjectId::kNull, OdDbGripOperations::kGripStart);
			}
		}
		if (EntityPath()) {
			OdDbBaseFullSubentPathArray SubentPaths;
			SubentPaths.append(m_SubentPath);
			m_GripManager->MoveGripPointsAtSubentPaths(m_Clone, SubentPaths, aIds, vOffset, 0);
			m_GripManager->SubentGripStatus(m_Clone, OdDb::kGripsToBeDeleted, m_SubentPath);
		} else {
			m_GripManager->MoveGripPointsAt(m_Clone, aIds, vOffset, 0);
			m_GripManager->GripStatus(m_Clone, OdDb::kGripsToBeDeleted);
		}
		for (auto& GripData : aCloneData) {
			if (GripData->gripOpStatFunc()) { GripData->gripOpStatFunc()(GripData, nullptr, OdDbGripOperations::kGripEnd); }
		}
	} else {
		m_GripManager->MoveGripPointsAt(m_Clone, aIndices, vOffset);
		m_GripManager->GripStatus(m_Clone, OdDb::kGripsToBeDeleted);
	}
	m_GripManager->OnModified(this);
}

void OdExGripDrag::MoveEntity(const OdGePoint3d& ptMoveAt) {
	OdIntArray aIndices;
	const auto bExMethod {LocateActiveGrips(aIndices)};
	const auto vOffset {ptMoveAt - m_GripManager->m_BasePoint};
	auto Entity {m_GripManager->OpenObject(EntityId(), OdDb::kForWrite)};
	ODA_ASSERT(Entity.get());
	const auto& rData {EntityPath() ? getSubentGripData(m_GripManager->m_GripData[EntityId()], m_SubentPath).m_pSubData : m_GripManager->m_GripData[EntityId()].m_pDataArray};
	if (bExMethod) {
		OdDbVoidPtrArray aIds;
		const unsigned long iSize = aIndices.size();
		for (unsigned i = 0; i < iSize; i++) {
			if (aIndices[i] < gsl::narrow_cast<int>(rData.size())) {
				aIds.push_back(rData[static_cast<unsigned>(aIndices[i])]->GripData()->appData());
			} else {
				ODA_ASSERT(0);
			}
		}
		if (EntityPath()) {
			OdDbBaseFullSubentPathArray aPaths;
			aPaths.append(m_SubentPath);
			m_GripManager->MoveGripPointsAtSubentPaths(Entity, aPaths, aIds, vOffset, 0);
		} else {
			m_GripManager->MoveGripPointsAt(Entity, aIds, vOffset, 0);
		}
	} else {
		//OdGePoint3dArray aPts;
		//unsigned long iSize = rData.size();
		//aPts.resize( iSize );
		//for (unsigned i = 0; i < iSize; i++ )
		//  aPts[ i ] = rData[ i ]->point();
		//iSize = aIndices.size();
		//for(unsigned i = 0; i < iSize; i++ )
		//{
		//  if ( aIndices[ i ] < (long)rData.size() ) 
		//  {
		//    aPts[ aIndices[ i ] ] += vOffset;
		//  }
		//  else
		//  {
		//    ODA_ASSERT( 0 );
		//  }
		//}
		m_GripManager->MoveGripPointsAt(Entity, aIndices, vOffset);
	}
}

void OdExGripDrag::NotifyDragStarted() {
	if (!m_GripManager) { return; }
	auto Entity {m_GripManager->OpenObject(EntityId())};
	if (Entity.get()) { m_GripManager->DragStatus(Entity, OdDb::kDragStart); }
	m_GripManager->DraggingStarted();
}

void OdExGripDrag::NotifyDragEnded() {
	if (!m_GripManager) { return; }
	auto Entity {m_GripManager->OpenObject(EntityId())};
	if (Entity.get()) { m_GripManager->DragStatus(Entity, OdDb::kDragEnd); }
	m_GripManager->DraggingStopped();
}

void OdExGripDrag::NotifyDragAborted() {
	if (!m_GripManager) { return; }
	auto Entity {m_GripManager->OpenObject(EntityId())};
	if (Entity.get()) { m_GripManager->DragStatus(Entity, OdDb::kDragAbort); }
	m_GripManager->DraggingStopped();
}

unsigned long OdExGripDrag::subSetAttributes(OdGiDrawableTraits* drawableTraits) const {
	if (m_Clone.isNull()) { return kDrawableIsInvisible; }
	const auto Result {m_Clone->setAttributes(drawableTraits)};
	auto EntityTraits {OdGiSubEntityTraits::cast(drawableTraits)};
	if (EntityTraits.get()) { EntityTraits->setFillType(kOdGiFillNever); }
	return Result;
}

bool OdExGripDrag::subWorldDraw(OdGiWorldDraw* worldDraw) const {
	if (m_Clone.isNull()) { return true; }
	return m_Clone->worldDraw(worldDraw);
}

void OdExGripDrag::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	if (m_Clone.get()) { m_Clone->viewportDraw(viewportDraw); }
}

OdExGripDataPtr OdExGripData::CreateObject(OdDbStub* id, OdDbGripDataPtr gripData, const OdGePoint3d& point, OdBaseGripManager* gripManager) {
	auto GripData {RXIMPL_CONSTR(OdExGripData)};
	GripData->m_SubentPath.objectIds().append(id);
	GripData->m_GripData = gripData;
	GripData->m_GripManager = gripManager;
	GripData->m_Point = point;
	return GripData;
}

OdExGripDataPtr OdExGripData::CreateObject(OdDbBaseFullSubentPath entityPath, OdDbGripDataPtr gripData, const OdGePoint3d& point, OdBaseGripManager* gripManager) {
	auto GripData {RXIMPL_CONSTR(OdExGripData)};
	GripData->m_SubentPath = entityPath;
	GripData->m_GripData = gripData;
	GripData->m_GripManager = gripManager;
	GripData->m_Point = point;
	return GripData;
}

OdExGripData::OdExGripData() noexcept {
	m_Status = OdDbGripOperations::kWarmGrip;
	m_Invisible = false;
	m_Shared = false;
	m_Point = OdGePoint3d::kOrigin;
	//m_SubentPath = OdDbBaseFullSubentPath();
	//m_GripData = 0;
	m_GripManager = nullptr;
}

OdExGripData::~OdExGripData() {
	if (m_GripData.get() && m_GripData->alternateBasePoint()) {
		delete m_GripData->alternateBasePoint();
		m_GripData->setAlternateBasePoint(nullptr);
	}
}

bool OdExGripData::ComputeDragPoint(OdGePoint3d& computedPoint) const {
	auto BasePoint {Point()};
	if (GripData().get() && GripData()->alternateBasePoint()) {
		BasePoint = *GripData()->alternateBasePoint();
	}
	auto Override {false};
	computedPoint = BasePoint;
	if (Status() == OdDbGripOperations::kDragImageGrip && GripData().get() && GripData()->drawAtDragImageGripPoint()) {
		computedPoint = BasePoint + (m_GripManager->m_LastPoint - m_GripManager->m_BasePoint);
		Override = true;
	}
	return Override;
}

unsigned long OdExGripData::subSetAttributes(OdGiDrawableTraits* drawableTraits) const {
	if (IsInvisible()) { return kDrawableIsInvisible; }
	auto pEntityTraits = OdGiSubEntityTraits::cast(drawableTraits);
	if (!pEntityTraits.get()) { return kDrawableNone; }
	switch (Status()) {
		case OdDbGripOperations::kWarmGrip:
			pEntityTraits->setTrueColor(m_GripManager->m_GRIPCOLOR);
			break;
		case OdDbGripOperations::kHotGrip: case OdDbGripOperations::kDragImageGrip:
			pEntityTraits->setTrueColor(m_GripManager->m_GRIPHOT);
			break;
		case OdDbGripOperations::kHoverGrip:
			pEntityTraits->setTrueColor(m_GripManager->m_GRIPHOVER);
			break;
	}
	pEntityTraits->setMaterial(nullptr);
	pEntityTraits->setLineWeight(OdDb::kLnWt000);
	return kDrawableRegenDraw;
}

bool OdExGripData::subWorldDraw(OdGiWorldDraw* worldDraw) const {
	auto GripSize {static_cast<double>(m_GripManager->m_GRIPSIZE)};
	if (!worldDraw->context() || !worldDraw->context()->database()) { GripSize = m_GripManager->m_GRIPSIZE; }

	// Here is the design flaw: ARX help says that grip size passed in callback below should be calculated individually for each viewport.
	if (GripData().get() && GripData()->worldDraw()) {
		OdGePoint3d ComputedPoint;
		OdGePoint3d* DrawAtDrag {nullptr};
		if (ComputeDragPoint(ComputedPoint)) { DrawAtDrag = &ComputedPoint; }
		OdGiDrawFlagsHelper DrawFlagsHelper(worldDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);
		return (*GripData()->worldDraw())(static_cast<OdDbGripData*>(GripData().get()), worldDraw, EntityId(), Status(), DrawAtDrag, GripSize);
	}
	return false;
}

void OdExGripData::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	OdGePoint3d ComputedPoint;
	OdGePoint3d* DrawAtDrag {nullptr};
	if (ComputeDragPoint(ComputedPoint)) { DrawAtDrag = &ComputedPoint; }
	OdGiDrawFlagsHelper DrawFlagsHelper(viewportDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);
	auto Default {true};
	if (GripData().get() && GripData()->viewportDraw()) {
		(*GripData()->viewportDraw())(static_cast<OdDbGripData*>(GripData().get()), viewportDraw, EntityId(), Status(), DrawAtDrag, m_GripManager->m_GRIPSIZE);
		Default = false;
	}
	if (Default) {
		OdGePoint2d ptDim;
		viewportDraw->viewport().getNumPixelsInUnitSquare(Point(), ptDim);
		OdGeVector3d v(m_GripManager->m_GRIPSIZE / ptDim.x, 0.0, 0.0);
		v.transformBy(viewportDraw->viewport().getWorldToEyeTransform());
		const auto GripSize {v.length()};
		auto OnScreenPoint {ComputedPoint};
		OnScreenPoint.transformBy(viewportDraw->viewport().getWorldToEyeTransform());
		viewportDraw->subEntityTraits().setFillType(kOdGiFillAlways);
		viewportDraw->subEntityTraits().setDrawFlags(OdGiSubEntityTraits::kDrawSolidFill | OdGiSubEntityTraits::kDrawPolygonFill);
		OdGePoint3d aPoly[4];
		aPoly[0].set(OnScreenPoint.x - GripSize, OnScreenPoint.y - GripSize, OnScreenPoint.z);
		aPoly[1].set(OnScreenPoint.x + GripSize, OnScreenPoint.y - GripSize, OnScreenPoint.z);
		aPoly[2].set(OnScreenPoint.x + GripSize, OnScreenPoint.y + GripSize, OnScreenPoint.z);
		aPoly[3].set(OnScreenPoint.x - GripSize, OnScreenPoint.y + GripSize, OnScreenPoint.z);
		viewportDraw->geometry().polygonEye(4, aPoly);
	}
}

OdBaseGripManager::OdBaseGripManager() noexcept {
	m_GripData.clear();
	m_HoverGripsData.clear();
	m_BasePoint = OdGePoint3d::kOrigin;
	m_LastPoint = OdGePoint3d::kOrigin;
	m_GripDrags.clear();
	m_Disabled = true;
	m_GRIPSIZE = 5;
	m_GRIPOBJLIMIT = 100;
}

OdBaseGripManager::~OdBaseGripManager() {
	EndHover();
}

bool OdBaseGripManager::OnMouseDown(const int x, const int y, const bool shiftIsDown) {
	EndHover();
	OdExGripDataPtrArray aKeys;
	LocateGripsAt(x, y, aKeys);
	if (aKeys.empty()) { return false; }
	if (shiftIsDown) { // Modify Grip  status().
		auto eNewStatus {OdDbGripOperations::kHotGrip};
		const unsigned long iSize = aKeys.size();
		for (unsigned i = 0; i < iSize; i++) {
			if (OdDbGripOperations::kHotGrip == aKeys[i]->Status()) {
				eNewStatus = OdDbGripOperations::kWarmGrip;
				break;
			}
		}
		for (unsigned i = 0; i < iSize; i++) {
			auto eCurStatus {eNewStatus};
			auto Grip {aKeys[i]};
			if (!Grip->GripData().isNull()) {
				if (Grip->GripData()->triggerGrip()) {
					eCurStatus = OdDbGripOperations::kWarmGrip;
				} else {
					if (Grip->GripData()->hotGripFunc()) {
						int Flags {OdDbGripOperations::kMultiHotGrip};
						if (Grip->IsShared()) { Flags |= OdDbGripOperations::kSharedGrip; }
						const auto Result {(*Grip->GripData()->hotGripFunc())(Grip->GripData(), Grip->EntityId(), Flags)};
						if (Result == eGripOpGripHotToWarm) { eCurStatus = OdDbGripOperations::kWarmGrip; }
					}
				}
			}
			aKeys[i]->SetStatus(eCurStatus);
		}
	} else { // Launch Grip Edit.
		auto MakeHot {true};
		{
			GripDataMap::const_iterator GripDataIterator {m_GripData.begin()};
			while (GripDataIterator != m_GripData.end() && MakeHot) {
				const auto& aData {GripDataIterator->second.m_pDataArray};
				for (unsigned i = 0; i < aData.size(); i++) {
					if (OdDbGripOperations::kHotGrip == aData[i]->Status()) {
						MakeHot = false;
						break;
					}
				}
				for (unsigned i = 0; i < GripDataIterator->second.m_GripDataSubEntity.size() && MakeHot; i++) {
					const auto& aData {GripDataIterator->second.m_GripDataSubEntity.at(i).m_pSubData};
					for (unsigned j = 0; j < aData.size(); j++) {
						if (OdDbGripOperations::kHotGrip == aData[j]->Status()) {
							MakeHot = false;
							break;
						}
					}
				}
				GripDataIterator++;
			}
		}
		auto GetNew {false};
		OdDbObjectId idEntityToUpdate;
		if (MakeHot) {
			const auto Size {aKeys.size()};
			for (unsigned i = 0; i < Size; i++) {
				auto Grip {aKeys[i]};
				auto eNew {OdDbGripOperations::kHotGrip};
				if (!Grip->GripData().isNull() && Grip->GripData()->hotGripFunc()) {
					auto Flags {0};
					if (Grip->IsShared()) { Flags |= OdDbGripOperations::kSharedGrip; }
					if (Grip->GripData()->triggerGrip()) {
						if (!Grip->IsShared()) {
							const auto Result {(*Grip->GripData()->hotGripFunc())(Grip->GripData(), Grip->EntityId(), Flags)};
							if (Result == eOk || Result == eGripOpGripHotToWarm) {
								eNew = OdDbGripOperations::kWarmGrip;
							} else if (Result == eGripOpGetNewGripPoints) {
								GetNew = true;
								idEntityToUpdate = Grip->EntityId();
							}
						}
					} else {
						const auto Result {(*Grip->GripData()->hotGripFunc())(Grip->GripData(), Grip->EntityId(), Flags)};
						if (!Grip->IsShared()) {
							if (Result == eGripOpGripHotToWarm) {
								eNew = OdDbGripOperations::kWarmGrip;
							} else if (Result == eGripOpGetNewGripPoints) {
								GetNew = true;
								idEntityToUpdate = Grip->EntityId();
							}
						}
					}
				}
				Grip->SetStatus(eNew);
			}
		}
		if (GetNew) { UpdateEntityGrips(idEntityToUpdate); }
	}
	return true;
}

OdResult OdBaseGripManager::StartHover(const int x, const int y, const bool shiftIsDown) {
	auto Result {eOk};
	if (!EndHover()) {
		Result = eGripOpFailure;
		clockStartHover = 0;
	}
	OdExGripDataPtrArray aKeys;
	LocateGripsAt(x, y, aKeys);
	if (!aKeys.empty()) {
		m_HoverGripsData = aKeys;
		for (unsigned i = 0; i < m_HoverGripsData.size(); i++) {
			auto Grip {m_HoverGripsData[i]};
			if (Grip->Status() == OdDbGripOperations::kWarmGrip) {
				Grip->SetStatus(OdDbGripOperations::kHoverGrip);
				if (!Grip->GripData().isNull()) {
					if ((Grip->GripData()->hoverFunc() != nullptr) && !shiftIsDown) {
						if (!clockStartHover) { clockStartHover = clock(); }
						if ((clock() - clockStartHover) * 1000 / CLOCKS_PER_SEC > 300) { // 300 ms delay before hover
							auto Flags {0};
							if (Grip->IsShared()) { Flags = OdDbGripOperations::kSharedGrip; }
							Result = (*Grip->GripData()->hoverFunc())(Grip->GripData(), Grip->EntityId(), Flags);
							if (Result == eGripOpGetNewGripPoints) {
								clockStartHover = 0;
								aKeys[i]->SetStatus(OdDbGripOperations::kHotGrip);
								m_BasePoint = aKeys.first()->Point();
								m_LastPoint = m_BasePoint;
								auto FirstData {aKeys.first()->GripData()};
								if (FirstData.get() != nullptr && FirstData->alternateBasePoint() != nullptr) { // Use alternative point
									m_BasePoint = *(FirstData->alternateBasePoint());
								}
							}
						}
					}
				}
				OnModified(Grip);
			}
		}
	}
	return Result;
}

bool OdBaseGripManager::EndHover() {
	if (m_HoverGripsData.empty()) { return false; }
	for (auto HoverGripData : m_HoverGripsData) {
		if (HoverGripData->Status() == OdDbGripOperations::kHoverGrip) {
			HoverGripData->SetStatus(OdDbGripOperations::kWarmGrip);
			OnModified(HoverGripData);
		}
	}
	m_HoverGripsData.clear();
	return true;
}

void OdBaseGripManager::SelectionSetChanged(OdSelectionSet* selectionSet) {
	auto RestoreOld {false};
	if (selectionSet->numEntities() > static_cast<unsigned>(m_GRIPOBJLIMIT)) {
		Disable(true);
	} else {
		if (IsDisabled()) { RestoreOld = true; }
		Disable(false);
	}
	auto Database {OdDbDatabase::cast(selectionSet->baseDatabase()).get()};
	{ // Old Entities.
		OdDbStubPtrArray aOld;
		auto GripDataIterator {m_GripData.begin()};
		while (GripDataIterator != m_GripData.end()) {
			if (IsDisabled()) {
				aOld.push_back(GripDataIterator->first);
			} else {
				if (!selectionSet->isMember(GripDataIterator->first)) {
					aOld.push_back(GripDataIterator->first);
				} else {
					// Remove if subentities changed
					auto Removed {false};
					for (unsigned long se = 0; se < GripDataIterator->second.m_GripDataSubEntity.size(); se++) {
						if (!selectionSet->isMember(GripDataIterator->second.m_GripDataSubEntity[se].subentPath)) {
							aOld.push_back(GripDataIterator->first);
							Removed = true;
							break;
						}
					}
					// Remove if new paths added also (workaround. technically new paths must be added on second step)
					if (!Removed) {
						auto SelectionSetIterator {searchObjectSSetIterator(selectionSet, GripDataIterator->first)};
						for (unsigned SubEntityIndex = 0; SubEntityIndex < SelectionSetIterator->subentCount(); SubEntityIndex++) {
							OdDbBaseFullSubentPath FullSubEntityPath;
							SelectionSetIterator->getSubentity(SubEntityIndex, FullSubEntityPath);
							auto searchPath {0u};
							auto Found {false};
							for (; searchPath < GripDataIterator->second.m_GripDataSubEntity.size(); searchPath++) {
								if (GripDataIterator->second.m_GripDataSubEntity.at(searchPath).subentPath == FullSubEntityPath) {
									Found = true;
									break;
								}
							}
							if (!Found) {
								aOld.push_back(GripDataIterator->first);
								break;
							}
						}
					}
				}
			}
			GripDataIterator++;
		}
		const auto Size {aOld.size()};
		for (unsigned i = 0; i < Size; i++) {
			RemoveEntityGrips(aOld[i], true);
			if (i % gc_GripManagerPageEachObject && Database) { Database->pageObjects(); }
		}
	}
	{ // New Entities.
		OdDbStubPtrArray aNew;
		auto SelectionSetIterator {selectionSet->newIterator()};
		while (!SelectionSetIterator->done()) {
			if (!IsDisabled() && m_GripData.end() == m_GripData.find(SelectionSetIterator->id())) { aNew.push_back(SelectionSetIterator->id()); }
			SelectionSetIterator->next();
		}
		const auto Size {aNew.size()};
		for (unsigned i = 0; i < Size; i++) {
			UpdateEntityGrips(aNew[i]);
			if (i % gc_GripManagerPageEachObject && Database) { Database->pageObjects(); }
		}
	}
	UpdateInvisibleGrips();
}

void OdBaseGripManager::UpdateEntityGrips(OdDbStub* id) {
	RemoveEntityGrips(id, false);
	auto SelectionSet {WorkingSelectionSet()};
	if (SelectionSet.isNull() || !SelectionSet->isMember(id)) { return; }
	auto Entity {OpenObject(id)};
	if (Entity.isNull()) { return; }
	OdExGripDataPtrArray aExt;
	OdDbGripDataPtrArray aPts;
	auto SelectionSetIterator {searchObjectSSetIterator(SelectionSet, id)};
	if (SelectionSetIterator->subentCount() > 0) {
		for (unsigned long se = 0; se < SelectionSetIterator->subentCount(); se++) {
			OdDbBaseFullSubentPath subEntPath;
			SelectionSetIterator->getSubentity(se, subEntPath);
			aPts.clear();
			if (GetGripPointsAtSubentPath(Entity, subEntPath, aPts, ActiveViewUnitSize(), m_GRIPSIZE, ActiveViewDirection(), 0) == eOk) {
				const unsigned long prevSize = aExt.size();
				aExt.resize(prevSize + aPts.size());
				for (unsigned i = 0; i < aPts.size(); i++) {
					aExt[i + prevSize] = OdExGripData::CreateObject(subEntPath, aPts[i], aPts[i]->gripPoint(), this);
				}
			}
		}
	} else {
		if (eOk == GetGripPoints(Entity, aPts, ActiveViewUnitSize(), m_GRIPSIZE, ActiveViewDirection(), 0)) {
			aExt.resize(aPts.size());
			const auto Size {aExt.size()};
			for (unsigned i = 0; i < Size; i++) {
				aExt[i] = OdExGripData::CreateObject(id, aPts[i], aPts[i]->gripPoint(), this);
			}
		} else {
			OdGePoint3dArray aOldPts;
			if (eOk == GetGripPoints(Entity, aOldPts)) {
				aExt.resize(aOldPts.size());
				const auto Size = aExt.size();
				for (unsigned i = 0; i < Size; i++) {
					aExt[i] = OdExGripData::CreateObject(id, nullptr, aOldPts[i], this);
				}
			}
		}
	}
	const auto bModel {IsModel(Entity)};
	if (!aExt.empty()) {
		const auto Size = aExt.size();
		OdExGripDataExt dExt;
		for (unsigned i = 0; i < Size; i++) {
			OdDbBaseFullSubentPath EntityPath;
			if (aExt[i]->EntityPath(&EntityPath)) {
				auto Found {false};
				for (auto& GripDatum : dExt.m_GripDataSubEntity) {
					if (GripDatum.subentPath == EntityPath) {
						Found = true;
						GripDatum.m_pSubData.append(aExt[i]);
						break;
					}
				}
				if (!Found) {
					OdExGripDataSubent se;
					se.subentPath = EntityPath;
					se.m_pSubData.append(aExt[i]);
					dExt.m_GripDataSubEntity.append(se);
				}
			} else {
				dExt.m_pDataArray.append(aExt[i]);
			}
		}
		m_GripData.insert(std::make_pair(id, dExt));
		for (unsigned i = 0; i < Size; i++) {
			ShowGrip(aExt[i], bModel);
		}
	}
}

void OdBaseGripManager::RemoveEntityGrips(OdDbStub* id, const bool fireDone) {
	auto GripDataIterator {m_GripData.find(id)};
	if (GripDataIterator != m_GripData.end()) {
		auto Entity {OpenObject(id)};
		if (Entity.get()) { GripStatus(Entity, OdDb::kGripsToBeDeleted); }
		const auto Model {IsModel(Entity)};
		const auto Size = GripDataIterator->second.m_pDataArray.size();
		for (unsigned i = 0; i < Size; i++) {
			auto GripData {GripDataIterator->second.m_pDataArray[i]};
			HideGrip(GripData, Model);
			if (!GripDataIterator->second.m_pDataArray[i]->GripData().isNull() && GripDataIterator->second.m_pDataArray[i]->GripData()->gripOpStatFunc()) {
				(*GripDataIterator->second.m_pDataArray[i]->GripData()->gripOpStatFunc())(GripDataIterator->second.m_pDataArray[i]->GripData(), id, OdDbGripOperations::kGripEnd);
			}
			GripDataIterator->second.m_pDataArray[i] = nullptr;
		}
		for (auto& GripDataSubEntity : GripDataIterator->second.m_GripDataSubEntity) {
			for (auto& GripData : GripDataSubEntity.m_pSubData) {
				auto GripDataCopy {GripData};
				HideGrip(GripDataCopy, Model);
				GripData = nullptr;
			}
		}
		if (fireDone) {
			if (Entity.get()) { GripStatus(Entity, OdDb::kGripsDone); }
		}
		m_GripData.erase(GripDataIterator);
	}
}

void OdBaseGripManager::LocateGripsAt(const int x, const int y, OdExGripDataPtrArray& aResult) {
	aResult.clear();
	const auto X {static_cast<double>(x)};
	const auto Y {static_cast<double>(y)};
	OdGePoint3d FirstPoint;
	GripDataMap::const_iterator GripDataIterator {m_GripData.begin()};
	while (GripDataIterator != m_GripData.end()) {
		for (unsigned se = 0; se < GripDataIterator->second.m_GripDataSubEntity.size() + 1; se++) {
			const auto& aData = se == 0 ? GripDataIterator->second.m_pDataArray : GripDataIterator->second.m_GripDataSubEntity[se - 1].m_pSubData;
			const auto DataSize {aData.size()};
			for (unsigned i = 0; i < DataSize; i++) {
				const auto& CurrentPoint {aData[i]->Point()};
				if (aResult.empty()) { // First grip is obtained by comparing grip point device position with cursor position.
					auto ptDC {CurrentPoint};
					ptDC.transformBy(ActiveGsView()->worldToDeviceMatrix());
					const auto DeltaX {fabs(X - ptDC.x)};
					const auto DeltaY {fabs(Y - ptDC.y)};
					const auto Ok {DeltaX <= m_GRIPSIZE && DeltaY <= m_GRIPSIZE};
					if (Ok) {
						FirstPoint = CurrentPoint;
						aResult.push_back(aData[i]);
					}
				} else { // Other grips are obtained by comparing world coordinates. The approach here is quite raw.
					if (CurrentPoint.isEqualTo(FirstPoint, 1E-4)) { aResult.push_back(aData[i]); }
				}
			}
		}
		GripDataIterator++;
	}
}

void OdBaseGripManager::LocateGripsByStatus(const OdDbGripOperations::DrawType eStatus, OdExGripDataPtrArray& aResult) {
	aResult.clear();
	GripDataMap::const_iterator GripDataIterator {m_GripData.begin()};
	while (GripDataIterator != m_GripData.end()) {
		for (unsigned long se = 0; se < GripDataIterator->second.m_GripDataSubEntity.size() + 1; se++) {
			const auto& aData {se == 0 ? GripDataIterator->second.m_pDataArray : GripDataIterator->second.m_GripDataSubEntity[se - 1].m_pSubData};
			const auto Size {aData.size()};
			for (unsigned i = 0; i < Size; i++) {
				if (eStatus == aData[i]->Status()) { aResult.push_back(aData[i]); }
			}
		}
		GripDataIterator++;
	}
}

namespace
{
	struct SortGripsAlongXAxis {
		bool operator()(const OdExGripDataPtr& grA, const OdExGripDataPtr& grB) {
			return OdPositive(grA->Point().x, grB->Point().x);
		}
	};
}

void OdBaseGripManager::UpdateInvisibleGrips() {
	OdExGripDataPtrArray Overall;
	GripDataMap::const_iterator GripDataIterator {m_GripData.begin()};
	while (GripDataIterator != m_GripData.end()) {
		Overall.insert(Overall.end(), GripDataIterator->second.m_pDataArray.begin(), GripDataIterator->second.m_pDataArray.end());
		for (const auto& GripDatum : GripDataIterator->second.m_GripDataSubEntity) {
			Overall.insert(Overall.end(), GripDatum.m_pSubData.begin(), GripDatum.m_pSubData.end());
		}
		GripDataIterator++;
	}
	auto Size {Overall.size()};
	for (unsigned i = 0; i < Size; i++) {
		Overall[i]->SetInvisible(false);
		Overall[i]->SetShared(false);
	}

	// Not the best approach for sorting.
	// Just for demonstration.
	std::sort(Overall.begin(), Overall.end(), SortGripsAlongXAxis());
	Size = Overall.size();
	for (unsigned i = 0; i < Size; i++) {
		if (Overall[i]->IsShared()) continue;
		OdUInt32Array aEq;
		aEq.push_back(i);
		const auto ptIni = Overall[i]->Point();
		auto iNext = i + 1;
		while (iNext < Size) {
			const auto ptCur {Overall[iNext]->Point()};
			if (OdEqual(ptIni.x, ptCur.x, 1E-6)) {
				if (ptIni.isEqualTo(ptCur, 1E-6)) { aEq.push_back(iNext); }
				iNext++;
			} else {
				break;
			}
		}
		if (aEq.size() >= 2) {
			auto Visible {0u};
			const auto jSize = aEq.size();
			for (unsigned j = 0; j < jSize; j++) {
				auto pGrip {Overall[aEq[j]]};
				auto Ok {true};
				if (!pGrip->GripData().isNull()) {
					if (pGrip->GripData()->skipWhenShared()) { Ok = false; }
				} else {
					Ok = false;
				}
				if (Ok) {
					Visible = j;
					break;
				}
			}
			for (unsigned j = 0; j < jSize; j++) {
				auto pGrip {Overall[aEq[j]]};
				pGrip->SetShared(true);
				pGrip->SetInvisible(j != Visible);
			}
		}
	}
}

void OdBaseGripManager::setValue(const OdGePoint3d& value) {
	const auto NewPoint {EyeToUcsPlane(value, m_BasePoint)};
	const auto Size {m_GripDrags.size()};
	for (unsigned GripDragIndex = 0; GripDragIndex < Size; GripDragIndex++) {
		m_GripDrags[GripDragIndex]->CloneEntity(NewPoint);
	}
	m_LastPoint = NewPoint;
}

OdDbStub* OdBaseGripManager::ActiveViewportId() const {
	OdGsClientViewInfo ClientViewInfo;
	ActiveGsView()->clientViewInfo(ClientViewInfo);
	return ClientViewInfo.viewportObjectId;
}

double OdBaseGripManager::ActiveViewUnitSize() const {
	const auto ActiveView {ActiveGsView()};

	// <tas="Duplicates function of inaccessible 'OdGiViewport::getNumPixelsInUnitSquare' here."/>
	OdGePoint2d LowerLeft;
	OdGePoint2d UpperRight;
	ActiveView->getViewport(LowerLeft, UpperRight);
	OdGsDCRect ScreenRectangle;
	ActiveView->getViewport(ScreenRectangle);
	OdGePoint2d ptDim;
	ptDim.x = fabs(double(ScreenRectangle.m_max.x - ScreenRectangle.m_min.x) / ActiveView->fieldWidth() * (UpperRight.x - LowerLeft.x));
	ptDim.y = fabs(double(ScreenRectangle.m_max.y - ScreenRectangle.m_min.y) / ActiveView->fieldHeight() * (UpperRight.y - LowerLeft.y));
	OdGeVector3d v(m_GRIPSIZE / ptDim.x, 0, 0);
	v.transformBy(ActiveView->viewingMatrix());
	return v.length() / m_GRIPSIZE;
}

OdGeVector3d OdBaseGripManager::ActiveViewDirection() const {
	const auto View {ActiveGsView()};
	return (View->position() - View->target()).normal();
}

void OdBaseGripManager::Disable(const bool disable) {
	m_Disabled = disable;
}

OdGiDrawablePtr OdExGripManager::CloneEntity(OdDbStub* id) {
	auto Entity {OdDbEntity::cast(OdDbObjectId(id).openObject())};
	if (Entity.isNull()) { return OdGiDrawablePtr(); }
	OdDbEntityPtr Clone;
	if (Entity->cloneMeForDragging()) { Clone = OdDbEntity::cast(Entity->clone()); }
	if (Clone.get()) {
		Clone->disableUndoRecording(true);
		Clone->setPropertiesFrom(Entity.get(), false);
	}
	return OdGiDrawable::cast(Clone);
}

void OdExGripManager::OnModified(OdGiDrawable* grip) {
	if (GetGsModel()) {
		GetGsModel()->onModified(grip, static_cast<OdGiDrawable*>(nullptr));
	} else if (GetGsLayoutHelper()) {
		GetGsLayoutHelper()->invalidate();
	}
}

OdExGripManager::OdExGripManager() noexcept {
	m_LayoutHelper = nullptr;
	m_cDbReactor.m_GripManager = this;
	m_pGetSelectionSetPtr = nullptr;
}

void OdExGripManager::Initialize(OdGsDevice* device, OdGsModel* gsModel, OdDbCommandContext* commandContext, GetSelectionSetPtr getSSet) {
	m_LayoutHelper = device;
	m_pGsModel = gsModel;
	m_CommandContext = commandContext;
	if (m_CommandContext->baseDatabase()) {
		const auto Database {m_CommandContext->database()};
		Disable(false);
		const auto HostApplicationServices {Database->appServices()};
		m_GRIPSIZE = HostApplicationServices->getGRIPSIZE();
		m_GRIPOBJLIMIT = HostApplicationServices->getGRIPOBJLIMIT();
		m_GRIPCOLOR.setColorIndex(HostApplicationServices->getGRIPCOLOR());
		m_GRIPHOVER.setColorIndex(HostApplicationServices->getGRIPHOVER());
		m_GRIPHOT.setColorIndex(HostApplicationServices->getGRIPHOT());
	}
	m_pGetSelectionSetPtr = getSSet;
	m_gripStretchCommand.m_parent = this;
}

void OdExGripManager::Uninitialize() {
	if (m_CommandContext) {
		Disable(true);
		m_CommandContext = nullptr;
	}
	m_LayoutHelper = nullptr;
}

void OdExGripManager::OdExGripCommand::execute(OdEdCommandContext* /*edCommandContext*/) {
	auto Ok = true;
	try {
		const auto FinalPoint {
		m_parent->m_CommandContext->dbUserIO()->getPoint(L"Specify stretch point or [Base point/Copy/Undo/eXit]:", OdEd::kGptNoLimCheck | OdEd::kGptDefault | OdEd::kGptNoUCS, &m_parent->m_BasePoint, L"Base Copy Undo eXit", m_parent)
		};
		for (auto& ParentGripDrag : m_parent->m_GripDrags) {
			ParentGripDrag->MoveEntity(m_parent->EyeToUcsPlane(FinalPoint, m_parent->m_BasePoint));
		}
	} catch (const OdEdCancel&) {
		Ok = false;
		for (auto& ParentGripDrag : m_parent->m_GripDrags) {
			ParentGripDrag->NotifyDragAborted();
		}
	}
	for (auto& ParentGripDrag : m_parent->m_GripDrags) {
		if (Ok) {
			ParentGripDrag->NotifyDragEnded();
			m_parent->UpdateEntityGrips(ParentGripDrag->EntityId());
		} else {
			ParentGripDrag->NotifyDragAborted();
		}
	}
	m_parent->m_GripDrags.clear();
	if (Ok) { m_parent->UpdateInvisibleGrips(); }
}

void OdExGripManager::AddToDrag(OdExGripDataPtrArray& aActiveKeys) {
	OdUInt32 i, iSize = aActiveKeys.size();
	for (i = 0; i < iSize; i++)
		aActiveKeys[i]->SetStatus(OdDbGripOperations::kDragImageGrip);
	GripDataMap::const_iterator it = m_GripData.begin();
	while (it != m_GripData.end()) {
		bool bActive = false;
		OdExGripDragPtr pDrag;
		{
			const OdExGripDataPtrArray& aData = it->second.m_pDataArray;
			OdUInt32 i, iSize = aData.size();
			for (i = 0; i < iSize; i++) {
				if (OdDbGripOperations::kDragImageGrip == aData[i]->Status()) {
					bActive = true;
					pDrag = OdExGripDrag::CreateObject(it->first, this);
					break;
				}
			}
			for (i = 0; (i < it->second.m_GripDataSubEntity.size()) && !bActive; i++) {
				const OdExGripDataPtrArray& aData = it->second.m_GripDataSubEntity.at(i).m_pSubData;
				OdUInt32 j, iSize = aData.size();
				for (j = 0; j < iSize; j++) {
					if (OdDbGripOperations::kDragImageGrip == aData[j]->Status()) {
						bActive = true;
						pDrag = OdExGripDrag::CreateObject(it->second.m_GripDataSubEntity.at(i).subentPath, this);
						break;
					}
				}
			}
		}
		if (bActive) {
			//pDrag->entityId = it->first;
			//pDrag->m_pOwner = this;
			m_GripDrags.push_back(pDrag);
		}
		it++;
	}
	iSize = m_GripDrags.size();
	for (i = 0; i < iSize; i++) {
		m_GripDrags[i]->NotifyDragStarted();
		m_GripDrags[i]->CloneEntity();
	}
}

bool OdExGripManager::OnMouseDown(const int x, const int y, const bool shiftIsDown) {
	if (!OdBaseGripManager::OnMouseDown(x, y, shiftIsDown)) { return false; }
	if (shiftIsDown) { return true; }
	OdExGripDataPtrArray aKeys;
	LocateGripsAt(x, y, aKeys);
	if (aKeys.empty()) { return true; }
	OdExGripDataPtrArray ActiveKeys;
	LocateGripsByStatus(OdDbGripOperations::kHotGrip, ActiveKeys);
	if (ActiveKeys.empty()) {
		return false;
	} // Valid situation. If trigger grip performed entity modification and returned eGripHotToWarm then nothing is to be done cause entity modification will cause reactor to regen grips.
	if (HandleMappedRtClk(ActiveKeys, x, y)) { return true; }
	for (auto& ActiveKey : ActiveKeys) {
		ActiveKey->SetStatus(OdDbGripOperations::kDragImageGrip);
	}
	GripDataMap::const_iterator GripDataIterator {m_GripData.begin()};
	while (GripDataIterator != m_GripData.end()) {
		auto Active {false};
		OdExGripDragPtr GripDrag;
		{
			const auto& aData {GripDataIterator->second.m_pDataArray};
			for (const auto& GripDatum : aData) {
				if (OdDbGripOperations::kDragImageGrip == GripDatum->Status()) {
					Active = true;
					GripDrag = OdExGripDrag::CreateObject(GripDataIterator->first, this);
					break;
				}
			}
			for (unsigned i = 0; i < GripDataIterator->second.m_GripDataSubEntity.size() && !Active; i++) {
				const auto& aData {GripDataIterator->second.m_GripDataSubEntity.at(i).m_pSubData};
				for (const auto& GripDatum : aData) {
					if (OdDbGripOperations::kDragImageGrip == GripDatum->Status()) {
						Active = true;
						GripDrag = OdExGripDrag::CreateObject(GripDataIterator->second.m_GripDataSubEntity.at(i).subentPath, this);
						break;
					}
				}
			}
		}
		if (Active) { m_GripDrags.push_back(GripDrag); }
		GripDataIterator++;
	}
	const auto GripDragsSize {m_GripDrags.size()};
	for (unsigned i = 0; i < GripDragsSize; i++) {
		m_GripDrags[i]->NotifyDragStarted();
		m_GripDrags[i]->CloneEntity();
	}
	m_BasePoint = aKeys.first()->Point();
	m_LastPoint = m_BasePoint;
	{
		// Use alternative point if needed.
		auto FirstGripData = aKeys.first()->GripData();
		if (FirstGripData.get() != nullptr) {
			if (FirstGripData->alternateBasePoint() != nullptr) { m_BasePoint = *FirstGripData->alternateBasePoint(); }
		}
	}
	m_CommandContext->database()->startUndoRecord();
	odedRegCmds()->executeCommand(&m_gripStretchCommand, m_CommandContext);
	for (auto& ActiveKey : ActiveKeys) {
		ActiveKey->SetStatus(OdDbGripOperations::kWarmGrip);
	}
	return true;
}

bool OdExGripManager::OnMouseMove(const int x, const int y, const bool shiftIsDown) {
	// restart hover operation
	const auto Result {StartHover(x, y, shiftIsDown)};
	if (Result == eGripOpFailure) { return false; }
	if (Result == eGripOpGetNewGripPoints) {
		OdExGripDataPtrArray ActiveKeys;
		LocateGripsByStatus(OdDbGripOperations::kHotGrip, ActiveKeys);
		if (ActiveKeys.empty()) { return false; } // Valid situation. If trigger grip performed entity modification and returned eGripHotToWarm then nothing is to be done cause entity modification will cause reactor to regen grips.
		AddToDrag(ActiveKeys);
		m_CommandContext->database()->startUndoRecord();
		::odedRegCmds()->executeCommand(&m_gripStretchCommand, m_CommandContext);
		for (auto& ActiveKey : ActiveKeys) {
			ActiveKey->SetStatus(OdDbGripOperations::kWarmGrip);
		}
	}
	return true;
}

bool OdExGripManager::OnControlClick() {
	if (m_GripDrags.empty()) { return false; }

	// TODO: Notify active grips.
	// AEC grips use CTRL key to change mode, but how to pass it threw standard interface is currently unknown.
	return true;
}

void OdExGripManager::ShowGrip(OdExGripData* gripData, bool /*model*/) {
	auto PaperLayoutHelper {OdGsPaperLayoutHelper::cast(m_LayoutHelper)};
	const auto NumberOfViews {m_LayoutHelper->numViews()};
	if (PaperLayoutHelper.get()) {
		auto ActiveViewport {m_CommandContext->database()->activeViewportId().openObject()};
		OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewport)) {
			AbstractViewportData->gsView(ActiveViewport)->add(gripData, m_pGsModel);
		} else {
			PaperLayoutHelper->overallView()->add(gripData, m_pGsModel);
		}
	} else {
		for (auto ViewIndex = 0; ViewIndex < NumberOfViews; ViewIndex++) {
			m_LayoutHelper->viewAt(ViewIndex)->add(gripData, m_pGsModel);
		}
	}
}

void OdExGripManager::HideGrip(OdExGripData* gripData, bool /*model*/) {
	auto PaperLayoutHelper {OdGsPaperLayoutHelper::cast(m_LayoutHelper)};
	const auto NumberOfViews {m_LayoutHelper->numViews()};
	if (PaperLayoutHelper.get()) {
		for (auto ViewIndex = 0; ViewIndex < NumberOfViews; ViewIndex++) {
			m_LayoutHelper->viewAt(ViewIndex)->erase(gripData);
		}
	} else {
		for (auto ViewIndex = 0; ViewIndex < NumberOfViews; ViewIndex++) {
			m_LayoutHelper->viewAt(ViewIndex)->erase(gripData);
		}
	}
}

int OdExGripManager::addDrawables(OdGsView* view) {
	ODA_ASSERT(view->device() == m_LayoutHelper->underlyingDevice().get());
	const auto Size = m_GripDrags.size();
	for (unsigned i = 0; i < Size; i++) {
		view->add(m_GripDrags[i].get(), /*m_pGsModel*/ nullptr);
	}
	return gsl::narrow_cast<int>(Size);
}

void OdExGripManager::removeDrawables(OdGsView* view) {
	const unsigned long iSize = m_GripDrags.size();
	for (unsigned i = 0; i < iSize; i++) {
		view->erase(m_GripDrags[i].get());
	}
}

inline void resetDragging(OdGsDevice* device, const bool option) {
	if (!device) { return; }
	auto Properties {device->properties()};
	if (Properties.isNull()) { return; }
	if (!Properties->has(L"DrawDragging")) { return; }
	Properties->putAt(L"DrawDragging", OdRxVariantValue(option));
}

void OdExGripManager::DraggingStarted() {
	resetDragging(m_LayoutHelper, true);
}

void OdExGripManager::DraggingStopped() {
	resetDragging(m_LayoutHelper, false);
}

OdSelectionSetPtr OdExGripManager::WorkingSelectionSet() const {
	if (m_pGetSelectionSetPtr) { return OdSelectionSet::cast(m_pGetSelectionSetPtr(m_CommandContext)); }
	return OdSelectionSetPtr();
}

OdGsView* OdExGripManager::ActiveGsView() const {
	return m_LayoutHelper->activeView();
}

OdGePoint3d OdExGripManager::EyeToUcsPlane(const OdGePoint3d& point, const OdGePoint3d& basePoint) const {
	auto ActiveViewport {OdDbObjectId(ActiveViewportId()).safeOpenObject()};
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewport);
	OdGePoint3d UcsOrigin;
	OdGeVector3d UcsXAxis;
	OdGeVector3d UcsYAxis;
	AbstractViewportData->getUcs(ActiveViewport, UcsOrigin, UcsXAxis, UcsYAxis);
	const OdGePlane Plane {basePoint, UcsXAxis, UcsYAxis};
	OdGeLine3d Line(point, ActiveViewDirection());
	OdGePoint3d NewPoint;
	if (!Plane.intersectWith(Line, NewPoint)) {
		Line.set(point, UcsXAxis.crossProduct(UcsYAxis));
		if (!Plane.intersectWith(Line, NewPoint)) { NewPoint = basePoint; }
	}
	return NewPoint;
}

bool OdExGripManager::HandleMappedRtClk(OdExGripDataPtrArray& activeKeys, const int x, const int y) {
#if !defined(ODA_UNIXOS) && !(defined(ODA_WINDOWS) && !defined(OD_WINDOWS_DESKTOP))
	const auto Size {activeKeys.size()};
	auto RightClickIndex {-1};
	for (unsigned Index = 0; Index < Size; Index++) {
		if (!activeKeys[Index]->GripData().isNull() && activeKeys[Index]->GripData()->rtClk() != nullptr && activeKeys[Index]->GripData()->mapGripHotToRtClk() && !activeKeys[Index]->IsShared()) {
			RightClickIndex = static_cast<int>(Index);
			break;
		}
	}
	if (RightClickIndex != -1) {
		OdDbStubPtrArray Entities;
		OdDbGripDataArray HotGrips;
		for (unsigned i = 0; i < Size; i++) {
			HotGrips.append(*activeKeys[i]->GripData());
			if (!Entities.contains(activeKeys[i]->EntityId())) { Entities.append(activeKeys[i]->EntityId()); }
		}
		OdString MenuName;
		ODHMENU Menu {nullptr};
		ContextMenuItemIndexPtr cb {nullptr};
		const auto Result {(*activeKeys[static_cast<unsigned>(RightClickIndex)]->GripData()->rtClk())(HotGrips, Entities, MenuName, Menu, cb)};
		if (Result == eOk && Menu != nullptr && cb != nullptr) {
			const auto ActiveWindow {GetActiveWindow()};
			POINT pt = {x, y};
			ClientToScreen(ActiveWindow, &pt);
			const unsigned Flags {TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_NOANIMATION};
			(*cb)(TrackPopupMenu(static_cast<HMENU>(Menu), Flags, pt.x, pt.y, 0, ActiveWindow, nullptr));
			DestroyMenu(static_cast<HMENU>(Menu));
			for (unsigned i = 0; i < Size; i++) {
				activeKeys[i]->SetStatus(OdDbGripOperations::kWarmGrip);
			}
			UpdateEntityGrips(activeKeys[static_cast<unsigned>(RightClickIndex)]->EntityId());
			return true;
		}
	}
#endif // ODA_UNIXOS
	return false;
}

void OdExGripManager::Disable(const bool disable) {
	if (m_Disabled != disable) {
		const auto Database {m_CommandContext->database()};
		m_Disabled = disable;
		if (disable) {
			Database->removeReactor(&m_cDbReactor);
		} else {
			Database->addReactor(&m_cDbReactor);
		}
	}
}

OdGiDrawablePtr OdExGripManager::OpenObject(OdDbStub* id, const bool isForWriteMode) {
	OdGiDrawablePtr Drawable;
	if (!id) { return Drawable; }
	Drawable = OdGiDrawable::cast(OdDbObjectId(id).openObject(isForWriteMode ? OdDb::kForWrite : OdDb::kForRead));
	return Drawable;
}

OdResult OdExGripManager::GetGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, const double curViewUnitSize, const int gripSize, const OdGeVector3d& curViewDir, const unsigned long bitFlags) const {
	return OdDbEntity::cast(entity)->getGripPointsAtSubentPath(*static_cast<const OdDbFullSubentPath*>(& path), grips, curViewUnitSize, gripSize, curViewDir, bitFlags);
}

OdResult OdExGripManager::GetGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, const double curViewUnitSize, const int gripSize, const OdGeVector3d& curViewDir, const int bitFlags) const {
	OdDbEntity* Entity {OdDbEntity::cast(entity)};
	if (!Entity) { return eNotApplicable; }
	return Entity->getGripPoints(grips, curViewUnitSize, gripSize, curViewDir, bitFlags);
}

OdResult OdExGripManager::GetGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const {
	OdDbEntity* Entity {OdDbEntity::cast(entity)};
	if (!Entity) { return eNotApplicable; }
	return Entity->getGripPoints(gripPoints);
}

OdResult OdExGripManager::MoveGripPointsAtSubentPaths(OdGiDrawable* entity, const OdDbBaseFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, const unsigned long bitFlags) {
	ODA_ASSERT_ONCE(sizeof(OdDbFullSubentPath) == sizeof(OdDbBaseFullSubentPath));
	return OdDbEntity::cast(entity)->moveGripPointsAtSubentPaths(*reinterpret_cast<const OdDbFullSubentPathArray*>(& paths), gripAppData, offset, bitFlags);
}

OdResult OdExGripManager::MoveGripPointsAt(OdGiDrawable* entity, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, const int bitFlags) {
	return OdDbEntity::cast(entity)->moveGripPointsAt(gripAppData, offset, bitFlags);
}

OdResult OdExGripManager::MoveGripPointsAt(OdGiDrawable* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return OdDbEntity::cast(entity)->moveGripPointsAt(indices, offset);
}

void OdExGripManager::SubentGripStatus(OdGiDrawable* entity, const OdDb::GripStat status, const OdDbBaseFullSubentPath& subentity) {
	OdDbEntity::cast(entity)->subentGripStatus(status, *static_cast<const OdDbFullSubentPath*>(& subentity));
}

void OdExGripManager::GripStatus(OdGiDrawable* entity, const OdDb::GripStat status) {
	OdDbEntity::cast(entity)->gripStatus(status);
}

void OdExGripManager::DragStatus(OdGiDrawable* entity, const OdDb::DragStat status) {
	OdDbEntity::cast(entity)->dragStatus(status);
}

bool OdExGripManager::IsModel(OdGiDrawable* entity) noexcept {
	OdDbEntity* Entity {OdDbEntity::cast(entity)};
	return !Entity || Entity->database()->getTILEMODE();
}

void OdExGripDbReactor::objectAppended(const OdDbDatabase* /*database*/, const OdDbObject* /*dbObject*/) noexcept {
	// New object.
}

void OdExGripDbReactor::objectModified(const OdDbDatabase*, const OdDbObject* dbObject) {
	m_GripManager->UpdateEntityGrips(dbObject->objectId());
	m_GripManager->UpdateInvisibleGrips();
}

void OdExGripDbReactor::objectErased(const OdDbDatabase* /*database*/, const OdDbObject* dbObject, const bool erased) {
	if (erased) {
		m_GripManager->RemoveEntityGrips(dbObject->objectId(), true);
		m_GripManager->UpdateInvisibleGrips();
	}
}
