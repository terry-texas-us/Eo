// From Examples\Editor\ExGripManager.cpp (last compare 19.12)

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
#if !defined(ODA_UNIXOS) //EMCC
#ifndef TPM_VERPOSANIMATION
static const UINT TPM_VERPOSANIMATION = 0x1000L;
#endif
#ifndef TPM_NOANIMATION
static const UINT TPM_NOANIMATION = 0x4000L;
#endif
#endif //#ifndef EMCC
//

#define GM_PAGE_EACH_OBJECT 200

namespace {

	static OdSelectionSetIteratorPtr searchObjectSSetIterator(OdSelectionSetPtr selectionSet, OdDbStub* id) {
		auto SelectionSetIterator {selectionSet->newIterator()};
		
		while (!SelectionSetIterator->done()) {
			if (SelectionSetIterator->id() == id) {
				return SelectionSetIterator;
			}
			SelectionSetIterator->next();
		}
		return OdSelectionSetIteratorPtr();
	}

	static OdBaseGripManager::OdExGripDataSubent& getSubentGripData(OdBaseGripManager::OdExGripDataExt& ext, OdDbBaseFullSubentPath entPath) {
		for (unsigned i = 0; i < ext.m_GripDataSubEntity.size(); i++) {
			if (ext.m_GripDataSubEntity.at(i).m_entPath == entPath)
				return ext.m_GripDataSubEntity.at(i);
		}
		ODA_FAIL();
		return ext.m_GripDataSubEntity.at(0);
	}

}

OdExGripDragPtr OdExGripDrag::createObject(OdDbStub* id, OdBaseGripManager* owner) {
	OdExGripDragPtr pRes = RXIMPL_CONSTR(OdExGripDrag);
	//pRes->m_entPath = OdDbBaseFullSubentPath();
	pRes->m_entPath.objectIds().append(id);
	pRes->m_pOwner = owner;
	return pRes;
}

OdExGripDragPtr OdExGripDrag::createObject(OdDbBaseFullSubentPath entPath, OdBaseGripManager* owner) {
	OdExGripDragPtr pRes = RXIMPL_CONSTR(OdExGripDrag);
	pRes->m_entPath = entPath;
	pRes->m_pOwner = owner;
	return pRes;
}

OdExGripDrag::OdExGripDrag() noexcept {
  //m_entPath = OdDbBaseFullSubentPath();
	m_pClone = 0;
	m_pOwner = 0;
}

OdExGripDrag::~OdExGripDrag() {
}

OdDbStub* OdExGripDrag::entityId() const {
	return m_entPath.objectIds().last();
}

bool OdExGripDrag::entPath(OdDbBaseFullSubentPath* pPath) const // = NULL
{
	if (pPath)
		* pPath = m_entPath;
	return m_entPath.subentId() != OdDbSubentId();
}

bool OdExGripDrag::locateActiveGrips(OdIntArray& indices) {
	const OdExGripDataPtrArray& GripData = (entPath()) ? getSubentGripData(m_pOwner->m_aGripData[entityId()], m_entPath).m_pSubData : m_pOwner->m_aGripData[entityId()].m_pDataArray;

	auto bExMethod {true};
	indices.clear();

	for (unsigned i = 0; i < GripData.size(); i++) {
		if (GripData[i]->GripData().isNull()) {
			bExMethod = false;
		}
		if (OdDbGripOperations::kDragImageGrip == GripData[i]->status()) {
			indices.push_back(i);
		}
	}
	ODA_ASSERT(GripData.size() == 0 || !indices.empty());
	return bExMethod;
}

void OdExGripDrag::CloneEntity() {
	m_pClone = m_pOwner ? m_pOwner->CloneEntity(entityId()) : OdGiDrawablePtr();
}

OdGiDrawablePtr OdExGripManager::CloneEntity(OdDbStub* id) {
	OdDbEntityPtr Entity = OdDbEntity::cast(OdDbObjectId(id).openObject());
	if (Entity.isNull())
		return OdGiDrawablePtr();

	OdDbEntityPtr Clone;
	if (Entity->cloneMeForDragging()) {
		Clone = OdDbEntity::cast(Entity->clone());
	}

	if (Clone.get()) {
		Clone->disableUndoRecording(true);
		Clone->setPropertiesFrom(Entity.get(), false);
	}

	return OdGiDrawable::cast(Clone);
}

void OdExGripDrag::CloneEntity(const OdGePoint3d & ptMoveAt) {
	CloneEntity();

	if (m_pClone.isNull()) { return; }

	OdIntArray aIndices;
	const auto bExMethod {locateActiveGrips(aIndices)};

	const auto vOffset {ptMoveAt - m_pOwner->m_ptBasePoint};

	if (bExMethod) {
		OdDbGripDataPtrArray aCloneData;
		
		if (entPath()) {
			m_pOwner->GetGripPointsAtSubentPath(m_pClone, m_entPath, aCloneData, m_pOwner->ActiveViewUnitSize(), m_pOwner->m_GRIPSIZE, m_pOwner->ActiveViewDirection(), 0);
		} else {
			m_pOwner->GetGripPoints(m_pClone, aCloneData, m_pOwner->ActiveViewUnitSize(), m_pOwner->m_GRIPSIZE, m_pOwner->ActiveViewDirection(), 0);
		}

		OdDbVoidPtrArray aIds;

		for (unsigned i = 0; i < aIndices.size(); i++) {
			
			if (aIndices[i] < (OdInt32) aCloneData.size()) {
				aIds.push_back(aCloneData[aIndices[i]]->appData());
			} else {
				ODA_ASSERT(0);
			}
		}
		for (unsigned i = 0; i < aCloneData.size(); ++i) {

			if (aCloneData[i]->gripOpStatFunc()) {
				(aCloneData[i]->gripOpStatFunc())(aCloneData[i], OdDbObjectId::kNull, OdDbGripOperations::kGripStart);
			}
		}

		if (entPath()) {
			OdDbBaseFullSubentPathArray aPaths;
			aPaths.append(m_entPath);
			m_pOwner->MoveGripPointsAtSubentPaths(m_pClone, aPaths, aIds, vOffset, 0);
			m_pOwner->SubentGripStatus(m_pClone, OdDb::kGripsToBeDeleted, m_entPath);
		} else {
			m_pOwner->MoveGripPointsAt(m_pClone, aIds, vOffset, 0);
			m_pOwner->GripStatus(m_pClone, OdDb::kGripsToBeDeleted);
		}
		for (unsigned i = 0; i < aCloneData.size(); ++i) {
			if (aCloneData[i]->gripOpStatFunc()) {
				(aCloneData[i]->gripOpStatFunc())(aCloneData[i], NULL, OdDbGripOperations::kGripEnd);
			}
		}
	} else {
		m_pOwner->MoveGripPointsAt(m_pClone, aIndices, vOffset);
		m_pOwner->GripStatus(m_pClone, OdDb::kGripsToBeDeleted);
	}
	m_pOwner->OnModified(this);
}
void OdExGripManager::OnModified(OdGiDrawable* grip) {
	if (GetGsModel()) {
		GetGsModel()->onModified(grip, (OdGiDrawable*)0);
	}
	else if (GetGsLayoutHelper()) {
		GetGsLayoutHelper()->invalidate();
	}
}

