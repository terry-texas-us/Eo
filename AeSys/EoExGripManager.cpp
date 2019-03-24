#include "Stdafx.h"

#include "OdaCommon.h"
#define STL_USING_MAP
#define STL_USING_ALGORITHM
#include "OdaSTL.h"
#include "UInt32Array.h"
#include "Gi/GiDrawableImpl.h"
#include "Gi/GiWorldDraw.h"
#include "Gi/GiViewportDraw.h"
#include "DbHostAppServices.h"
#include "DbCommandContext.h"
#include "DbEntity.h"
#include "DbAbstractViewportData.h"
#include "EoExGripManager.h"
#include "Gs/GsModel.h"

// Menu animation flags
#ifndef TPM_VERPOSANIMATION
static const UINT TPM_VERPOSANIMATION = 0x1000L;
#endif
#ifndef TPM_NOANIMATION
static const UINT TPM_NOANIMATION = 0x4000L;
#endif
//

#define GM_PAGE_EACH_OBJECT 200

namespace {
	static OdDbSelectionSetIteratorPtr searchObjectSSetIterator(OdDbSelectionSetPtr selectionSet, const OdDbObjectId& id) {
		OdDbSelectionSetIteratorPtr pIter = selectionSet->newIterator();
		while (!pIter->done()) {
			if (pIter->objectId() == id) {
				return pIter;
			}
			pIter->next();
		}
		return OdDbSelectionSetIteratorPtr();
	}
	static EoExGripManager::EoExGripDataSubent &getSubentGripData(EoExGripManager::EoExGripDataExt &ext, OdDbFullSubentPath entPath) {
		for (OdUInt32 i = 0; i < ext.m_pDataSub.size(); i++) {
			if (ext.m_pDataSub.at(i).m_entPath == entPath)
				return ext.m_pDataSub.at(i);
		}
		ODA_FAIL();
		return ext.m_pDataSub.at(0);
	}
}
EoExGripDragPtr EoExGripDrag::createObject(OdDbObjectId id, EoExGripManager* gripManager) {
	EoExGripDragPtr pRes = OdRxObjectImpl<EoExGripDrag>::createObject();
	pRes->m_entPath = OdDbFullSubentPath();
	pRes->m_entPath.objectIds().append(id);
	pRes->m_pOwner = gripManager;
	return pRes;
}
EoExGripDragPtr EoExGripDrag::createObject(OdDbFullSubentPath entPath, EoExGripManager* gripManager) {
	EoExGripDragPtr pRes = OdRxObjectImpl<EoExGripDrag>::createObject();
	pRes->m_entPath = entPath;
	pRes->m_pOwner = gripManager;
	return pRes;
}
EoExGripDrag::EoExGripDrag() {
	m_entPath = OdDbFullSubentPath();
	m_pClone = 0;
	m_pOwner = 0;
}
EoExGripDrag::~EoExGripDrag() {
}
bool EoExGripDrag::locateActiveGrips(OdIntArray& aIndices) {
	const EoExGripDataPtrArray& rData = (entPath()) ? getSubentGripData(m_pOwner->m_aGripData[entityId()], m_entPath).m_pSubData : m_pOwner->m_aGripData[ entityId() ].m_pDataArray;

	bool bExMethod = true;
	aIndices.clear();
	OdUInt32 i, iSize = rData.size();
	for (i = 0; i < iSize; i++) {
		if (rData[i]->data().isNull())
			bExMethod = false;

		if (OdDbGripOperations::kDragImageGrip == rData[i]->status())
			aIndices.push_back(i);
	}
	ODA_ASSERT(!aIndices.empty());
	return bExMethod;
}
void EoExGripDrag::cloneEntity() {
	m_pClone = 0;

	OdDbEntityPtr pEntity = OdDbEntity::cast(entityId().openObject());
	if (pEntity.isNull() || 0 == m_pOwner) {
		return;
	}

	if (pEntity->cloneMeForDragging()) {
		m_pClone = OdDbEntity::cast(pEntity->clone());
	}
	if (false == m_pClone.isNull()) {
		m_pClone->disableUndoRecording(true);
		m_pClone->setPropertiesFrom(pEntity.get(), false);
	}
}
void EoExGripDrag::cloneEntity(const OdGePoint3d& ptMoveAt) {
	cloneEntity();
	if (m_pClone.isNull())
		return;

	OdIntArray aIndices;
	bool bExMethod = locateActiveGrips(aIndices);

	OdGeVector3d vOffset =
		ptMoveAt - m_pOwner->m_ptBasePoint;

	if (bExMethod) {
		OdDbGripDataPtrArray aCloneData;
		double ActiveViewUnitSize(m_pOwner->activeViewUnitSize());
		OdGeVector3d ActiveViewDirection(m_pOwner->activeViewDirection());
		if (entPath()) {
			m_pClone->getGripPointsAtSubentPath(m_entPath, aCloneData, ActiveViewUnitSize, m_pOwner->m_GRIPSIZE, ActiveViewDirection, 0);
		}
		else {
			m_pClone->getGripPoints(aCloneData, ActiveViewUnitSize, m_pOwner->m_GRIPSIZE, ActiveViewDirection, 0);
		}
		OdDbVoidPtrArray aIds;
		OdUInt32 i, iSize = aIndices.size();
		for (i = 0; i < iSize; i++) {
			if (aIndices[i] < (OdInt32)aCloneData.size()) {
				aIds.push_back(aCloneData[ aIndices[i] ]->appData());
			}
			else {
				ODA_ASSERT(0);
			}
		}
		if (entPath()) {
			OdDbFullSubentPathArray aPaths;
			aPaths.append(m_entPath);
			m_pClone->moveGripPointsAtSubentPaths(aPaths, aIds, vOffset, 0);
			m_pClone->subentGripStatus(OdDb::kGripsToBeDeleted, m_entPath);
		}
		else {
			m_pClone->moveGripPointsAt(aIds, vOffset, 0);
			m_pClone->gripStatus(OdDb::kGripsToBeDeleted);
			//m_pClone->gripStatus(OdDb::kDimDataToBeDeleted);
		}
		for (i = 0; i < aCloneData.size(); ++i)
			if (aCloneData[i]->gripOpStatFunc())
				(aCloneData[i]->gripOpStatFunc())(aCloneData[i], OdDbObjectId::kNull, OdDbGripOperations::kGripEnd);
	}
	else {
		//OdGePoint3dArray aPts;
		//m_pClone->getGripPoints(aPts);

		//OdUInt32 i, iSize = aIndices.size();
		//for (i = 0; i < iSize; i++) {
		//	if (aIndices[i] < (OdInt32)aPts.size())  {
		//		aPts[ aIndices[i] ] += vOffset;
		//	}
		//	else {
		//		ODA_ASSERT(0);
		//	}
		//}

		m_pClone->moveGripPointsAt(aIndices, vOffset);
		m_pClone->gripStatus(OdDb::kGripsToBeDeleted);
	}
	if (m_pOwner->m_pGsModel)
		m_pOwner->m_pGsModel->onModified(this, (OdGiDrawable*)0);
	else
		m_pOwner->m_pDevice->invalidate();
}
void EoExGripDrag::moveEntity(const OdGePoint3d& ptMoveAt) {
	OdIntArray aIndices;
	bool bExMethod = locateActiveGrips(aIndices);

	OdGeVector3d vOffset =
		ptMoveAt - m_pOwner->m_ptBasePoint;

	OdDbEntityPtr pEntity =
		OdDbEntity::cast(entityId().openObject(OdDb::kForWrite));
	ODA_ASSERT(!pEntity.isNull());

	const EoExGripDataPtrArray& rData = (entPath()) ?
		getSubentGripData(m_pOwner->m_aGripData[entityId()], m_entPath).m_pSubData
		:
		m_pOwner->m_aGripData[ entityId() ].m_pDataArray;

	if (bExMethod) {
		OdDbVoidPtrArray aIds;
		OdUInt32 i, iSize = aIndices.size();
		for (i = 0; i < iSize; i++) {
			if (aIndices[i] < (OdInt32)rData.size()) {
				aIds.push_back(rData[ aIndices[i] ]->data()->appData());
			}
			else {
				ODA_ASSERT(0);
			}
		}

		if (entPath()) {
			OdDbFullSubentPathArray aPaths;
			aPaths.append(m_entPath);
			pEntity->moveGripPointsAtSubentPaths(aPaths, aIds, vOffset, 0);
		}
		else {
			pEntity->moveGripPointsAt(aIds, vOffset, 0);
		}
	}
	else {
		//OdGePoint3dArray aPts;
		//OdUInt32 i, iSize = rData.size();
		//aPts.resize(iSize);
		//for (i = 0; i < iSize; i++)
		//  aPts[i] = rData[i]->point();

		//iSize = aIndices.size();
		//for (i = 0; i < iSize; i++) {
		//	if (aIndices[i] < (OdInt32)rData.size()) {
		//		aPts[ aIndices[i] ] += vOffset;
		//	}
		//	else {
		//		ODA_ASSERT(0);
		//	}
		//}
		pEntity->moveGripPointsAt(aIndices, vOffset);
	}
}
void EoExGripDrag::notifyDragStarted() {
	OdDbEntityPtr pEntity =
		OdDbEntity::cast(entityId().openObject());
	if (false == pEntity.isNull())
		pEntity->dragStatus(OdDb::kDragStart);
}
void EoExGripDrag::notifyDragEnded() {
	OdDbEntityPtr pEntity =
		OdDbEntity::cast(entityId().openObject());
	if (false == pEntity.isNull())
		pEntity->dragStatus(OdDb::kDragEnd);
}
void EoExGripDrag::notifyDragAborted() {
	OdDbEntityPtr pEntity =
		OdDbEntity::cast(entityId().openObject());
	if (false == pEntity.isNull())
		pEntity->dragStatus(OdDb::kDragAbort);
}
OdDbObjectId EoExGripDrag::entityId() const {
	return m_entPath.objectIds().last();
}
bool EoExGripDrag::entPath(OdDbFullSubentPath* path) const {
	if (path) {
		*path = m_entPath;
	}
	return m_entPath.subentId() != OdDbSubentId();
}
OdUInt32 EoExGripDrag::subSetAttributes(OdGiDrawableTraits* traits) const {
	if (m_pClone.isNull()) {
		return(kDrawableIsInvisible);
	}
	else {
		OdUInt32 iRet = m_pClone->setAttributes(traits);

		OdGiSubEntityTraitsPtr pEntityTraits =
			OdGiSubEntityTraits::cast(traits);
		if (pEntityTraits.isNull()) {
			ODA_ASSERT(0);
			return(OdGiDrawable::kDrawableNone);
		}
		pEntityTraits->setFillType(kOdGiFillNever);

		return iRet;
	}
}
bool EoExGripDrag::subWorldDraw(OdGiWorldDraw* worldDraw) const {
	if (m_pClone.isNull()) {
		return true;
	}
	else {
		return m_pClone->worldDraw(worldDraw);
	}
}
void EoExGripDrag::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	if (false == m_pClone.isNull()) {
		m_pClone->viewportDraw(viewportDraw);
	}
}