void OdExGripDrag::moveEntity(const OdGePoint3d & ptMoveAt) {
	OdIntArray aIndices;
	const bool bExMethod = locateActiveGrips(aIndices);

	const OdGeVector3d vOffset = ptMoveAt - m_pOwner->m_ptBasePoint;

	OdGiDrawablePtr Entity = m_pOwner->OpenObject(entityId(), OdDb::kForWrite);
	ODA_ASSERT(Entity.get());

	const OdExGripDataPtrArray& rData = (entPath()) ? getSubentGripData(m_pOwner->m_aGripData[entityId()], m_entPath).m_pSubData : m_pOwner->m_aGripData[entityId()].m_pDataArray;

	if (bExMethod) {
		OdDbVoidPtrArray aIds;
		const OdUInt32 iSize = aIndices.size();
		
		for (unsigned i = 0; i < iSize; i++) {
		
			if (aIndices[i] < (OdInt32) rData.size()) {
				aIds.push_back(rData[aIndices[i]]->GripData()->appData());
			} else {
				ODA_ASSERT(0);
			}
		}

		if (entPath()) {
			OdDbBaseFullSubentPathArray aPaths;
			aPaths.append(m_entPath);
			m_pOwner->MoveGripPointsAtSubentPaths(Entity, aPaths, aIds, vOffset, 0);
		} else {
			m_pOwner->MoveGripPointsAt(Entity, aIds, vOffset, 0);
		}
	} else {
	  //OdGePoint3dArray aPts;
	  //OdUInt32 iSize = rData.size();
	  //aPts.resize( iSize );
	  //for (unsigned i = 0; i < iSize; i++ )
	  //  aPts[ i ] = rData[ i ]->point();

	  //iSize = aIndices.size();
	  //for(unsigned i = 0; i < iSize; i++ )
	  //{
	  //  if ( aIndices[ i ] < (OdInt32)rData.size() ) 
	  //  {
	  //    aPts[ aIndices[ i ] ] += vOffset;
	  //  }
	  //  else
	  //  {
	  //    ODA_ASSERT( 0 );
	  //  }
	  //}

		m_pOwner->MoveGripPointsAt(Entity, aIndices, vOffset);
	}
}

void OdExGripDrag::notifyDragStarted() {
	if (!m_pOwner) { return; }

	OdGiDrawablePtr Entity {m_pOwner->OpenObject(entityId())};
	
	if (Entity.get()) {
		m_pOwner->DragStatus(Entity, OdDb::kDragStart);
	}
	m_pOwner->DraggingStarted();
}

void OdExGripDrag::notifyDragEnded() {
	if (!m_pOwner) { return; }
	
	OdGiDrawablePtr Entity = m_pOwner->OpenObject(entityId());
	
	if (Entity.get()) {
		m_pOwner->DragStatus(Entity, OdDb::kDragEnd);
	}
	m_pOwner->DraggingStopped();
}

void OdExGripDrag::notifyDragAborted() {
	if (!m_pOwner) { return; }

	OdGiDrawablePtr Entity = m_pOwner->OpenObject(entityId());

	if (Entity.get()) {
		m_pOwner->DragStatus(Entity, OdDb::kDragAbort);
	}
	m_pOwner->DraggingStopped();
}

OdUInt32 OdExGripDrag::subSetAttributes(OdGiDrawableTraits* drawableTraits) const {

	if (m_pClone.isNull()) { return kDrawableIsInvisible; }

	const OdUInt32 iRet = m_pClone->setAttributes(drawableTraits);

	OdGiSubEntityTraitsPtr pEntityTraits = OdGiSubEntityTraits::cast(drawableTraits);

	if (pEntityTraits.get()) {
		pEntityTraits->setFillType(kOdGiFillNever);
	}
	return iRet;
}

bool OdExGripDrag::subWorldDraw(OdGiWorldDraw* worldDraw) const {
	
	if (m_pClone.isNull()) { return true; }

	return m_pClone->worldDraw(worldDraw);
}

void OdExGripDrag::subViewportDraw(OdGiViewportDraw * viewportDraw) const {

	if (m_pClone.get()) {
		m_pClone->viewportDraw(viewportDraw);
	}
}

OdExGripDataPtr OdExGripData::createObject(OdDbStub * id, OdDbGripDataPtr gripData, const OdGePoint3d & point, OdBaseGripManager * pOwner) {
	OdExGripDataPtr pRes = RXIMPL_CONSTR(OdExGripData);
	//pRes->m_entPath = OdDbBaseFullSubentPath();
	pRes->m_entPath.objectIds().append(id);
	pRes->m_GripData = gripData;
	pRes->m_pOwner = pOwner;
	pRes->m_point = point;
	return pRes;
}

OdExGripDataPtr OdExGripData::createObject(OdDbBaseFullSubentPath entPath, OdDbGripDataPtr gripData, const OdGePoint3d & point, OdBaseGripManager * pOwner) {
	OdExGripDataPtr pRes = RXIMPL_CONSTR(OdExGripData);
	pRes->m_entPath = entPath;
	pRes->m_GripData = gripData;
	pRes->m_pOwner = pOwner;
	pRes->m_point = point;
	return pRes;
}

OdExGripData::OdExGripData() noexcept {
	m_Status = OdDbGripOperations::kWarmGrip;
	m_Invisible = false;
	m_Shared = false;
	m_point = OdGePoint3d::kOrigin;
	//m_entPath = OdDbBaseFullSubentPath();
	//m_GripData = 0;
	m_pOwner = 0;
}

OdExGripData::~OdExGripData() {
	if (m_GripData.get() && m_GripData->alternateBasePoint()) {
		delete m_GripData->alternateBasePoint();
		m_GripData->setAlternateBasePoint(0);
	}
}

bool OdExGripData::computeDragPoint(OdGePoint3d & ptOverride) const {
	OdGePoint3d ptBase = point();

	if (GripData().get() && GripData()->alternateBasePoint()) {
		ptBase = *(GripData()->alternateBasePoint());
	}
	bool bOverride = false;
	ptOverride = ptBase;

	if (status() == OdDbGripOperations::kDragImageGrip && GripData().get() && GripData()->drawAtDragImageGripPoint()) {
		ptOverride = ptBase + (m_pOwner->m_ptLastPoint - m_pOwner->m_ptBasePoint);
		bOverride = true;
	}
	return bOverride;
}

OdUInt32 OdExGripData::subSetAttributes(OdGiDrawableTraits* drawableTraits) const {

	if (isInvisible()) { return kDrawableIsInvisible; }

	OdGiSubEntityTraitsPtr pEntityTraits = OdGiSubEntityTraits::cast(drawableTraits);

	if (!pEntityTraits.get()) { return kDrawableNone; }

	switch (status()) {
		case OdDbGripOperations::kWarmGrip:
			pEntityTraits->setTrueColor(m_pOwner->m_GRIPCOLOR);
			break;

		case OdDbGripOperations::kHotGrip:
		case OdDbGripOperations::kDragImageGrip:
			pEntityTraits->setTrueColor(m_pOwner->m_GRIPHOT);
			break;

		case OdDbGripOperations::kHoverGrip:
			pEntityTraits->setTrueColor(m_pOwner->m_GRIPHOVER);
			break;
	}
	pEntityTraits->setMaterial(NULL);
	pEntityTraits->setLineWeight(OdDb::kLnWt000);
	return kDrawableRegenDraw;
}

bool OdExGripData::subWorldDraw(OdGiWorldDraw* worldDraw) const {
	double dGripSize = m_pOwner->m_GRIPSIZE;

	if (!worldDraw->context() || !worldDraw->context()->database()) {
		dGripSize = m_pOwner->m_GRIPSIZE;
	}

	// Here is the design flaw: ARX help says that grip size passed in callback below should be calculated individually for each viewport.

	if (GripData().get() && GripData()->worldDraw()) {
		OdGePoint3d ptComputed;
		OdGePoint3d* pDrawAtDrag = 0;

		if (computeDragPoint(ptComputed)) {
			pDrawAtDrag = &ptComputed;
		}
		OdGiDrawFlagsHelper _dfh(worldDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);
		
		return((*GripData()->worldDraw())((OdDbGripData*)GripData().get(), worldDraw, entityId(), status(), pDrawAtDrag, dGripSize));
	}
	return false;
}

void OdExGripData::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	OdGePoint3d ptComputed;
	OdGePoint3d* pDrawAtDrag = 0;
	
	if (computeDragPoint(ptComputed)) {
		pDrawAtDrag = &ptComputed;
	}
	OdGiDrawFlagsHelper _dfh(viewportDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);

	bool bDefault = true;
	
	if (GripData().get() && GripData()->viewportDraw()) {
		(*GripData()->viewportDraw())((OdDbGripData*)GripData().get(), viewportDraw, entityId(), status(), pDrawAtDrag, m_pOwner->m_GRIPSIZE);
		bDefault = false;
	}

	if (bDefault) {
	  //if (m_pOwner->GetGsModel() && m_pOwner->GetGsModel()->renderType() < OdGsModel::kDirect) {
	  //  // Commented since renderTypes implemented, so no need to translate objects for kDirect renderType
	  //  OdGeVector3d vpDirection(viewportDraw->viewport().viewDir());
	  //  OdGePoint3d vpOrigin(viewportDraw->viewport().getCameraLocation());
	  //  double ptLength = (ptComputed - vpOrigin).dotProduct(vpDirection);
	  //  ptComputed -= vpDirection * ptLength;
	  //}

		double dGripSize;
		{
			OdGePoint2d ptDim;
			viewportDraw->viewport().getNumPixelsInUnitSquare(point(), ptDim);
			OdGeVector3d v(m_pOwner->m_GRIPSIZE / ptDim.x, 0.0, 0.0);
			v.transformBy(viewportDraw->viewport().getWorldToEyeTransform());
			//if (viewportDraw->viewport().isPerspective())
			//{
			//  OdGePoint3d perspFix(v.length(), 0.0, OdGePoint3d(ptComputed).transformBy(viewportDraw->viewport().getWorldToEyeTransform()).z);
			//  viewportDraw->viewport().doInversePerspective(perspFix);
			//  v.x = perspFix.x;
			//  v.y = v.z = 0.0;
			//}
			dGripSize = v.length();
		}

		OdGePoint3d ptOnScreen = ptComputed; //+ 1E10 * viewportDraw->viewport().viewDir();
		ptOnScreen.transformBy(viewportDraw->viewport().getWorldToEyeTransform());

		viewportDraw->subEntityTraits().setFillType(kOdGiFillAlways);
		viewportDraw->subEntityTraits().setDrawFlags(OdGiSubEntityTraits::kDrawSolidFill | OdGiSubEntityTraits::kDrawPolygonFill);

		//ptOnScreen.z = 0.;

		OdGePoint3d aPoly[4];
		aPoly[0].set(ptOnScreen.x - dGripSize, ptOnScreen.y - dGripSize, ptOnScreen.z);
		aPoly[1].set(ptOnScreen.x + dGripSize, ptOnScreen.y - dGripSize, ptOnScreen.z);
		aPoly[2].set(ptOnScreen.x + dGripSize, ptOnScreen.y + dGripSize, ptOnScreen.z);
		aPoly[3].set(ptOnScreen.x - dGripSize, ptOnScreen.y + dGripSize, ptOnScreen.z);
		viewportDraw->geometry().polygonEye(4, aPoly);
	}
}

OdBaseGripManager::OdBaseGripManager() noexcept {
	m_aGripData.clear();
	m_HoverGripsData.clear();

	m_ptBasePoint = OdGePoint3d::kOrigin;
	m_ptLastPoint = OdGePoint3d::kOrigin;
	m_aDrags.clear();

	m_Disabled = true;

	m_GRIPSIZE = 5;
	m_GRIPOBJLIMIT = 100;
}

OdExGripManager::OdExGripManager() noexcept {
	m_LayoutHelper = 0;
	m_CommandContext = 0;
	m_pGsModel = 0;

	m_cDbReactor.m_pOwner = this;

	m_pGetSelectionSetPtr = 0;
}

OdBaseGripManager::~OdBaseGripManager() {
	EndHover();
}

OdExGripManager::~OdExGripManager() {
}

void OdExGripManager::Initialize(OdGsDevice* device, OdGsModel * gsModel, OdDbCommandContext * dbCommandContext, GetSelectionSetPtr pGetSSet) {
	m_LayoutHelper = device;
	m_pGsModel = gsModel;
	m_CommandContext = dbCommandContext;

	if (m_CommandContext->baseDatabase()) {
		OdDbDatabase* pDb = m_CommandContext->database();
		Disable(false);

		OdDbHostAppServices* pAppSvcs = pDb->appServices();
		m_GRIPSIZE = pAppSvcs->getGRIPSIZE();
		m_GRIPOBJLIMIT = pAppSvcs->getGRIPOBJLIMIT();
		m_GRIPCOLOR.setColorIndex(pAppSvcs->getGRIPCOLOR());
		m_GRIPHOVER.setColorIndex(pAppSvcs->getGRIPHOVER());
		m_GRIPHOT.setColorIndex(pAppSvcs->getGRIPHOT());
	}
	m_pGetSelectionSetPtr = pGetSSet;
	m_gripStretchCommand.m_parent = this;
}

void OdExGripManager::Uninitialize() {
	if (m_CommandContext) {
		Disable(true);
		m_CommandContext = 0;
	}
	m_LayoutHelper = 0;
}