EoExGripDataPtr EoExGripData::createObject(OdDbObjectId id, OdDbGripDataPtr gripData, const OdGePoint3d& point,  EoExGripManager* gripManager) { 
	EoExGripDataPtr pRes = OdRxObjectImpl<EoExGripData>::createObject();
	pRes->m_entPath = OdDbFullSubentPath();
	pRes->m_entPath.objectIds().append(id);
	pRes->m_pData = gripData;
	pRes->m_pOwner = gripManager;
	pRes->m_point = point;
	return pRes;
}
EoExGripDataPtr EoExGripData::createObject(OdDbFullSubentPath fullSentPath, OdDbGripDataPtr gripData, const OdGePoint3d& point,  EoExGripManager* gripManager) { 
	EoExGripDataPtr pRes = OdRxObjectImpl<EoExGripData>::createObject();
	pRes->m_entPath = fullSentPath;
	pRes->m_pData = gripData;
	pRes->m_pOwner = gripManager;
	pRes->m_point = point;
	return pRes;
}
EoExGripData::EoExGripData() {
	m_status = OdDbGripOperations::kWarmGrip;
	m_bInvisible = false;
	m_bShared = false;
	m_point = OdGePoint3d::kOrigin;
	m_entPath = OdDbFullSubentPath();
	m_pData = 0;
	m_pOwner = 0;
}
EoExGripData::~EoExGripData() {
	if(m_pData.get() && m_pData->alternateBasePoint())
	{
		delete m_pData->alternateBasePoint();
		m_pData->setAlternateBasePoint(0);
	}
}
bool EoExGripData::computeDragPoint(OdGePoint3d& ptOverride) const {
	OdGePoint3d ptBase = point();
	if (!data().isNull() && 0 != data()->alternateBasePoint()) {
		ptBase = *(data()->alternateBasePoint());
	}

	bool bOverride = false;
	ptOverride = ptBase;

	if (status() == OdDbGripOperations::kDragImageGrip) {
		if (!data().isNull()) {
			if (data()->drawAtDragImageGripPoint()) {
				ptOverride = ptBase + (m_pOwner->m_ptLastPoint - m_pOwner->m_ptBasePoint);
				bOverride = true;
			}
		}
	}
	return bOverride;
}
OdUInt32 EoExGripData::subSetAttributes(OdGiDrawableTraits* traits) const {
	if (isInvisible())
		return kDrawableIsInvisible;

	OdGiSubEntityTraitsPtr pEntityTraits = OdGiSubEntityTraits::cast(traits);
	if (!pEntityTraits.get())
		return kDrawableNone;

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
	pEntityTraits->setMaterial(NULL/*entityId().database()->byBlockMaterialId()*/);
	pEntityTraits->setLineWeight(OdDb::kLnWt000);
	return kDrawableRegenDraw;
}
bool EoExGripData::subWorldDraw(OdGiWorldDraw* worldDraw) const {
	double GripSize = m_pOwner->m_GRIPSIZE; 
	{
		if (0 == worldDraw->context() || 0 == worldDraw->context()->database()) {
			GripSize = m_pOwner->m_GRIPSIZE;
		}
		else {
			// Here is the design flaw:
			// ARX help says that grip size passed in callback below should be calculated individually for each viewport.
		}
	}

	if (!data().isNull()) {
		if (0 != data()->worldDraw()) {
			OdGePoint3d ptComputed;
			OdGePoint3d* pDrawAtDrag = 0;
			if (computeDragPoint(ptComputed))
				pDrawAtDrag = &ptComputed;

			OdGiDrawFlagsHelper _dfh(worldDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);
			return ((*data()->worldDraw()) ((OdDbGripData*)data().get(), worldDraw, entityId(),  status(), pDrawAtDrag, GripSize));
		}
	}
	return (false);
}
void EoExGripData::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	OdGePoint3d ptComputed;
	OdGePoint3d* pDrawAtDrag = 0;
	if (computeDragPoint(ptComputed))
		pDrawAtDrag = &ptComputed;
	OdGiDrawFlagsHelper _dfh(viewportDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);

	bool bDefault = true;
	if (!data().isNull()) {
		if (0 != data()->viewportDraw()) {
			(*data()->viewportDraw())((OdDbGripData*)data().get(), viewportDraw, entityId(),  status(), pDrawAtDrag, m_pOwner->m_GRIPSIZE);
			bDefault = false;
		}
	}
	if (bDefault) {
		if (!m_pOwner->m_pGsModel || m_pOwner->m_pGsModel->renderType() < OdGsModel::kDirect) {
			// Commented since renderTypes implemented, so no need to translate objects for kDirect renderType
			OdGeVector3d vpDirection(viewportDraw->viewport().viewDir());
			OdGePoint3d vpOrigin(viewportDraw->viewport().getCameraLocation());
			double ptLength = (ptComputed - vpOrigin).dotProduct(vpDirection);
			ptComputed -= vpDirection * ptLength;
		}
		double dGripSize;
		{
			OdGePoint2d ptDim;
			viewportDraw->viewport().getNumPixelsInUnitSquare(point(), ptDim);
			OdGeVector3d v(m_pOwner->m_GRIPSIZE / ptDim.x, 0.0, 0.0);
			v.transformBy(viewportDraw->viewport().getWorldToEyeTransform());
			//if (pViewportDraw->viewport().isPerspective())
			//{
			//  OdGePoint3d perspFix(v.length(), 0.0, OdGePoint3d(ptComputed).transformBy(pViewportDraw->viewport().getWorldToEyeTransform()).z);
			//  pViewportDraw->viewport().doInversePerspective(perspFix);
			//  v.x = perspFix.x;
			//  v.y = v.z = 0.0;
			//}
			dGripSize = v.length();
		}

		OdGePoint3d ptOnScreen = ptComputed; //+ 1E10 * pViewportDraw->viewport().viewDir();
		ptOnScreen.transformBy(viewportDraw->viewport().getWorldToEyeTransform());

		viewportDraw->subEntityTraits().setFillType(kOdGiFillAlways);

		//ptOnScreen.z = 0.;

		OdGePoint3d aPoly[ 4 ];
		aPoly[ 0 ].set(ptOnScreen.x - dGripSize, ptOnScreen.y - dGripSize, ptOnScreen.z);
		aPoly[ 1 ].set(ptOnScreen.x + dGripSize, ptOnScreen.y - dGripSize, ptOnScreen.z);
		aPoly[ 2 ].set(ptOnScreen.x + dGripSize, ptOnScreen.y + dGripSize, ptOnScreen.z);
		aPoly[ 3 ].set(ptOnScreen.x - dGripSize, ptOnScreen.y + dGripSize, ptOnScreen.z);
		viewportDraw->geometry().polygonEye(4, aPoly);
	}
}
OdDbGripOperations::DrawType EoExGripData::status() const {
	return m_status;
}
bool EoExGripData::isInvisible() const {
	return m_bInvisible;
}
bool EoExGripData::isShared() const {
	return m_bShared;
}
OdGePoint3d EoExGripData::point() const {
	return m_point;
}
OdDbGripDataPtr EoExGripData::data() const {
	return m_pData;
}
OdDbObjectId EoExGripData::entityId() const {
	return m_entPath.objectIds().last();
}
bool EoExGripData::entPath(OdDbFullSubentPath* path) const {
	if (path) {
		*path = m_entPath;
	}
	return m_entPath.subentId() != OdDbSubentId();
}
void EoExGripData::setStatus(OdDbGripOperations::DrawType val) {
	m_status = val;
}
void EoExGripData::setInvisible(bool val) {
	m_bInvisible = val;
}
void EoExGripData::setShared(bool val) {
	m_bShared = val;
}