void OdExGripManager::OdExGripCommand::execute(OdEdCommandContext * edCommandContext) {
	bool bOk = true;
	try {
		const OdGePoint3d ptFinal = m_parent->m_CommandContext->dbUserIO()->getPoint(L"Specify stretch point or [Base point/Copy/Undo/eXit]:", OdEd::kGptNoLimCheck | OdEd::kGptDefault | OdEd::kGptNoUCS, &m_parent->m_ptBasePoint, L"Base Copy Undo eXit", m_parent);

		for (unsigned i = 0; i < m_parent->m_aDrags.size(); i++) {
			m_parent->m_aDrags[i]->moveEntity(m_parent->EyeToUcsPlane(ptFinal, m_parent->m_ptBasePoint));
		}
	} catch (const OdEdCancel&) {
		bOk = false;
		for (unsigned i = 0; i < m_parent->m_aDrags.size(); i++)
			m_parent->m_aDrags[i]->notifyDragAborted();

	}

	for (unsigned i = 0; i < m_parent->m_aDrags.size(); i++) {
		if (bOk) {
			m_parent->m_aDrags[i]->notifyDragEnded();
			m_parent->UpdateEntityGrips(m_parent->m_aDrags[i]->entityId());
		} else {
			m_parent->m_aDrags[i]->notifyDragAborted();
		}
	}

	m_parent->m_aDrags.clear();

	if (bOk) {
		m_parent->UpdateInvisibleGrips();
	}
}

bool OdBaseGripManager::OnMouseDown(int x, int y, bool shiftIsDown) {
	EndHover();

	OdExGripDataPtrArray aKeys;
	LocateGripsAt(x, y, aKeys);

	if (aKeys.empty()) { return false; }

	if (shiftIsDown) { // Modify Grip  status().
		OdDbGripOperations::DrawType eNewStatus = OdDbGripOperations::kHotGrip;
		const OdUInt32 iSize = aKeys.size();

		for (unsigned i = 0; i < iSize; i++) {
			if (OdDbGripOperations::kHotGrip == aKeys[i]->status()) {
				eNewStatus = OdDbGripOperations::kWarmGrip;
				break;
			}
		}

		for (unsigned i = 0; i < iSize; i++) {
			OdDbGripOperations::DrawType eCurStatus = eNewStatus;
			OdExGripDataPtr pGrip = aKeys[i];
			if (!pGrip->GripData().isNull()) {
				if (pGrip->GripData()->triggerGrip()) {
					eCurStatus = OdDbGripOperations::kWarmGrip;
				} else {
					if (pGrip->GripData()->hotGripFunc()) {
						int iFlags = OdDbGripOperations::kMultiHotGrip;
						if (pGrip->isShared())
							iFlags |= OdDbGripOperations::kSharedGrip;

						OdResult eRet =
							(*pGrip->GripData()->hotGripFunc())(pGrip->GripData(), pGrip->entityId(), iFlags);
						switch (eRet) {
							case eGripOpGripHotToWarm:
								eCurStatus = OdDbGripOperations::kWarmGrip;
								break;
							default:
							  // no op
								break;
						}
					}
				}
			}
			aKeys[i]->setStatus(eCurStatus);
		}
	} else { // Launch Grip Edit.
		bool bMakeHot = true;
		{
			GripDataMap::const_iterator it = m_aGripData.begin();
			while ((it != m_aGripData.end()) && bMakeHot) {
				const OdExGripDataPtrArray& aData = it->second.m_pDataArray;

				for (unsigned i = 0; i < aData.size(); i++) {
					if (OdDbGripOperations::kHotGrip == aData[i]->status()) {
						bMakeHot = false;
						break;
					}
				}
				for (unsigned i = 0; (i < it->second.m_GripDataSubEntity.size()) && bMakeHot; i++) {
					const OdExGripDataPtrArray& aData = it->second.m_GripDataSubEntity.at(i).m_pSubData;

					for (OdUInt32 j = 0; j < aData.size(); j++) {
						if (OdDbGripOperations::kHotGrip == aData[j]->status()) {
							bMakeHot = false;
							break;
						}
					}
				}
				it++;
			}
		}
		bool bGetNew = false;
		OdDbObjectId idEntityToUpdate;

		if (bMakeHot) {
			const OdUInt32 iSize = aKeys.size();
			for (unsigned i = 0; i < iSize; i++) {
				OdExGripDataPtr pGrip = aKeys[i];

				auto eNew {OdDbGripOperations::kHotGrip};

				if (!pGrip->GripData().isNull() && pGrip->GripData()->hotGripFunc()) {
					int iFlags = 0;
					
					if (pGrip->isShared())
						iFlags |= OdDbGripOperations::kSharedGrip;

					if (pGrip->GripData()->triggerGrip()) {

						if (!pGrip->isShared()) {
							OdResult eRet = (*pGrip->GripData()->hotGripFunc())(pGrip->GripData(), pGrip->entityId(), iFlags);
							
							switch (eRet) {
								case eOk:
								case eGripOpGripHotToWarm:
									eNew = OdDbGripOperations::kWarmGrip;
									break;
								case eGripOpGetNewGripPoints:
									bGetNew = true;
									idEntityToUpdate = pGrip->entityId();
									break;
								default: // no op
									break;
							}
						}
					} else {
						OdResult eRet = (*pGrip->GripData()->hotGripFunc())(pGrip->GripData(), pGrip->entityId(), iFlags);

						if (!pGrip->isShared()) {
							switch (eRet) {
								case eGripOpGripHotToWarm:
									eNew = OdDbGripOperations::kWarmGrip;
									break;
								case eGripOpGetNewGripPoints:
									bGetNew = true;
									idEntityToUpdate = pGrip->entityId();
									break;
								default: // no op
									break;
							}
						}
					}
				}
				pGrip->setStatus(eNew);
			}
		}
		if (bGetNew) {
			UpdateEntityGrips(idEntityToUpdate);
			//return true;
		}
	}
	return true;
}
bool OdExGripManager::OnMouseDown(int x, int y, bool shiftIsDown) {
	if (!OdBaseGripManager::OnMouseDown(x, y, shiftIsDown)) { return false; }

	if (shiftIsDown) { return true; }

	OdExGripDataPtrArray aKeys;
	LocateGripsAt(x, y, aKeys);

	if (aKeys.empty()) { return true; }

	OdExGripDataPtrArray aActiveKeys;
	LocateGripsByStatus(OdDbGripOperations::kHotGrip, aActiveKeys);

	if (aActiveKeys.empty()) {
	  // Valid situation.
	  // If trigger grip performed entity modification and returned eGripHotToWarm
	  // then nothing is to be done cause entity modification will cause reactor to regen grips.
		return false;
	}

	if (handleMappedRtClk(aActiveKeys, x, y)) { return true; }

	OdUInt32 iSize = aActiveKeys.size();
	for (unsigned i = 0; i < iSize; i++)
		aActiveKeys[i]->setStatus(OdDbGripOperations::kDragImageGrip);

	GripDataMap::const_iterator it = m_aGripData.begin();
	while (it != m_aGripData.end()) {
		bool bActive = false;
		OdExGripDragPtr pDrag;
		{
			const OdExGripDataPtrArray& aData = it->second.m_pDataArray;

			for (unsigned i = 0; i < aData.size(); i++) {
				if (OdDbGripOperations::kDragImageGrip == aData[i]->status()) {
					bActive = true;
					pDrag = OdExGripDrag::createObject(it->first, this);
					break;
				}
			}
			for (unsigned i = 0; (i < it->second.m_GripDataSubEntity.size()) && !bActive; i++) {
				const OdExGripDataPtrArray& aData = it->second.m_GripDataSubEntity.at(i).m_pSubData;

				for (OdUInt32 j = 0; j < aData.size(); j++) {
					if (OdDbGripOperations::kDragImageGrip == aData[j]->status()) {
						bActive = true;
						pDrag = OdExGripDrag::createObject(it->second.m_GripDataSubEntity.at(i).m_entPath, this);
						break;
					}
				}
			}
		}
		if (bActive) {
		  //pDrag->entityId = it->first;
		  //pDrag->m_pOwner = this;
			m_aDrags.push_back(pDrag);
		}
		it++;
	}

	iSize = m_aDrags.size();
	for (unsigned i = 0; i < iSize; i++) {
		m_aDrags[i]->notifyDragStarted();
		m_aDrags[i]->CloneEntity();
	}

	m_ptBasePoint = aKeys.first()->point();
	m_ptLastPoint = m_ptBasePoint;
	{
	  // Use alternative point if needed.
		auto FirstGripData = aKeys.first()->GripData();
		
		if (0 != FirstGripData.get()) {
			
			if (0 != FirstGripData->alternateBasePoint()) {
				m_ptBasePoint = *(FirstGripData->alternateBasePoint());
			}
		}
	}
	m_CommandContext->database()->startUndoRecord();
	::odedRegCmds()->executeCommand(&m_gripStretchCommand, m_CommandContext);

	iSize = aActiveKeys.size();
	for (unsigned i = 0; i < iSize; i++)
		aActiveKeys[i]->setStatus(OdDbGripOperations::kWarmGrip);

	  //  } // of else of if ( bShift )
	  //}
	  //else
	  //{
	  //  return false;
	  //}

	return true;
}

bool OdBaseGripManager::StartHover(int x, int y) {
	bool bRet = EndHover();

	OdExGripDataPtrArray aKeys;
	LocateGripsAt(x, y, aKeys);
	
	if (!aKeys.empty()) {
		m_HoverGripsData = aKeys;

		for (unsigned i = 0; i < m_HoverGripsData.size(); i++) {
			auto HoverGripData {m_HoverGripsData[i]};

			if (HoverGripData->status() == OdDbGripOperations::kWarmGrip) {
				HoverGripData->setStatus(OdDbGripOperations::kHoverGrip);

				if (!HoverGripData->GripData().isNull()) {
					
					if (0 != HoverGripData->GripData()->hoverFunc()) {
						int iFlags = 0;
						if (HoverGripData->isShared()) {
							iFlags = OdDbGripOperations::kSharedGrip;
						}
						(*HoverGripData->GripData()->hoverFunc())(HoverGripData->GripData(), HoverGripData->entityId(), iFlags);
					}
				}
				OnModified(HoverGripData);
			}
		}
		bRet = true;
	}
	return bRet;
}

bool OdBaseGripManager::EndHover() {
	
	if (m_HoverGripsData.empty()) { return(false); }

	for (unsigned i = 0; i < m_HoverGripsData.size(); i++) {
		auto HoverGripData {m_HoverGripsData[i]};

		if (HoverGripData->status() == OdDbGripOperations::kHoverGrip) {
			HoverGripData->setStatus(OdDbGripOperations::kWarmGrip);
			OnModified(HoverGripData);
		}
	}
	m_HoverGripsData.clear();
	return(true);
}

bool OdExGripManager::OnMouseMove(int x, int y) {
	return StartHover(x, y);
}

bool OdExGripManager::onControlClick() {
	if (m_aDrags.empty())
		return false;

	  // TODO: Notify active grips.

	  // AEC grips use CTRL key to change mode,
	  // but how to pass it throw standard interface is currently unknown.

	return true;
}

void OdBaseGripManager::SelectionSetChanged(OdSelectionSet* selectionSet) {
	bool bRestoreOld = false;

	if (selectionSet->numEntities() > (unsigned)m_GRIPOBJLIMIT) {
		Disable(true);
	}
	else {
		if (IsDisabled()) {
			bRestoreOld = true;
		}
		Disable(false);
	}

	OdDbDatabase* pDb = OdDbDatabase::cast(selectionSet->baseDatabase()).get();

	{ // Old Entities.
		OdDbStubPtrArray aOld;
		GripDataMap::iterator GripDataIterator = m_aGripData.begin();
		
		while (GripDataIterator != m_aGripData.end()) {

			if (IsDisabled()) {
				aOld.push_back(GripDataIterator->first);
			} else {
				if (!selectionSet->isMember(GripDataIterator->first)) {
					aOld.push_back(GripDataIterator->first);
				} else {
				  // Remove if subentities changed
					bool bRemoved = false;
					OdUInt32 se;

					for (se = 0; se < GripDataIterator->second.m_GripDataSubEntity.size(); se++) {

						if (!selectionSet->isMember(GripDataIterator->second.m_GripDataSubEntity[se].m_entPath)) {
							aOld.push_back(GripDataIterator->first);
							bRemoved = true;
							break;
						}
					}
					// Remove if new paths added also (workaround. tehnically new pathes must be added on second step)
					if (!bRemoved) {
						auto SelectionSetIterator {searchObjectSSetIterator(selectionSet, GripDataIterator->first)};

						for (unsigned SubEntityIndex = 0; SubEntityIndex < SelectionSetIterator->subentCount(); SubEntityIndex++) {
							OdDbBaseFullSubentPath FullSubEntityPath;
							SelectionSetIterator->getSubentity(SubEntityIndex, FullSubEntityPath);
							OdUInt32 searchPath = 0;
							bool bFound = false;

							for (; searchPath < GripDataIterator->second.m_GripDataSubEntity.size(); searchPath++) {

								if (GripDataIterator->second.m_GripDataSubEntity.at(searchPath).m_entPath == FullSubEntityPath) {
									bFound = true;
									break;
								}
							}
							if (!bFound) {
								aOld.push_back(GripDataIterator->first);
								break;
							}
						}
					}
				}
			}
			GripDataIterator++;
		}
		const OdUInt32 iSize = aOld.size();

		for (unsigned i = 0; i < iSize; i++) {
			RemoveEntityGrips(aOld[i], true);
			
			if ((i % GM_PAGE_EACH_OBJECT) && pDb) {
				pDb->pageObjects();
			}
		}
	}
	{ // New Entities.

		OdDbStubPtrArray aNew;
		OdSelectionSetIteratorPtr pIter = selectionSet->newIterator();
		while (!pIter->done()) {
			if (!IsDisabled() && m_aGripData.end() == m_aGripData.find(pIter->id())) {
				aNew.push_back(pIter->id());
			}

			pIter->next();
		}
		const OdUInt32 iSize = aNew.size();
		for (unsigned i = 0; i < iSize; i++) {
			UpdateEntityGrips(aNew[i]);
			
			if ((i % GM_PAGE_EACH_OBJECT) && pDb) {
				pDb->pageObjects();
			}
		}
	}

	UpdateInvisibleGrips();
}