EoExGripManager::EoExGripManager() {
	m_pDevice = 0;
	m_pCmdCtx = 0;
	m_aGripData.clear();

	m_aHoverGrips.clear();

	m_ptBasePoint = OdGePoint3d::kOrigin;
	m_ptLastPoint = OdGePoint3d::kOrigin;
	m_aDrags.clear();

	m_cDbReactor.m_pOwner = this;

	m_bDisabled = true;

	m_GRIPSIZE = 5;
	m_GRIPOBJLIMIT = 100;

	m_pGetSelectionSetPtr = 0;
}
EoExGripManager::~EoExGripManager() {
	endHover();
}
void EoExGripManager::init(OdGsDevice* device, OdGsModel* gsModel, OdDbCommandContext* commandContext, GetSelectionSetPtr selectionSet) {
	m_pDevice = device;
	m_pGsModel = gsModel;
	m_pCmdCtx = commandContext;

	OdDbDatabase* pDb = m_pCmdCtx->database();
	disable(false);

	OdDbHostAppServices* pAppSvcs = pDb->appServices();
	m_GRIPSIZE = pAppSvcs->getGRIPSIZE();
	m_GRIPOBJLIMIT = pAppSvcs->getGRIPOBJLIMIT();
	m_GRIPCOLOR.setColorIndex(pAppSvcs->getGRIPCOLOR());
	m_GRIPHOVER.setColorIndex(pAppSvcs->getGRIPHOVER());
	m_GRIPHOT.setColorIndex(pAppSvcs->getGRIPHOT());

	m_pGetSelectionSetPtr = selectionSet;
}
void EoExGripManager::uninit() {
	if (m_pCmdCtx) {
		disable(true);
		m_pCmdCtx = 0;
	}
	m_pDevice = 0;
}
bool EoExGripManager::onMouseDown(int x, int y, bool shiftIsDown) {
	endHover();

	EoExGripDataPtrArray aKeys;
	locateGripsAt(x, y, aKeys);

	if (false == aKeys.empty()) {
		if (shiftIsDown) {
			// Modify Grip Status().
			OdDbGripOperations::DrawType eNewStatus = OdDbGripOperations::kHotGrip;
			OdUInt32 i, iSize = aKeys.size();
			for (i = 0; i < iSize; i++) {
				if (OdDbGripOperations::kHotGrip == aKeys[i]->status()) {
					eNewStatus = OdDbGripOperations::kWarmGrip;
					break;
				}
			}
			for (i = 0; i < iSize; i++) {
				OdDbGripOperations::DrawType eCurStatus = eNewStatus;
				EoExGripDataPtr pGrip = aKeys[i];
				if (!pGrip->data().isNull()) {
					if (pGrip->data()->triggerGrip()) {
						eCurStatus = OdDbGripOperations::kWarmGrip;
					}
					else {
						if (0 != pGrip->data()->hotGripFunc()) {
							int iFlags = OdDbGripOperations::kMultiHotGrip;
							if (pGrip->isShared())
								iFlags |= OdDbGripOperations::kSharedGrip;

							OdResult eRet =
								(*pGrip->data()->hotGripFunc())(pGrip->data(), pGrip->entityId(), iFlags);
							switch(eRet) {
							case eGripOpGripHotToWarm : {
									eCurStatus = OdDbGripOperations::kWarmGrip;
									break;
								}
							}
						}
					}
				}
				aKeys[i]->setStatus(eCurStatus);
			}
		}
		else {
			// Launch Grip Edit.
			bool bMakeHot = true;
			{
				GripDataMap::const_iterator it = m_aGripData.begin();
				while ((it != m_aGripData.end()) && bMakeHot) {
					const EoExGripDataPtrArray& aData = it->second.m_pDataArray;

					OdUInt32 i, iSize = aData.size();
					for (i = 0; i < iSize; i++) {
						if (OdDbGripOperations::kHotGrip == aData[i]->status()) {
							bMakeHot = false;
							break;
						}
					}
					for (i = 0; (i < it->second.m_pDataSub.size()) && bMakeHot; i++) {
						const EoExGripDataPtrArray& aData = it->second.m_pDataSub.at(i).m_pSubData;
						OdUInt32 j, iSize = aData.size();
						for (j = 0; j < iSize; j++) {
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
				OdUInt32 i, iSize = aKeys.size();
				for (i = 0; i < iSize; i++) {
					EoExGripDataPtr pGrip = aKeys[i];

					OdDbGripOperations::DrawType eNew = OdDbGripOperations::kHotGrip;

					if (!pGrip->data().isNull() && 0 != pGrip->data()->hotGripFunc()) {
						int iFlags = 0;
						if (pGrip->isShared())
							iFlags |= OdDbGripOperations::kSharedGrip;

						if (pGrip->data()->triggerGrip()) {
							if (false == pGrip->isShared()) {
								OdResult eRet =
									(*pGrip->data()->hotGripFunc())(pGrip->data(), pGrip->entityId(), iFlags);
								switch(eRet) {
								case eOk :
								case eGripOpGripHotToWarm : {
										eNew = OdDbGripOperations::kWarmGrip;
										break;
									}
								case eGripOpGetNewGripPoints : {
										bGetNew = true;
										idEntityToUpdate = pGrip->entityId();
										break;
									}
								}
							}
						}
						else {
							OdResult eRet = (*pGrip->data()->hotGripFunc())(pGrip->data(), pGrip->entityId(), iFlags);
							if (false == pGrip->isShared()) {
								switch(eRet) {
								case eGripOpGripHotToWarm : {
										eNew = OdDbGripOperations::kWarmGrip;
										break;
									}
								case eGripOpGetNewGripPoints : {
										bGetNew = true;
										idEntityToUpdate = pGrip->entityId();
										break;
									}
								}
							}
						}
					}

					pGrip->setStatus(eNew);
				}
			}

			if (bGetNew) {
				updateEntityGrips(idEntityToUpdate);
				return true;
			}
			EoExGripDataPtrArray aActiveKeys;
			locateGripsByStatus(OdDbGripOperations::kHotGrip, aActiveKeys);
			if (aActiveKeys.empty()) {
				// Valid situation.
				// If trigger grip performed entity modification and returned eGripHotToWarm
				// then nothing is to be done cause entity modification will cause reactor to regen grips.
				return false;
			}
			if (handleMappedRtClk(aActiveKeys, x, y))
				return true;

			OdUInt32 i, iSize = aActiveKeys.size();
			for (i = 0; i < iSize; i++)
				aActiveKeys[i]->setStatus(OdDbGripOperations::kDragImageGrip);

			GripDataMap::const_iterator it = m_aGripData.begin();
			while (it != m_aGripData.end()) {
				bool bActive = false;
				EoExGripDragPtr pDrag;
				{
					const EoExGripDataPtrArray& aData = it->second.m_pDataArray;
					OdUInt32 i, iSize = aData.size();
					for (i = 0; i < iSize; i++) {
						if (OdDbGripOperations::kDragImageGrip == aData[i]->status()) {
							bActive = true;
							pDrag = EoExGripDrag::createObject(it->first, this);
							break;
						}
					}
					for (i = 0; (i < it->second.m_pDataSub.size()) && !bActive; i++) {
						const EoExGripDataPtrArray& aData = it->second.m_pDataSub.at(i).m_pSubData;
						OdUInt32 j, iSize = aData.size();
						for (j = 0; j < iSize; j++) {
							if (OdDbGripOperations::kDragImageGrip == aData[j]->status()) {
								bActive = true;
								pDrag = EoExGripDrag::createObject(it->second.m_pDataSub.at(i).m_entPath, this);
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
			for (i = 0; i < iSize; i++) {
				m_aDrags[i]->notifyDragStarted();
				m_aDrags[i]->cloneEntity();
			}

			bool bOk = true;

			m_ptBasePoint = aKeys.first()->point();
			m_ptLastPoint = m_ptBasePoint;
			{
				// Use alternative point if needed.
				OdDbGripDataPtr pFirstData = aKeys.first()->data();
				if (0 != pFirstData.get()) {
					if (0 != pFirstData->alternateBasePoint()) {
						m_ptBasePoint = *(pFirstData->alternateBasePoint());
					}
				}
			}
			try {
				const OdString Prompt(L"Specify stretch point or [Base point/Copy/Undo/eXit]:");
				const OdString Keywords(L"Base Copy Undo eXit");
				int Options(OdEd::kGptNoLimCheck | OdEd::kGptDefault | OdEd::kGptNoUCS);
				OdGePoint3d ptFinal = m_pCmdCtx->dbUserIO()->getPoint(Prompt, Options, &m_ptBasePoint, Keywords, this);
				m_pCmdCtx->database()->startUndoRecord();

				iSize = m_aDrags.size();
				for (i = 0; i < iSize; i++)
					m_aDrags[i]->moveEntity(eyeToUcsPlane(ptFinal, m_ptBasePoint));
			}
			catch(const OdEdCancel&) {
				bOk = false;

				iSize = m_aDrags.size();
				for (i = 0; i < iSize; i++)
					m_aDrags[i]->notifyDragAborted();
			}
			iSize = m_aDrags.size();
			for (i = 0; i < iSize; i++) {
				if (bOk) {
					m_aDrags[i]->notifyDragEnded();
					updateEntityGrips(m_aDrags[i]->entityId());
				}
				else {
					m_aDrags[i]->notifyDragAborted();
				}
			}
			m_aDrags.clear();

			if (bOk)
				updateInvisibleGrips();

			iSize = aActiveKeys.size();
			for (i = 0; i < iSize; i++)
				aActiveKeys[i]->setStatus(OdDbGripOperations::kWarmGrip);
		}
	}
	else {
		return false;
	}
	return true;
}
bool EoExGripManager::startHover(int x, int y) {
	bool bRet = endHover();

	EoExGripDataPtrArray aKeys;
	locateGripsAt(x, y, aKeys);
	if (false == aKeys.empty()) {
		m_aHoverGrips = aKeys;

		OdUInt32 i, iSize = m_aHoverGrips.size();
		for (i = 0; i < iSize; i++) {
			EoExGripDataPtr pGrip = m_aHoverGrips[i];
			if (pGrip->status() == OdDbGripOperations::kWarmGrip) {
				pGrip->setStatus(OdDbGripOperations::kHoverGrip);

				if (!pGrip->data().isNull()) {
					if (0 != pGrip->data()->hoverFunc()) {
						int iFlags = 0;
						if (pGrip->isShared())
							iFlags = OdDbGripOperations::kSharedGrip;
						//OdResult eRet =
						(*pGrip->data()->hoverFunc())(pGrip->data(), pGrip->entityId(), iFlags);
					}
				}
				if (m_pGsModel) {
					m_pGsModel->onModified(pGrip.get(), (OdGiDrawable*) 0);
				}
				else {
					m_pDevice->invalidate();
				}
			}
		}
		bRet = true;
	}
	return(bRet);
}
bool EoExGripManager::endHover() {
	if (m_aHoverGrips.empty())
		return(false);

	OdUInt32 i, iSize = m_aHoverGrips.size();
	for (i = 0; i < iSize; i++) {
		EoExGripDataPtr pGrip = m_aHoverGrips[i];
		if (pGrip->status() == OdDbGripOperations::kHoverGrip) {
			pGrip->setStatus(OdDbGripOperations::kWarmGrip);
			if (m_pGsModel) {
				m_pGsModel->onModified(pGrip.get(), (OdGiDrawable*) 0);
			}
			else {
				m_pDevice->invalidate();
			}
		}
	}
	m_aHoverGrips.clear();
	return(true);
}
bool EoExGripManager::onMouseMove(int x, int y) {
	return (startHover(x, y));
}
bool EoExGripManager::onControlClick() {
	if (m_aDrags.empty())
		return false;

	// TODO: Notify active grips.

	// AEC grips use CTRL key to change mode,
	// but how to pass it throw standard interface is currently unknown.

	return true;
}
void EoExGripManager::selectionSetChanged(OdDbSelectionSetPtr selectionSet) {
	bool bRestoreOld = false;
	if (selectionSet->numEntities() > (unsigned)m_GRIPOBJLIMIT) {
		disable(true);
	}
	else {
		if (isDisabled()) {
			bRestoreOld = true;
		}
		disable(false);
	}
	OdDbDatabase* pDb = OdDbDatabase::cast(selectionSet->baseDatabase()).get();
	// Old Entities.
	{
		OdDbObjectIdArray aOld;
		GripDataMap::iterator it = m_aGripData.begin();
		while (it != m_aGripData.end()) {
			if (isDisabled()) {
				aOld.push_back(it->first);
			}
			else {
				if (false == selectionSet->isMember(it->first)) {
					aOld.push_back(it->first);
				}
				else {
					// Remove if subentities changed
					bool bRemoved = false;
					OdUInt32 se;
					for (se = 0; se < it->second.m_pDataSub.size(); se++) {
						if (!selectionSet->isMember(it->second.m_pDataSub[se].m_entPath)) {
							aOld.push_back(it->first);
							bRemoved = true;
							break;
						}
					}
					// Remove if new paths added also (workaround. tehnically new pathes must be added on second step)
					if (!bRemoved) {
						OdDbSelectionSetIteratorPtr ssIt = searchObjectSSetIterator(selectionSet, it->first);
						for (se = 0; se < ssIt->subentCount(); se++) {
							OdDbFullSubentPath tmpPath;
							ssIt->getSubentity(se, tmpPath);
							OdUInt32 searchPath = 0;
							bool bFound = false;
							for (; searchPath < it->second.m_pDataSub.size(); searchPath++) {
								if (it->second.m_pDataSub.at(searchPath).m_entPath == tmpPath) {
									bFound = true;
									break;
								}
							}
							if (!bFound) {
								aOld.push_back(it->first);
								break;
							}
						}
					}
				}
			}
			it++;
		}
		OdUInt32 i, iSize = aOld.size();
		for (i = 0; i < iSize; i++) {
			removeEntityGrips(aOld[i], true); 
			if (i % GM_PAGE_EACH_OBJECT && pDb)
			    pDb->pageObjects();
		}
	}
	// New Entities.
	{
		OdDbObjectIdArray aNew;
		OdDbSelectionSetIteratorPtr pIter = selectionSet->newIterator();
		while (false == pIter->done()) {
			if (false == isDisabled()) {
				if (m_aGripData.end() == m_aGripData.find(pIter->objectId())) {
					aNew.push_back(pIter->objectId());
				}
			}
			pIter->next();
		}
		OdUInt32 i, iSize = aNew.size();
		for (i = 0; i < iSize; i++) {
			updateEntityGrips(aNew[i]); 
			if (i % GM_PAGE_EACH_OBJECT)
				pDb->pageObjects();
		}
	}
	updateInvisibleGrips();
}
void EoExGripManager::updateEntityGrips(const OdDbObjectId& id) {
	removeEntityGrips(id, false);

	OdDbSelectionSetPtr SelectionSet = workingSelectionSet();
	if (SelectionSet.isNull() || !SelectionSet->isMember(id))
		return;

	OdDbEntityPtr pEntity = OdDbEntity::cast(id.openObject());
	if (pEntity.isNull())
		return;

	EoExGripDataPtrArray aExt;
	OdDbGripDataPtrArray aPts;

	OdDbSelectionSetIteratorPtr pObjIt = searchObjectSSetIterator(SelectionSet, id);
	if (pObjIt->subentCount() > 0) {
		for (OdUInt32 se = 0; se < pObjIt->subentCount(); se++) {
			OdDbFullSubentPath subEntPath;
			pObjIt->getSubentity(se, subEntPath);
			aPts.clear();
			if (pEntity->getGripPointsAtSubentPath(subEntPath, aPts, activeViewUnitSize(), m_GRIPSIZE, activeViewDirection(), 0) == eOk) {
				OdUInt32 prevSize = aExt.size();
				aExt.resize(prevSize + aPts.size());
				for (OdUInt32 i = 0; i < aPts.size(); i++) {
					aExt[i + prevSize] = EoExGripData::createObject(subEntPath, aPts[i], aPts[i]->gripPoint(), this);
				}
			}
		}
	}
	else {
		if (eOk == pEntity->getGripPoints(aPts, activeViewUnitSize(), m_GRIPSIZE, activeViewDirection(), 0)) {
			aExt.resize(aPts.size());
			OdUInt32 i, iSize = aExt.size();
			for (i = 0; i < iSize; i++) {
				aExt[i] = EoExGripData::createObject(pEntity->objectId(), aPts[i], aPts[i]->gripPoint(), this);
			}
		}
		else {
			OdGePoint3dArray aOldPts;
			if (eOk == pEntity->getGripPoints(aOldPts)) {
				aExt.resize(aOldPts.size());
				OdUInt32 i, iSize = aExt.size();
				for (i = 0; i < iSize; i++) {
					aExt[i] = EoExGripData::createObject(pEntity->objectId(), 0, aOldPts[i], this);
				}
			}
		}
	}
	bool IsModel = pEntity->database()->getTILEMODE();
	if (!aExt.empty()) {
		OdUInt32 i, iSize = aExt.size();
		EoExGripDataExt dExt;
		for (i = 0; i < iSize; i++) {
			OdDbFullSubentPath entPath;
			if (aExt[i]->entPath(&entPath)) {
				bool bFound = false;
				for (OdUInt32 j = 0; j < dExt.m_pDataSub.size(); j++) {
					if (dExt.m_pDataSub[j].m_entPath == entPath) {
						bFound = true;
						dExt.m_pDataSub[j].m_pSubData.append(aExt[i]);
						break;
					}
				}
				if (!bFound) {
					EoExGripDataSubent se;
					se.m_entPath = entPath;
					se.m_pSubData.append(aExt[i]);
					dExt.m_pDataSub.append(se);
				}
			}
			else {
				dExt.m_pDataArray.append(aExt[i]);
			}
		}
		//dExt.m_pDataArray = aExt;
		m_aGripData.insert(std::make_pair(id, dExt));

		for (i = 0; i < iSize; i++) {
			showGrip(aExt[i], IsModel);
		}
	}
}
void EoExGripManager::removeEntityGrips(const OdDbObjectId& id, bool bFireDone) {
	GripDataMap::iterator it = m_aGripData.find(id);
	if (it != m_aGripData.end()) {
		OdDbEntityPtr pEntity = OdDbEntity::cast(id.openObject());
		if (pEntity.get())
			pEntity->gripStatus(OdDb::kGripsToBeDeleted);

		bool bModel = id.database()->getTILEMODE();
		OdUInt32 i, iSize = it->second.m_pDataArray.size();
		for (i = 0; i < iSize; i++) {
			EoExGripDataPtr pData = it->second.m_pDataArray[i];
			hideGrip(pData, bModel);
			if (!it->second.m_pDataArray[i]->data().isNull() && it->second.m_pDataArray[i]->data()->gripOpStatFunc())
				(*it->second.m_pDataArray[i]->data()->gripOpStatFunc())(it->second.m_pDataArray[i]->data(), id, OdDbGripOperations::kGripEnd);
			it->second.m_pDataArray[i] = 0;
		}
		for (i = 0; i < it->second.m_pDataSub.size(); i++) {
			for (OdUInt32 j = 0; j < it->second.m_pDataSub.at(i).m_pSubData.size(); j++) {
				EoExGripDataPtr pData = it->second.m_pDataSub.at(i).m_pSubData[j];
				hideGrip(pData, bModel);
				it->second.m_pDataSub.at(i).m_pSubData[j] = 0;
			}
		}

		if (bFireDone) {
			if (pEntity.get())
				pEntity->gripStatus(OdDb::kGripsDone);
		}
		m_aGripData.erase(it);
	}
}
void EoExGripManager::locateGripsAt(int x, int y, EoExGripDataPtrArray& aResult) {
	aResult.clear();

	double dX = x;
	double dY = y;

	OdGePoint3d ptFirst;
	GripDataMap::const_iterator it = m_aGripData.begin();
	while (it != m_aGripData.end()) {
		for (OdUInt32 se = 0; se < it->second.m_pDataSub.size() + 1; se++) {
			const EoExGripDataPtrArray& aData = (se == 0) ? it->second.m_pDataArray : it->second.m_pDataSub[se - 1].m_pSubData;

			OdUInt32 i, iSize = aData.size();
			for (i = 0; i < iSize; i++) {
				const OdGePoint3d& ptCurrent = aData[i]->point();

				if (aResult.empty()) {
					// First grip is obtained by comparing
					// grip point device position with cursor position.
					OdGePoint3d ptDC = ptCurrent;
					ptDC.transformBy(m_pDevice->activeView()->worldToDeviceMatrix());

					double dDeltaX = ::fabs(dX - ptDC.x);
					double dDeltaY = ::fabs(dY - ptDC.y);
					bool bOk = (dDeltaX <= m_GRIPSIZE) && (dDeltaY <= m_GRIPSIZE);
					if (bOk) {
						ptFirst = ptCurrent;
						aResult.push_back(aData[i]);
					}
				}
				else {
					// Other grips are obtained by comparing world coordinates.
					// The approach here is quite raw.
					if (ptCurrent.isEqualTo(ptFirst, 1E-4)) {
						aResult.push_back(aData[i]);
					}
				}
			}
		}
		it++;
	}
}
void EoExGripManager::locateGripsByStatus(OdDbGripOperations::DrawType status, EoExGripDataPtrArray& aResult) {
	aResult.clear();

	GripDataMap::const_iterator it = m_aGripData.begin();
	while (it != m_aGripData.end()) {
		for (OdUInt32 se = 0; se < it->second.m_pDataSub.size() + 1; se++) {
			const EoExGripDataPtrArray& aData = (se == 0) ? it->second.m_pDataArray : it->second.m_pDataSub[se - 1].m_pSubData;
			OdUInt32 i, iSize = aData.size();
			for (i = 0; i < iSize; i++) {
				if (status == aData[i]->status()) {
					aResult.push_back(aData[i]);
				}
			}
		}
		it++;
	}
}
namespace {
	struct SortGripsAlongXAxis {
		bool operator()(const EoExGripDataPtr& grA, const EoExGripDataPtr& grB) {
			return OdPositive(grA->point().x, grB->point().x);
		}
	};

}
void EoExGripManager::updateInvisibleGrips() {
	EoExGripDataPtrArray aOverall;
	GripDataMap::const_iterator it = m_aGripData.begin();
	while (it != m_aGripData.end()) {
		aOverall.insert(aOverall.end(), it->second.m_pDataArray.begin(), it->second.m_pDataArray.end());
		for (OdUInt32 i = 0; i < it->second.m_pDataSub.size(); i++) {
			aOverall.insert(aOverall.end(), it->second.m_pDataSub[i].m_pSubData.begin(), it->second.m_pDataSub[i].m_pSubData.end());
		}
		it++;
	}
	OdUInt32 i, iSize = aOverall.size();
	for (i = 0; i < iSize; i++) {
		aOverall[i]->setInvisible(false);
		aOverall[i]->setShared(false);
	}
	// Not the best approach for sorting.
	// Just for demonstration.
	std::sort(aOverall.begin(), aOverall.end(), SortGripsAlongXAxis());

	iSize = aOverall.size();
	for (i = 0; i < iSize; i++) {
		if (aOverall[i]->isShared())
			continue;

		OdUInt32Array aEq;
		aEq.push_back(i);

		OdGePoint3d ptIni = aOverall[i]->point();

		OdUInt32 iNext = i + 1;
		while (iNext < iSize) {
			OdGePoint3d ptCur = aOverall[ iNext ]->point();

			if (OdEqual(ptIni.x, ptCur.x, 1E-6)) {
				if (ptIni.isEqualTo(ptCur, 1E-6)) {
					aEq.push_back(iNext);
				}
				iNext++;
			}
			else {
				break;
			}
		}
		if (aEq.size() >= 2) {
			OdUInt32 iVisible = 0;
			OdUInt32 j, jSize = aEq.size();
			for (j = 0; j < jSize; j++) {
				EoExGripDataPtr pGrip = aOverall[ aEq[ j ] ];

				bool bOk = true;
				if (!pGrip->data().isNull()) {
					if (pGrip->data()->skipWhenShared())
						bOk = false;
				}
				else {
					bOk = false;
				}

				if (bOk) {
					iVisible = j;
					break;
				}
			}
			for (j = 0; j < jSize; j++) {
				EoExGripDataPtr pGrip = aOverall[ aEq[ j ] ];

				pGrip->setShared(true);
				pGrip->setInvisible(j != iVisible);
			}
		}
	}
}
void EoExGripManager::showGrip(EoExGripData* pGrip, bool isModel) {
	OdGsPaperLayoutHelperPtr PaperLayerHelper = OdGsPaperLayoutHelper::cast(m_pDevice);
	int NumberOfViews = m_pDevice->numViews();
	if (PaperLayerHelper.get()) {
		//for (int i = 0; i < NumberOfViews; i++) {
		//	if (isModel == (PaperLayerHelper->viewAt(i) != pPaperHelper->overallView())) {
		//		PaperLayerHelper->viewAt(i)->add(pGrip, m_pGsModel);
		//	}
		//}
		OdDbObjectPtr ActiveViewportObject = m_pCmdCtx->database()->activeViewportId().openObject();
		OdDbAbstractViewportDataPtr AbstractViewportData = OdDbAbstractViewportDataPtr(ActiveViewportObject);
		if (!AbstractViewportData.isNull() && AbstractViewportData->gsView(ActiveViewportObject))
			AbstractViewportData->gsView(ActiveViewportObject)->add(pGrip, m_pGsModel);
		else {
			PaperLayerHelper->overallView()->add(pGrip, m_pGsModel);
		}
	}
	else {
		for (int i = 0; i < NumberOfViews; i++)
			m_pDevice->viewAt(i)->add(pGrip, m_pGsModel);
	}
}
void EoExGripManager::hideGrip(EoExGripData* pGrip, bool isModel) {
	OdGsPaperLayoutHelperPtr PaperLayoutHelper = OdGsPaperLayoutHelper::cast(m_pDevice);
	OdUInt32 i, iSize = m_pDevice->numViews();
	if(PaperLayoutHelper.get()) {
		for (i = 0; i < iSize; i++)
			m_pDevice->viewAt(i)->erase(pGrip);
	}
	else {
		for (i = 0; i < iSize; i++)
			m_pDevice->viewAt(i)->erase(pGrip);
	}
}
void EoExGripManager::setValue(const OdGePoint3d& pointValue) {
	OdUInt32 i, iSize = m_aDrags.size();
	OdGePoint3d newPoint = eyeToUcsPlane(pointValue, m_ptBasePoint);
	for (i = 0; i < iSize; i++) {
		m_aDrags[i]->cloneEntity(newPoint);
	}
	m_ptLastPoint = newPoint;
}
int EoExGripManager::addDrawables(OdGsView* view) {
	ODA_ASSERT(view->device() == m_pDevice->underlyingDevice().get());

	OdUInt32 i, iSize = m_aDrags.size();
	for (i = 0; i < iSize; i++) {
		view->add(m_aDrags[i].get(), /*m_pGsModel*/ 0);
	}
	return iSize;
}
void EoExGripManager::removeDrawables(OdGsView* pView) {
	OdUInt32 i, iSize = m_aDrags.size();
	for (i = 0; i < iSize; i++)
		pView->erase(m_aDrags[i].get());
}
OdDbSelectionSetPtr EoExGripManager::workingSelectionSet() const {
	if (m_pGetSelectionSetPtr) {
		return m_pGetSelectionSetPtr(m_pCmdCtx);
	}
	return OdDbSelectionSetPtr();
}
OdDbObjectId EoExGripManager::activeVpId() const {
	OdGsClientViewInfo ClientViewInfo;
	m_pDevice->activeView()->clientViewInfo(ClientViewInfo);
	return OdDbObjectId(ClientViewInfo.viewportObjectId);
}
double EoExGripManager::activeViewUnitSize() const {
	OdGsView* pView = m_pDevice->activeView();

	// Do not have access to getNumPixelsInUnitSquare here.
	double dRes;
	{
		OdGePoint2d ptDim; // getNumPixelsInUnitSquare
		{
			OdGePoint2d ll, ur;
			pView->getViewport(ll, ur);

			OdGsDCRect scrRect;
			pView->getViewport(scrRect);
			ptDim.x = fabs(double(scrRect.m_max.x - scrRect.m_min.x) / pView->fieldWidth()  * (ur.x-ll.x));
			ptDim.y = fabs(double(scrRect.m_max.y - scrRect.m_min.y) / pView->fieldHeight() * (ur.y-ll.y));
		}
		OdGeVector3d v(m_GRIPSIZE / ptDim.x, 0, 0);
		v.transformBy(pView->viewingMatrix());
		dRes = v.length() / m_GRIPSIZE;
	}
	return dRes;
}
OdGeVector3d EoExGripManager::activeViewDirection() const {
	OdGsView* pView = m_pDevice->activeView();
	return (pView->position() - pView->target()).normal();
}
OdGePoint3d EoExGripManager::eyeToUcsPlane(const OdGePoint3d &point, const OdGePoint3d &basePoint) const {
	// #8043
	OdDbObjectPtr ActiveViewportObject = activeVpId().safeOpenObject();
	OdDbAbstractViewportDataPtr AbstractViewportData(ActiveViewportObject);
	OdGePoint3d ucsOrigin;
	OdGeVector3d ucsXAxis, ucsYAxis;
	AbstractViewportData->getUcs(ActiveViewportObject, ucsOrigin, ucsXAxis, ucsYAxis);
	OdGePlane plane(/*ucsOrigin*/basePoint, ucsXAxis, ucsYAxis);
	OdGeLine3d line(point, activeViewDirection());
	OdGePoint3d newPoint;
	if (!plane.intersectWith(line, newPoint)) {
		line.set(point, ucsXAxis.crossProduct(ucsYAxis));
		if (!plane.intersectWith(line, newPoint)) // #7727
			newPoint = basePoint;
	}
	return newPoint;
}
bool EoExGripManager::handleMappedRtClk(EoExGripDataPtrArray &aActiveKeys, int x, int y) {
	OdUInt32 iSize = aActiveKeys.size();
	int rtClkIndex = -1;
	for (OdUInt32 i = 0; i < iSize; i++) {
		if (!aActiveKeys[i]->data().isNull() && 0 != aActiveKeys[i]->data()->rtClk() && aActiveKeys[i]->data()->mapGripHotToRtClk() && !aActiveKeys[i]->isShared()) {
			rtClkIndex = i;
			break;
		}
	}
	if (rtClkIndex != -1) {
		OdDbStubPtrArray ents;
		OdDbGripDataArray hotGrips;
		for (OdUInt32 i = 0; i < iSize; i++) {
			hotGrips.append(*aActiveKeys[i]->data());
			if (!ents.contains(aActiveKeys[i]->entityId()))
				ents.append(aActiveKeys[i]->entityId());
		}
		OdString menuName;
		ODHMENU menu = 0;
		ContextMenuItemIndexPtr cb = 0;
		OdResult eRet = (*aActiveKeys[rtClkIndex]->data()->rtClk())(hotGrips, ents, menuName, menu, cb);
		if (eRet == eOk && menu != 0 && cb != 0) {
			HWND wnd = ::GetActiveWindow();
			POINT pt = {x, y};
			::ClientToScreen(wnd, &pt);
			(*cb)(::TrackPopupMenu((HMENU)menu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY|TPM_RETURNCMD|TPM_LEFTBUTTON|TPM_NOANIMATION, pt.x, pt.y, 0, wnd, 0));
			::DestroyMenu((HMENU)menu);
			for (OdUInt32 i = 0; i < iSize; i++) {
				aActiveKeys[i]->setStatus(OdDbGripOperations::kWarmGrip);
			}
			updateEntityGrips(aActiveKeys[rtClkIndex]->entityId());
			return true;
		}
	}
	return false;
}
void EoExGripManager::disable(bool isDisabled) {
	if (m_bDisabled != isDisabled) {
		OdDbDatabase* pDb = m_pCmdCtx->database();
		m_bDisabled = isDisabled;
		if (isDisabled) {
			pDb->removeReactor(&m_cDbReactor);
		}
		else {
			pDb->addReactor(&m_cDbReactor);
		}
	}
}

EoExGripDbReactor::EoExGripDbReactor() : 
	m_pOwner(0) {
}
void EoExGripDbReactor::objectAppended(const OdDbDatabase* , const OdDbObject*) {
	// New object.
}
void EoExGripDbReactor::objectModified(const OdDbDatabase*, const OdDbObject* pDbObj) {
	m_pOwner->updateEntityGrips(pDbObj->objectId());
	m_pOwner->updateInvisibleGrips();
}
void EoExGripDbReactor::objectErased(const OdDbDatabase*, const OdDbObject* pDbObj, bool erased) {
	if (erased) {
		m_pOwner->removeEntityGrips(pDbObj->objectId(), true);
		m_pOwner->updateInvisibleGrips();
	}
}

#undef GM_PAGE_EACH_OBJECT