void OdBaseGripManager::UpdateEntityGrips(OdDbStub* id) {
	RemoveEntityGrips(id, false);

	OdSelectionSetPtr SelectionSet = WorkingSelectionSet();

	if (SelectionSet.isNull() || !SelectionSet->isMember(id)) { return; }

	OdGiDrawablePtr Entity = OpenObject(id);

	if (Entity.isNull()) { return; }

	OdExGripDataPtrArray aExt;
	OdDbGripDataPtrArray aPts;

	OdSelectionSetIteratorPtr pObjIt = searchObjectSSetIterator(SelectionSet, id);

	if (pObjIt->subentCount() > 0) {
		for (OdUInt32 se = 0; se < pObjIt->subentCount(); se++) {
			OdDbBaseFullSubentPath subEntPath;
			pObjIt->getSubentity(se, subEntPath);
			aPts.clear();
			if (GetGripPointsAtSubentPath(Entity, subEntPath, aPts, ActiveViewUnitSize(), m_GRIPSIZE, ActiveViewDirection(), 0) == eOk) {
				const OdUInt32 prevSize = aExt.size();
				aExt.resize(prevSize + aPts.size());
				for (unsigned i = 0; i < aPts.size(); i++) {
					aExt[i + prevSize] = OdExGripData::createObject(
						subEntPath, aPts[i], aPts[i]->gripPoint(), this);
				}
			}
		}
	} else {
		if (eOk == GetGripPoints(Entity, aPts, ActiveViewUnitSize(), m_GRIPSIZE, ActiveViewDirection(), 0)) {
			aExt.resize(aPts.size());
			const OdUInt32 iSize = aExt.size();
			for (unsigned i = 0; i < iSize; i++) {
				aExt[i] = OdExGripData::createObject(
					id, aPts[i], aPts[i]->gripPoint(), this);
			}
		} else {
			OdGePoint3dArray aOldPts;
			
			if (eOk == GetGripPoints(Entity, aOldPts)) {
				aExt.resize(aOldPts.size());
				const OdUInt32 iSize = aExt.size();
				for (unsigned i = 0; i < iSize; i++) {
					aExt[i] = OdExGripData::createObject(id, 0, aOldPts[i], this);
				}
			}
		}
	}

	const bool bModel = IsModel(Entity);
	
	if (!aExt.empty()) {
		const OdUInt32 iSize = aExt.size();
		OdExGripDataExt dExt;
		for (unsigned i = 0; i < iSize; i++) {
			OdDbBaseFullSubentPath entPath;
			if (aExt[i]->entPath(&entPath)) {
				bool bFound = false;
				for (OdUInt32 j = 0; j < dExt.m_GripDataSubEntity.size(); j++) {
					if (dExt.m_GripDataSubEntity[j].m_entPath == entPath) {
						bFound = true;
						dExt.m_GripDataSubEntity[j].m_pSubData.append(aExt[i]);
						break;
					}
				}
				if (!bFound) {
					OdExGripDataSubent se;
					se.m_entPath = entPath;
					se.m_pSubData.append(aExt[i]);
					dExt.m_GripDataSubEntity.append(se);
				}
			} else {
				dExt.m_pDataArray.append(aExt[i]);
			}
		}
		m_aGripData.insert(std::make_pair(id, dExt));

		for (unsigned i = 0; i < iSize; i++) {
			ShowGrip(aExt[i], bModel);
		}
	}
}

void OdBaseGripManager::RemoveEntityGrips(OdDbStub * id, bool bFireDone) {
	GripDataMap::iterator it = m_aGripData.find(id);
	
	if (it != m_aGripData.end()) {
		OdGiDrawablePtr Entity = OpenObject(id);
		
		if (Entity.get()) {
			GripStatus(Entity, OdDb::kGripsToBeDeleted);
		}
		const bool Model = IsModel(Entity);
		const OdUInt32 iSize = it->second.m_pDataArray.size();
		for (unsigned i = 0; i < iSize; i++) {
			OdExGripDataPtr GripData = it->second.m_pDataArray[i];
			HideGrip(GripData, Model);
			
			if (!it->second.m_pDataArray[i]->GripData().isNull() && it->second.m_pDataArray[i]->GripData()->gripOpStatFunc()) {
				(*it->second.m_pDataArray[i]->GripData()->gripOpStatFunc())(it->second.m_pDataArray[i]->GripData(), id, OdDbGripOperations::kGripEnd);
			}
			it->second.m_pDataArray[i] = 0;
		}
		for (unsigned i = 0; i < it->second.m_GripDataSubEntity.size(); i++) {
			for (OdUInt32 j = 0; j < it->second.m_GripDataSubEntity.at(i).m_pSubData.size(); j++) {
				OdExGripDataPtr GripData = it->second.m_GripDataSubEntity.at(i).m_pSubData[j];
				HideGrip(GripData, Model);
				it->second.m_GripDataSubEntity.at(i).m_pSubData[j] = 0;
			}
		}

		if (bFireDone) {
			if (Entity.get()) {
				GripStatus(Entity, OdDb::kGripsDone);
			}
		}
		m_aGripData.erase(it);
	}
}

void OdBaseGripManager::LocateGripsAt(int x, int y, OdExGripDataPtrArray & aResult) {
	aResult.clear();

	const double dX = x;
	const double dY = y;

	OdGePoint3d ptFirst;
	GripDataMap::const_iterator it = m_aGripData.begin();
	while (it != m_aGripData.end()) {
		for (OdUInt32 se = 0; se < it->second.m_GripDataSubEntity.size() + 1; se++) {
			const OdExGripDataPtrArray& aData = (se == 0) ? it->second.m_pDataArray : it->second.m_GripDataSubEntity[se - 1].m_pSubData;

			const OdUInt32 iSize = aData.size();
			for (unsigned i = 0; i < iSize; i++) {
				const OdGePoint3d& ptCurrent = aData[i]->point();

				if (aResult.empty()) {
				  // First grip is obtained by comparing grip point device position with cursor position.
					OdGePoint3d ptDC = ptCurrent;
					ptDC.transformBy(ActiveGsView()->worldToDeviceMatrix());

					const double dDeltaX = ::fabs(dX - ptDC.x);
					const double dDeltaY = ::fabs(dY - ptDC.y);
					const bool bOk = (dDeltaX <= m_GRIPSIZE) && (dDeltaY <= m_GRIPSIZE);
					if (bOk) {
						ptFirst = ptCurrent;
						aResult.push_back(aData[i]);
					}
				} else { // Other grips are obtained by comparing world coordinates. The approach here is quite raw.
					
					if (ptCurrent.isEqualTo(ptFirst, 1E-4)) {
						aResult.push_back(aData[i]);
					}
				}
			}
		}
		it++;
	}
}

void OdBaseGripManager::LocateGripsByStatus(OdDbGripOperations::DrawType eStatus, OdExGripDataPtrArray & aResult) {
	aResult.clear();

	GripDataMap::const_iterator it = m_aGripData.begin();
	while (it != m_aGripData.end()) {
		for (OdUInt32 se = 0; se < it->second.m_GripDataSubEntity.size() + 1; se++) {
			const OdExGripDataPtrArray& aData = (se == 0) ? it->second.m_pDataArray : it->second.m_GripDataSubEntity[se - 1].m_pSubData;
			const OdUInt32 iSize = aData.size();
			for (unsigned i = 0; i < iSize; i++) {
				if (eStatus == aData[i]->status()) {
					aResult.push_back(aData[i]);
				}
			}
		}
		it++;
	}
}

namespace {

	struct SortGripsAlongXAxis {
		bool operator()(const OdExGripDataPtr& grA, const OdExGripDataPtr& grB) {
			return OdPositive(grA->point().x, grB->point().x);
		}
	};

}

void OdBaseGripManager::UpdateInvisibleGrips() {
	OdExGripDataPtrArray aOverall;
	GripDataMap::const_iterator it = m_aGripData.begin();
	
	while (it != m_aGripData.end()) {
		aOverall.insert(aOverall.end(), it->second.m_pDataArray.begin(), it->second.m_pDataArray.end());
		for (unsigned i = 0; i < it->second.m_GripDataSubEntity.size(); i++) {
			aOverall.insert(aOverall.end(), it->second.m_GripDataSubEntity[i].m_pSubData.begin(), it->second.m_GripDataSubEntity[i].m_pSubData.end());
		}
		it++;
	}

	OdUInt32 iSize = aOverall.size();
	for (unsigned i = 0; i < iSize; i++) {
		aOverall[i]->setInvisible(false);
		aOverall[i]->setShared(false);
	}

	// Not the best approach for sorting.
	// Just for demonstration.
	std::sort(aOverall.begin(), aOverall.end(), SortGripsAlongXAxis());

	iSize = aOverall.size();
	for (unsigned i = 0; i < iSize; i++) {
		if (aOverall[i]->isShared())
			continue;

		OdUInt32Array aEq;
		aEq.push_back(i);

		const OdGePoint3d ptIni = aOverall[i]->point();

		OdUInt32 iNext = i + 1;
		while (iNext < iSize) {
			const OdGePoint3d ptCur = aOverall[iNext]->point();

			if (OdEqual(ptIni.x, ptCur.x, 1E-6)) {
				if (ptIni.isEqualTo(ptCur, 1E-6)) {
					aEq.push_back(iNext);
				}
				iNext++;
			} else {
				break;
			}
		}

		if (aEq.size() >= 2) {
			OdUInt32 iVisible = 0;
			const OdUInt32 jSize = aEq.size();
			for (OdUInt32 j = 0; j < jSize; j++) {
				OdExGripDataPtr pGrip = aOverall[aEq[j]];

				bool bOk = true;
				if (!pGrip->GripData().isNull()) {
					if (pGrip->GripData()->skipWhenShared())
						bOk = false;
				} else {
					bOk = false;
				}

				if (bOk) {
					iVisible = j;
					break;
				}
			}

			for (OdUInt32 j = 0; j < jSize; j++) {
				OdExGripDataPtr pGrip = aOverall[aEq[j]];

				pGrip->setShared(true);
				pGrip->setInvisible(j != iVisible);
			}
		}
	}
}

void OdExGripManager::ShowGrip(OdExGripData* pGrip, bool model) {
	OdGsPaperLayoutHelperPtr pPaperHelper = OdGsPaperLayoutHelper::cast(m_LayoutHelper);
	const OdUInt32 iSize = m_LayoutHelper->numViews();
	
	if (pPaperHelper.get()) {
	  //for( i = 0; i < iSize; i++ )
	  //  if(bModel==(pPaperHelper->viewAt(i) != pPaperHelper->overallView()))
	  //    pPaperHelper->viewAt( i )->add( pGrip, m_pGsModel );
		OdDbObjectPtr pVpObj = m_CommandContext->database()->activeViewportId().openObject();
		OdDbAbstractViewportDataPtr pAVD = OdDbAbstractViewportDataPtr(pVpObj);
	
		if (!pAVD.isNull() && pAVD->gsView(pVpObj)) {
			pAVD->gsView(pVpObj)->add(pGrip, m_pGsModel);
		} else {
			pPaperHelper->overallView()->add(pGrip, m_pGsModel);
		}
	} else {
		for (unsigned i = 0; i < iSize; i++) {
			m_LayoutHelper->viewAt(i)->add(pGrip, m_pGsModel);
		}
	}
}

void OdExGripManager::HideGrip(OdExGripData * grip, bool model) {
	OdGsPaperLayoutHelperPtr pPaperHelper = OdGsPaperLayoutHelper::cast(m_LayoutHelper);
	const OdUInt32 iSize = m_LayoutHelper->numViews();

	if (pPaperHelper.get()) {
		for (unsigned i = 0; i < iSize; i++)
			m_LayoutHelper->viewAt(i)->erase(grip);
	} else {
		for (unsigned i = 0; i < iSize; i++)
			m_LayoutHelper->viewAt(i)->erase(grip);
	}
}

void OdBaseGripManager::setValue(const OdGePoint3d & ptValue) {
	const OdUInt32 iSize = m_aDrags.size();
	const OdGePoint3d newPoint = EyeToUcsPlane(ptValue, m_ptBasePoint);
	for (unsigned i = 0; i < iSize; i++)
		m_aDrags[i]->CloneEntity(newPoint);
	m_ptLastPoint = newPoint;
}

int OdExGripManager::addDrawables(OdGsView* view) {
	ODA_ASSERT(view->device() == m_LayoutHelper->underlyingDevice().get());

	const OdUInt32 iSize = m_aDrags.size();

	for (unsigned i = 0; i < iSize; i++) {
		view->add(m_aDrags[i].get(), /*m_pGsModel*/ 0);
	}
	return iSize;
}

void OdExGripManager::removeDrawables(OdGsView* view) {
	const OdUInt32 iSize = m_aDrags.size();
	
	for (unsigned i = 0; i < iSize; i++) {
		view->erase(m_aDrags[i].get());
	}
}

inline void resetDragging(OdGsDevice* pDevice, bool bOp) {
	if (!pDevice) { return; }

	OdRxDictionaryPtr pProps = pDevice->properties();

	if (pProps.isNull()) { return; }

	if (!pProps->has(L"DrawDragging")) { return; }

	pProps->putAt(L"DrawDragging", OdRxVariantValue(bOp));
}

void OdExGripManager::DraggingStarted() {
	::resetDragging(m_LayoutHelper, true);
}

void OdExGripManager::DraggingStopped() {
	::resetDragging(m_LayoutHelper, false);
}

OdSelectionSetPtr OdExGripManager::WorkingSelectionSet() const {
	if (m_pGetSelectionSetPtr) {
		return OdSelectionSet::cast(m_pGetSelectionSetPtr(m_CommandContext));
	}
	return OdSelectionSetPtr();
}

OdGsView* OdExGripManager::ActiveGsView() const {
	return m_LayoutHelper->activeView();
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

	return (v.length() / m_GRIPSIZE);
}

OdGeVector3d OdBaseGripManager::ActiveViewDirection() const {
	auto View {ActiveGsView()};

	return (View->position() - View->target()).normal();
}

OdGePoint3d OdExGripManager::EyeToUcsPlane(const OdGePoint3d & pPoint, const OdGePoint3d & pBasePoint) const {

	OdDbObjectPtr pVpObj = OdDbObjectId(ActiveViewportId()).safeOpenObject();
	OdDbAbstractViewportDataPtr pAVD(pVpObj);
	OdGePoint3d ucsOrigin;
	OdGeVector3d ucsXAxis, ucsYAxis;
	pAVD->getUcs(pVpObj, ucsOrigin, ucsXAxis, ucsYAxis);
	OdGePlane plane(/*ucsOrigin*/pBasePoint, ucsXAxis, ucsYAxis);
	OdGeLine3d line(pPoint, ActiveViewDirection());
	OdGePoint3d newPoint;
	if (!plane.intersectWith(line, newPoint)) {
		line.set(pPoint, ucsXAxis.crossProduct(ucsYAxis));
		if (!plane.intersectWith(line, newPoint)) // #7727
			newPoint = pBasePoint;
	}
	return newPoint;
}

bool OdExGripManager::handleMappedRtClk(OdExGripDataPtrArray & aActiveKeys, int x, int y) {
#if !defined(ODA_UNIXOS) && !(defined(ODA_WINDOWS) && !defined(OD_WINDOWS_DESKTOP))
	const OdUInt32 iSize = aActiveKeys.size();
	int rtClkIndex = -1;
	for (unsigned i = 0; i < iSize; i++) {
		if (!aActiveKeys[i]->GripData().isNull() && 0 != aActiveKeys[i]->GripData()->rtClk()
			&& aActiveKeys[i]->GripData()->mapGripHotToRtClk() && !aActiveKeys[i]->isShared()) {
			rtClkIndex = i;
			break;
		}
	}
	if (rtClkIndex != -1) {
		OdDbStubPtrArray ents;
		OdDbGripDataArray hotGrips;
		for (unsigned i = 0; i < iSize; i++) {
			hotGrips.append(*aActiveKeys[i]->GripData());
			if (!ents.contains(aActiveKeys[i]->entityId()))
				ents.append(aActiveKeys[i]->entityId());
		}
		OdString menuName;
		ODHMENU menu = 0;
		ContextMenuItemIndexPtr cb = 0;
		OdResult eRet = (*aActiveKeys[rtClkIndex]->GripData()->rtClk())(hotGrips, ents, menuName, menu, cb);
		if (eRet == eOk && menu != 0 && cb != 0) {
			HWND wnd = ::GetActiveWindow();
			POINT pt = {x, y};
			::ClientToScreen(wnd, &pt);
			(*cb)(::TrackPopupMenu((HMENU) menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_NOANIMATION, pt.x, pt.y, 0, wnd, 0));
			::DestroyMenu((HMENU) menu);
			
			for (unsigned i = 0; i < iSize; i++) {
				aActiveKeys[i]->setStatus(OdDbGripOperations::kWarmGrip);
			}
			UpdateEntityGrips(aActiveKeys[rtClkIndex]->entityId());
			return true;
		}
	}
#endif // ODA_UNIXOS
	return false;
}

void OdBaseGripManager::Disable(bool disable) noexcept {
	m_Disabled = disable;
}

void OdExGripManager::Disable(bool disable) noexcept {
	if (m_Disabled != disable) {
		OdDbDatabase* pDb = m_CommandContext->database();
		m_Disabled = disable;
		
		if (disable) {
			pDb->removeReactor(&m_cDbReactor);
		}
		else {
			pDb->addReactor(&m_cDbReactor);
		}
	}
}

OdGiDrawablePtr OdExGripManager::OpenObject(OdDbStub * id,
	bool isForWriteMode) // = false
{
	OdGiDrawablePtr pObj;
	if (!id)
		return pObj;
	pObj = OdGiDrawable::cast(OdDbObjectId(id).openObject(isForWriteMode ? OdDb::kForWrite : OdDb::kForRead));
	return pObj;
}

OdResult OdExGripManager::GetGripPointsAtSubentPath(OdGiDrawable* entity, const OdDbBaseFullSubentPath& path, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, const OdUInt32 bitflags) const {
	return OdDbEntity::cast(entity)->getGripPointsAtSubentPath(*((const OdDbFullSubentPath*)& path), grips, curViewUnitSize, gripSize, curViewDir, bitflags);
}

OdResult OdExGripManager::GetGripPoints(OdGiDrawable* entity, OdDbGripDataPtrArray& grips, double curViewUnitSize, int gripSize, const OdGeVector3d& curViewDir, int bitFlags) const {
	OdDbEntity* pEnt = OdDbEntity::cast(entity);

	if (!pEnt) { return eNotApplicable; }

	return pEnt->getGripPoints(grips, curViewUnitSize, gripSize, curViewDir, bitFlags);
}

OdResult OdExGripManager::GetGripPoints(OdGiDrawable* entity, OdGePoint3dArray& gripPoints) const {
	OdDbEntity* pEnt = OdDbEntity::cast(entity);

	if (!pEnt) { return eNotApplicable; }

	return pEnt->getGripPoints(gripPoints);
}

OdResult OdExGripManager::MoveGripPointsAtSubentPaths(OdGiDrawable* entity, const OdDbBaseFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, OdUInt32 bitflags) {
	ODA_ASSERT_ONCE(sizeof(OdDbFullSubentPath) == sizeof(OdDbBaseFullSubentPath));
	return OdDbEntity::cast(entity)->moveGripPointsAtSubentPaths(*((const OdDbFullSubentPathArray*)& paths), gripAppData, offset, bitflags);
}

OdResult OdExGripManager::MoveGripPointsAt(OdGiDrawable* entity, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, int bitFlags) {
	return OdDbEntity::cast(entity)->moveGripPointsAt(gripAppData, offset, bitFlags);
}

OdResult OdExGripManager::MoveGripPointsAt(OdGiDrawable* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return OdDbEntity::cast(entity)->moveGripPointsAt(indices, offset);
}

void OdExGripManager::SubentGripStatus(OdGiDrawable* entity, OdDb::GripStat status, const OdDbBaseFullSubentPath& subentity) {
	OdDbEntity::cast(entity)->subentGripStatus(status, *((const OdDbFullSubentPath*)& subentity));
}

void OdExGripManager::GripStatus(OdGiDrawable* entity, OdDb::GripStat st) {
	OdDbEntity::cast(entity)->gripStatus(st);
}

void OdExGripManager::DragStatus(OdGiDrawable* entity, OdDb::DragStat st) {
	OdDbEntity::cast(entity)->dragStatus(st);
}

bool OdExGripManager::IsModel(OdGiDrawable* entity) noexcept {
	OdDbEntity* pEnt = OdDbEntity::cast(entity);
	return !pEnt || pEnt->database()->getTILEMODE();
}

OdExGripDbReactor::OdExGripDbReactor() noexcept
	: m_pOwner(0) {
}

void OdExGripDbReactor::objectAppended(const OdDbDatabase*, const OdDbObject*) noexcept {
  // New object.
}

void OdExGripDbReactor::objectModified(const OdDbDatabase*, const OdDbObject * pDbObj) {
	m_pOwner->UpdateEntityGrips(pDbObj->objectId());
	m_pOwner->UpdateInvisibleGrips();
}

void OdExGripDbReactor::objectErased(const OdDbDatabase*, const OdDbObject * pDbObj, bool pErased) {
	if (pErased) {
		m_pOwner->RemoveEntityGrips(pDbObj->objectId(), true);
		m_pOwner->UpdateInvisibleGrips();
	}
}

#undef GM_PAGE_EACH_OBJECT
