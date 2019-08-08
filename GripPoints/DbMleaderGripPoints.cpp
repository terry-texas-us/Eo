#include <OdaCommon.h>
#include "DbMleaderGripPoints.h"
#include <DbMLeader.h>
#include <DbGrip.h>
#include <Ge/GeLineSeg3d.h>
#include <DbBaseDatabase.h>
#include <Gi/GiAnnoScaleSet.h>
#include <DbAnnotativeObjectPE.h>
#include <DbObjectContextInterface.h>
#include <DbBlockTableRecord.h>
#include <DbLayout.h>
#include <DbViewport.h>
#include <DbSymUtl.h>
#include <DbObjectContextPE.h>
//#define  SUBENTITY_TEST 1
const int START_GRIP = 1;
const int DOGLEG_START_GRIP = START_GRIP;
const int DOGLEG_CENTER_GRIP = START_GRIP + 1;
const int DOGLEG_END_GRIP = START_GRIP + 2;
const int TEXT_POS_GRIP = START_GRIP + 3;
const int BLOCK_POS_GRIP = START_GRIP + 4;
const int LINE_START_GRIP = START_GRIP + 5;

static OdGePoint3d ProjectPointToLine(const OdGePoint3d ptLineStart, const OdGePoint3d ptLineEnd, const OdGePoint3d ptBase) {
	const auto a {ptBase.distanceTo(ptLineStart)};
	const auto b {ptBase.distanceTo(ptLineEnd)};
	const auto c {ptLineStart.distanceTo(ptLineEnd)};
	if (c < 1e-8) {
		return ptLineStart;
	}
	const auto d {(a * a + c * c - b * b) / (2 * c)};
	OdGePoint3d ptRet;
	ptRet.x = ptLineStart.x + (ptLineEnd.x - ptLineStart.x) * d / c;
	ptRet.y = ptLineStart.y + (ptLineEnd.y - ptLineStart.y) * d / c;
	ptRet.z = ptLineStart.z + (ptLineEnd.z - ptLineStart.z) * d / c;
	return ptRet;
}

static bool IsOnSegment(const OdGePoint3d ptSegStart, const OdGePoint3d ptSegEnd, const OdGePoint3d ptCheck) {
	const OdGeLineSeg3d pLineSeg(ptSegStart, ptSegEnd);
	return pLineSeg.isOn(ptCheck, 0);
}

/**
 * \brief 
 * \param pMLeader 
 * \param pbConnectedAtDogLeg	"Horizontal Landing" in ACAD property
 * \param pbUseDogLegCenterGrip use DOGLEG_CENTER_GRIP
 * \param pbSkipForLastVertex 
 * \return 
 */
static bool IsDoglegEnabled(OdDbMLeader* pMLeader, bool* pbConnectedAtDogLeg = nullptr, bool* pbUseDogLegCenterGrip = nullptr, bool* pbSkipForLastVertex = nullptr) {
	if (pbConnectedAtDogLeg) {
		*pbConnectedAtDogLeg = true;
	}
	if (pbUseDogLegCenterGrip) { // = true // old way
		*pbUseDogLegCenterGrip = false;
	}
	if (pbSkipForLastVertex) {
		*pbSkipForLastVertex = true;
	}
	// last vertex will be moved via DOGLEG_END_GRIP (is a duplicate of last grip point on arrow)
	ODA_ASSERT_ONCE_X(MLEADER, pMLeader);
	if (!pMLeader) {
		return false;
	}
	auto EnableDogleg {pMLeader->enableDogleg()};
	const auto tp {pMLeader->textAttachmentDirection()};
	if (tp == OdDbMLeaderStyle::kAttachmentVertical || pMLeader->leaderLineType() == OdDbMLeaderStyle::kSplineLeader) {
		EnableDogleg = false;
	}
	if (!EnableDogleg) {
		switch (pMLeader->contentType()) {
			case OdDbMLeaderStyle::kBlockContent:
				if (!pMLeader->blockContentId().isNull() && pMLeader->blockConnectionType() == OdDbMLeaderStyle::kConnectBase) {
					if (pbConnectedAtDogLeg) {
						*pbConnectedAtDogLeg = false;
					}
				}
				break;
			case OdDbMLeaderStyle::kMTextContent:
				switch (tp) {
					case OdDbMLeaderStyle::kAttachmentHorizontal:
						if (EnableDogleg) {
							break;
						}
						// remove useless grip point if dogleg is disabled // CORE-12183
					case OdDbMLeaderStyle::kAttachmentVertical:
						if (pbConnectedAtDogLeg) {
							*pbConnectedAtDogLeg = false;
						}
						if (pbSkipForLastVertex) {
							*pbSkipForLastVertex = false;
						}
						break;
				}
				break;
		}
	}
	return EnableDogleg;
}

static OdInt32 GripTypeByIndex(const OdDbEntity* entity, int iIndex) {
	auto pMLeader {OdDbMLeader::cast(entity).get()};
	ODA_ASSERT_ONCE_X(MLEADER, iIndex >= 0);
	bool bConnectedAtDogLeg = true, bUseDogLegCenterGrip = true, bSkipForLastVertex = false, bEnableDogleg = IsDoglegEnabled(pMLeader, &bConnectedAtDogLeg, &bUseDogLegCenterGrip, &bSkipForLastVertex);
	auto index {0};
	OdIntArray LeaderIndexes;
	pMLeader->getLeaderIndexes(LeaderIndexes);
	for (unsigned idx = 0, sz = LeaderIndexes.size(); idx < sz; idx++) {
		if (bEnableDogleg) {
			switch (iIndex - index) {
				case 0:
					return DOGLEG_START_GRIP;
				case 1:
					return bUseDogLegCenterGrip ? DOGLEG_CENTER_GRIP : DOGLEG_END_GRIP;
				case 2:
					if (bUseDogLegCenterGrip) {
						return DOGLEG_END_GRIP;
					}
			}
			index += bUseDogLegCenterGrip ? 3 : 2;
		} else if (bConnectedAtDogLeg) {
			if (iIndex == index) {
				return DOGLEG_END_GRIP;
			}
			index++;
		}
		OdIntArray LeaderLineIndexes;
		pMLeader->getLeaderLineIndexes(LeaderIndexes[idx], LeaderLineIndexes);
		for (unsigned idxLn = 0, szLn = LeaderLineIndexes.size(); idxLn < szLn; idxLn++) // INT-6499 problem with attachment of CORE-8843
		{
			auto nVertices {0};
			pMLeader->numVertices(LeaderLineIndexes[idxLn], nVertices);
			if (bSkipForLastVertex) {
				nVertices--;
			}
			if (iIndex < index + nVertices) {
				return LINE_START_GRIP;
			}
			index += nVertices;
		}
	}
	if (pMLeader->contentType() == OdDbMLeaderStyle::kMTextContent && ((pMLeader->mtext().get() && !pMLeader->mtext()->contents().isEmpty()) || !LeaderIndexes.isEmpty())) {
		if (iIndex == index) {
			return TEXT_POS_GRIP;
		}
		index++;
	} else if (pMLeader->contentType() == OdDbMLeaderStyle::kBlockContent && ! bConnectedAtDogLeg) {
		if (iIndex == index) {
			return BLOCK_POS_GRIP;
		}
		index++;
	}
	ODA_ASSERT_ONCE_X(MLEADER, iIndex == index); // test
	return 0;                                    // none
}

static double getScale(OdDbMLeader* pMLeader) {
	ODA_ASSERT_ONCE(pMLeader);
	auto dScale {pMLeader->scale()};
	if (dScale < 1e-8) // is zero for annotative (ObjectArx) 
	{
		// INT-6499 problem with attachment of CORE-10120 via annotative scale. 
		dScale = 1.0;
		auto pAnno {OdDbAnnotativeObjectPE::cast(pMLeader)};
		auto isAnno {pAnno.get() && pAnno->annotative(pMLeader)};
		ODA_ASSERT_ONCE_X(MLEADER, isAnno);
		OdGiAnnoScaleSet res;
		auto pDbPE {OdDbBaseDatabasePE::cast(pMLeader->database()).get()};
		if (pDbPE && !pDbPE->getAnnoScaleSet(pMLeader->objectId(), res)) {
			isAnno = false;
		}
		if (isAnno) {
			const auto pDb {pMLeader->database()};
			ODA_ASSERT_ONCE_X(MLEADER, pDb);
			OdDbAnnotationScalePtr pScale;
			OdDbAnnotScaleObjectContextDataPtr pScaleCtx;
			if (pDb->getActiveLayoutBTRId() == pDb->getModelSpaceId()) {
				pScale = pDb->getCANNOSCALE();
			} else {
				auto BlockTableRecord {OdDbBlockTableRecord::cast(pDb->getActiveLayoutBTRId().safeOpenObject())};
				auto Layout {OdDbLayout::cast(BlockTableRecord->getLayoutId().safeOpenObject(OdDb::kForRead))};
				auto pVpt {OdDbViewport::cast(Layout->activeViewportId().safeOpenObject())};
				pScale = pVpt->annotationScale();
			}
			OdDbObjectContextInterfacePtr op(pMLeader);
			if (!op->hasContext(pMLeader, *pScale)) { // is accessible via OdDbObjectContextPE only
				OdDbObjectContextPEPtr pCtxPE = op;
				auto pCtxDef {pCtxPE->getDefaultContextData(pMLeader, ODDB_ANNOTATIONSCALES_COLLECTION)};
				pScaleCtx = OdDbAnnotScaleObjectContextData::cast(pCtxDef);
				ODA_ASSERT_ONCE_X(MLEADER, pScaleCtx.get());
				if (pScaleCtx.get()) {
					pScale = nullptr;
				}
			}
			auto res {eInvalidContext};
			if (pScale.get()) {
				res = pScale->getScale(dScale);
			} else if (pScaleCtx.get()) {
				res = pScaleCtx->getScale(dScale);
			}
			ODA_ASSERT_ONCE_X(MLEADER, res == eOk && !OdZero(dScale));
			if (res == eOk && !OdZero(dScale)) {
				dScale = 1.0 / dScale;
			} else {
				dScale = 1.0;
			}
		}
	}
	return dScale;
}

static bool GetConnectionData(OdDbMLeader* pMLeader, int leaderIndex, bool bEnableDogleg, bool bConnectedAtDogLeg, OdGePoint3d& ptConnect, OdGeVector3d& vrDoglegDir, double& dDoglegLength, double* pdScale = nullptr) {
	vrDoglegDir = OdGeVector3d();
	ptConnect = OdGePoint3d();
	dDoglegLength = 0;
	const auto dScale {getScale(pMLeader)};
	if (pdScale) {
		*pdScale = dScale;
	}
	const auto tp {pMLeader->textAttachmentDirection()};
	if (!bEnableDogleg && !bConnectedAtDogLeg) // vertical
	{
		ODA_ASSERT_ONCE_X(MLEADER, pMLeader->contentType() != OdDbMLeaderStyle::kMTextContent || tp == OdDbMLeaderStyle::kAttachmentVertical || ! bEnableDogleg);
		// there is no reason in connection date for such mleader
		return true;
	}
	ODA_ASSERT_ONCE_X(MLEADER, tp != OdDbMLeaderStyle::kAttachmentVertical);
	if (tp == OdDbMLeaderStyle::kAttachmentVertical) {
		return true;
	}
	pMLeader->getDoglegDirection(leaderIndex, vrDoglegDir);
	vrDoglegDir = - vrDoglegDir;
	if (pMLeader->connectionPoint(vrDoglegDir, ptConnect) != eOk) {
		// via text location point
		return false;
	}
	auto ptCnt {ptConnect};
	if (pMLeader->connectionPoint(OdGeVector3d::kIdentity, ptCnt, tp) == eOk && ptCnt != ptConnect) { // via current value of connection point
		ptConnect = ptCnt;
	}
	dDoglegLength = pMLeader->doglegLength(leaderIndex) * dScale;
	if (pMLeader->contentType() != OdDbMLeaderStyle::kMTextContent || ! bEnableDogleg) {
		return true;
	}
	OdIntArray leaderLineIndexes;
	pMLeader->getLeaderLineIndexes(leaderIndex, leaderLineIndexes);
	if (leaderLineIndexes.isEmpty()) {
		return false;
	}
	OdGePoint3d ptConnectOpposite;
	if (pMLeader->getLastVertex(leaderLineIndexes[0], ptConnectOpposite) != eOk) {
		return true;
	}
	const auto ptConnectAlt {ptConnectOpposite - vrDoglegDir * dDoglegLength};
	if (ptConnect == ptConnectAlt) {
		return true;
	}
	// this offset (exists in some files) will disappear after recomputing
	const auto vOffs {ptConnectAlt - ptConnect};
	double dProjX = vOffs.dotProduct(vrDoglegDir), dProjAbsX = fabs(dProjX), dProjY = vOffs.dotProduct(vrDoglegDir.crossProduct(OdGeVector3d::kZAxis)), dProjAbsY = fabs(dProjY), dLen = vOffs.length();
	if (dProjAbsX * 100.0 < dLen) {
		ptConnect = ptConnectAlt;
	} else if (dProjAbsY * 25.0 < dLen) {
		ptConnect += vrDoglegDir * dProjX;
	}
	return true;
}

OdResult OdDbMleaderGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbMLeader* pMLeader = OdDbMLeader::cast(entity);
	if (pMLeader->leaderLineType() == OdDbMLeaderStyle::kInVisibleLeader) {
		return eOk;
	}
	bool bConnectedAtDogLeg = true, bUseDogLegCenterGrip = true, bSkipForLastVertex = false, bEnableDogleg = IsDoglegEnabled(pMLeader, &bConnectedAtDogLeg, &bUseDogLegCenterGrip, &bSkipForLastVertex);
	OdIntArray leaderIndexes;
	pMLeader->getLeaderIndexes(leaderIndexes);
	ODA_ASSERT_VAR(OdInt32 idxGrip = 0;)
	ODA_ASSERT_VAR(OdInt32 iGripType = 0;)
	for (OdUInt32 i = 0, sz = leaderIndexes.size(); i < sz; i++) {
		OdGePoint3d ptConnect;
		OdGeVector3d vrDoglegDir;
		double dDoglegLength = 0;
		const auto bRes {GetConnectionData(pMLeader, leaderIndexes[i], bEnableDogleg, bConnectedAtDogLeg, ptConnect, vrDoglegDir, dDoglegLength)};
		if (bRes) {
			if (bEnableDogleg) {
				ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == DOGLEG_START_GRIP);
				gripPoints.append(ptConnect);
				if (bUseDogLegCenterGrip) {
					auto tmpPt1(ptConnect + vrDoglegDir * dDoglegLength / 2);
					ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == DOGLEG_CENTER_GRIP);
					gripPoints.append(tmpPt1);
				}
				auto tmpPt2(ptConnect + vrDoglegDir * dDoglegLength);
				ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == DOGLEG_END_GRIP);
				gripPoints.append(tmpPt2);
			} else if (bConnectedAtDogLeg) {
				auto tmpPt2(ptConnect + vrDoglegDir * dDoglegLength);
				ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == DOGLEG_END_GRIP);
				gripPoints.append(tmpPt2);
			}
			OdIntArray leaderLineIndexes;
			pMLeader->getLeaderLineIndexes(leaderIndexes[i], leaderLineIndexes);
			for (OdUInt32 k = 0, lsz = leaderLineIndexes.size(); k < lsz; k++) {
				auto nVertices {0};
				if (pMLeader->numVertices(leaderLineIndexes[k], nVertices) == eOk) {
					for (auto j = 0; j < (bSkipForLastVertex ? nVertices - 1 : nVertices); j++) {
						OdGePoint3d ptVertex;
						pMLeader->getVertex(leaderLineIndexes[k], j, ptVertex);
						ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == LINE_START_GRIP);
						gripPoints.append(ptVertex);
					}
				}
			}
		}
	}
	if (pMLeader->contentType() == OdDbMLeaderStyle::kBlockContent && ! bConnectedAtDogLeg) {
		ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == BLOCK_POS_GRIP);
		OdGePoint3d ptBlockPos;
		pMLeader->getBlockPosition(ptBlockPos);
		gripPoints.append(ptBlockPos);
	} else if (pMLeader->contentType() == OdDbMLeaderStyle::kMTextContent) {
		if ((pMLeader->mtext().get() && !pMLeader->mtext()->contents().isEmpty()) || !leaderIndexes.isEmpty()) {
			ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == TEXT_POS_GRIP);
			OdGePoint3d ptTextPos;
			pMLeader->getTextLocation(ptTextPos);
			gripPoints.append(ptTextPos);
		}
	}
	ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip)) == 0);
	return eOk;
} //#endif
OdResult OdDbMleaderGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	const auto size {indices.size()};
	if (size == 0) {
		return eOk;
	}
	auto pMLeader {OdDbMLeader::cast(entity)};
	bool bConnectedAtDogLeg = true, bUseDogLegCenterGrip = true, bSkipForLastVertex = false, bEnableDogleg = IsDoglegEnabled(pMLeader, &bConnectedAtDogLeg, &bUseDogLegCenterGrip, &bSkipForLastVertex);
	OdIntArray leaderIndexes;
	pMLeader->getLeaderIndexes(leaderIndexes);
	for (unsigned iPt = 0; iPt < size; iPt++) {
		const auto iIndex {indices[iPt]};
		auto iCurIndex {0};
		auto MoveGripPoint {false};
		for (OdUInt32 i = 0; i < leaderIndexes.size(); i++) {
			OdGePoint3d ptConnect;
			OdGeVector3d vrDoglegDir;
			double dDoglegLength = 0, dScale = 1.0;
			const auto bRes {GetConnectionData(pMLeader, leaderIndexes[i], bEnableDogleg, bConnectedAtDogLeg, ptConnect, vrDoglegDir, dDoglegLength, &dScale)};
			if (bRes) {
				if (bEnableDogleg) {
					if (iIndex - iCurIndex < (bUseDogLegCenterGrip ? 3 : 2)) {
						// Dogleg
						auto StartDoglegPt {ptConnect};
						auto EndDoglegPt(ptConnect + vrDoglegDir * dDoglegLength);
						if (iIndex == iCurIndex) {
							if (dDoglegLength < 1e-8) {
								break;
							}
							auto tmpPt {ptConnect};
							tmpPt += offset;
							auto tmpNewPt {ProjectPointToLine(StartDoglegPt, EndDoglegPt, tmpPt)};
							const auto dNewLength {tmpNewPt.distanceTo(EndDoglegPt)};
							auto ZeroLength {true};
							if (IsOnSegment(StartDoglegPt, EndDoglegPt, tmpNewPt) || StartDoglegPt.distanceTo(tmpNewPt) < EndDoglegPt.distanceTo(tmpNewPt)) {
								if (dNewLength > 1e-8) {
									ZeroLength = false;
								}
							}
							if (!ZeroLength) {
								ODA_ASSERT_VAR(OdInt32 iGripType = GripTypeByIndex(entity, iIndex);)
								ODA_ASSERT_ONCE_X(MLEADER, iGripType == DOGLEG_START_GRIP)
								auto vOffset {-vrDoglegDir * (dNewLength - dDoglegLength)};
								const auto ContentType {pMLeader->contentType()};
								switch (ContentType) {
									case OdDbMLeaderStyle::kNoneContent:
										break;
									case OdDbMLeaderStyle::kMTextContent:
										break;
									default:
										pMLeader->moveMLeader(vOffset, OdDbMLeader::kMoveAllExceptArrowHeaderPoints);
										break;
								}
								pMLeader->setDoglegLength(leaderIndexes[i], dNewLength / dScale);
								if (!pMLeader->isDBRO()) {
									OdDbObject::cast(pMLeader)->subClose();
								}
								// force recompute of clone object
							}
						} else if (bUseDogLegCenterGrip && iIndex == iCurIndex + 1) {
							ODA_ASSERT_VAR(OdInt32 iGripType = GripTypeByIndex(entity, iIndex);)
							ODA_ASSERT_ONCE_X(MLEADER, iGripType == DOGLEG_CENTER_GRIP)
							pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints);
						} else {
							if (dDoglegLength < 1e-8) {
								break;
							}
							auto tmpPt {EndDoglegPt};
							tmpPt += offset;
							auto tmpNewPt {ProjectPointToLine(StartDoglegPt, EndDoglegPt, tmpPt)};
							auto tmpOffset {tmpNewPt - EndDoglegPt};
							ODA_ASSERT_VAR(OdInt32 iGripType = GripTypeByIndex(entity, iIndex);)
							ODA_ASSERT_ONCE_X(MLEADER, iGripType == DOGLEG_END_GRIP)
							pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints); // INT-6499
						}
						MoveGripPoint = true;
						break;
					}
					iCurIndex += bUseDogLegCenterGrip ? 3 : 2;
				} else if (bConnectedAtDogLeg) {
					if (iIndex == iCurIndex) {
						ODA_ASSERT_VAR(OdInt32 iGripType = GripTypeByIndex(entity, iIndex);)
						ODA_ASSERT_ONCE_X(MLEADER, iGripType == DOGLEG_END_GRIP);
						pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints);
						MoveGripPoint = true;
						break;
					}
					iCurIndex++;
				}
			}
			if (MoveGripPoint) {
				break;
			}
			OdIntArray leaderLineIndexes;
			pMLeader->getLeaderLineIndexes(leaderIndexes[i], leaderLineIndexes);
			for (OdUInt32 k = 0; k < leaderLineIndexes.size(); k++) {
				auto nVertices {0};
				if (pMLeader->numVertices(leaderLineIndexes[k], nVertices) == eOk) {
					for (auto j = 0; j < (bSkipForLastVertex ? nVertices - 1 : nVertices); j++) {
						if (iIndex == iCurIndex) {
							MoveGripPoint = true;
							ODA_ASSERT_VAR(OdInt32 iGripType = GripTypeByIndex(entity, iIndex);)
							ODA_ASSERT_ONCE_X(MLEADER, iGripType == LINE_START_GRIP);
							if (j == nVertices - 1) {
								if (!bEnableDogleg && !bConnectedAtDogLeg && pMLeader->contentType() == OdDbMLeaderStyle::kMTextContent) {
									pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints, false);
									MoveGripPoint = true;
									return eOk;
								}
								// last vertex will be moved via DOGLEG_END_GRIP (is a duplicate of last grip point on arrow) 
								ODA_FAIL_X(MLEADER);
								break;
							} // line pt
							OdGePoint3d ptVertex;
							pMLeader->getVertex(leaderLineIndexes[k], j, ptVertex);
							ptVertex += offset;
							pMLeader->setVertex(leaderLineIndexes[k], j, ptVertex);
							break;
						}
						iCurIndex++;
					}
					if (MoveGripPoint) {
						break;
					}
				}
				if (MoveGripPoint) {
					break;
				}
			}
			if (MoveGripPoint) {
				break;
			}
		}
		if (MoveGripPoint) {
			continue;
		}
		if (pMLeader->contentType() == OdDbMLeaderStyle::kBlockContent && ! bConnectedAtDogLeg) {
			if (iIndex == iCurIndex) {
				ODA_ASSERT_VAR(OdInt32 iGripType = GripTypeByIndex(entity, iIndex);)
				ODA_ASSERT_ONCE_X(MLEADER, iGripType == BLOCK_POS_GRIP); // block pt
				pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints);
				MoveGripPoint = true;
				break;
			}
			iCurIndex++;
		} else if (pMLeader->contentType() == OdDbMLeaderStyle::kMTextContent) {
			if (iIndex == iCurIndex) {
				ODA_ASSERT_VAR(OdInt32 iGripType = GripTypeByIndex(entity, iIndex);)
				ODA_ASSERT_ONCE_X(MLEADER, iGripType == TEXT_POS_GRIP);
				pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints);
				MoveGripPoint = true;
				break;
			}
			iCurIndex++;
		} //#endif
	}
	return eOk;
}

OdResult OdDbMleaderGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbMleaderGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbMleaderGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const {
	OdRxObjectPtrArray arrExploded;
	const auto Result {entity->explode(arrExploded)};
	if (Result != eOk) {
		return Result;
	}
	for (unsigned i = 0; i < arrExploded.size(); ++i) {
		auto Entity {OdDbEntity::cast(arrExploded[i])};
		if (!Entity.isNull()) {
			Entity->getOsnapPoints(objectSnapMode, selectionMarker, pickPoint, lastPoint, worldToEyeTransform, snapPoints);
		}
	}
	return eOk;
}

OdResult OdDbMleaderGripPointsPE::getGripPointsAtSubentPath(const OdDbEntity* entity, const OdDbFullSubentPath& path, OdDbGripDataPtrArray& grips, const double /*currentViewUnitSize*/, const int /*gripSize*/, const OdGeVector3d& /*currentViewDirection*/, const OdUInt32 /*bitflags*/) const {
	OdDbMLeader* pMLeader = OdDbMLeader::cast(entity);
	const auto gsMarker {(int) path.subentId().index()};
	if (gsMarker < OdDbMLeader::kLeaderLineMark || gsMarker >= OdDbMLeader::kBlockAttribute) {
		return eOk;
	}
	bool bConnectedAtDogLeg = true, bUseDogLegCenterGrip = true, bSkipForLastVertex = false, bEnableDogleg = IsDoglegEnabled(pMLeader, &bConnectedAtDogLeg, &bUseDogLegCenterGrip, &bSkipForLastVertex);
	OdIntArray leaderIndexes;
	pMLeader->getLeaderIndexes(leaderIndexes);
	if (bEnableDogleg && gsMarker >= OdDbMLeader::kDoglegMark && gsMarker < OdDbMLeader::kMTextMark) {
		for (OdUInt32 i = 0; i < leaderIndexes.size(); i++) {
			if (gsMarker != OdDbMLeader::kDoglegMark + leaderIndexes[i]) {
				continue;
			}
			OdGePoint3d ptConnect;
			OdGeVector3d vrDoglegDir;
			double dDoglegLength = 0;
			const auto bRes {GetConnectionData(pMLeader, leaderIndexes[i], bEnableDogleg, bConnectedAtDogLeg, ptConnect, vrDoglegDir, dDoglegLength)};
			if (bRes) {
				OdDbGripData* pGrip1 = new OdDbGripData();
				pGrip1->setAppData(OdIntToPtr(DOGLEG_START_GRIP));
				pGrip1->setGripPoint(ptConnect);
				grips.append(pGrip1);
				if (bUseDogLegCenterGrip) {
					const auto tmpPt1 {ptConnect + vrDoglegDir * dDoglegLength / 2};
					OdDbGripData* pGrip2 = new OdDbGripData();
					pGrip2->setAppData(OdIntToPtr(DOGLEG_CENTER_GRIP));
					pGrip2->setGripPoint(tmpPt1);
					grips.append(pGrip2);
				}
				const auto tmpPt2 {ptConnect + vrDoglegDir * dDoglegLength};
				OdDbGripData* pGrip3 = new OdDbGripData();
				pGrip3->setAppData(OdIntToPtr(DOGLEG_END_GRIP));
				pGrip3->setGripPoint(tmpPt2);
				grips.append(pGrip3);
			}
			break;
		}
	} else if (gsMarker >= OdDbMLeader::kLeaderLineMark && gsMarker < OdDbMLeader::kDoglegMark) {
		auto AddGrips {false};
		for (OdUInt32 i = 0; i < leaderIndexes.size(); i++) {
			OdGePoint3d ptConnect, ptFirst;
			OdGeVector3d vrDoglegDir;
			double dDoglegLength = 0;
			const auto bRes {GetConnectionData(pMLeader, leaderIndexes[i], bEnableDogleg, bConnectedAtDogLeg, ptConnect, vrDoglegDir, dDoglegLength)};
			if (bRes) {
				if (bEnableDogleg) {
					ptFirst = ptConnect;
				} else {
					ptFirst = ptConnect + vrDoglegDir * dDoglegLength;
				}
				OdIntArray leaderLineIndexes;
				pMLeader->getLeaderLineIndexes(leaderIndexes[i], leaderLineIndexes);
				for (OdUInt32 k = 0; k < leaderLineIndexes.size(); k++) {
					if (gsMarker != OdDbMLeader::kLeaderLineMark + leaderLineIndexes[k]) {
						continue;
					}
					auto nVertices {0};
					if (pMLeader->numVertices(leaderLineIndexes[k], nVertices) == eOk) {
						OdDbGripData* pGrip = new OdDbGripData();
						pGrip->setAppData(OdIntToPtr(LINE_START_GRIP));
						pGrip->setGripPoint(ptFirst);
						grips.append(pGrip);
						for (auto j = 0; j < (bSkipForLastVertex ? nVertices - 1 : nVertices); j++) {
							OdGePoint3d ptVertex;
							pMLeader->getVertex(leaderLineIndexes[k], j, ptVertex);
							OdDbGripData* pGrip1 = new OdDbGripData();
							pGrip1->setAppData(OdIntToPtr(LINE_START_GRIP + j + 1));
							pGrip1->setGripPoint(ptVertex);
							grips.append(pGrip1);
						}
					}
					AddGrips = true;
					break;
				}
			}
			if (AddGrips) {
				break;
			}
		}
	} else if (gsMarker >= OdDbMLeader::kMTextMark) {
		if (pMLeader->contentType() == OdDbMLeaderStyle::kMTextContent) {
			if (gsMarker == OdDbMLeader::kMTextMark) {
				OdGePoint3d ptTextPos;
				pMLeader->getTextLocation(ptTextPos);
				OdDbGripData* pGrip = new OdDbGripData();
				pGrip->setAppData(OdIntToPtr(TEXT_POS_GRIP));
				pGrip->setGripPoint(ptTextPos);
				grips.append(pGrip);
			}
		}
	}
	return eOk;
}

OdResult OdDbMleaderGripPointsPE::moveGripPointsAtSubentPaths(OdDbEntity* entity, const OdDbFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, const OdUInt32 /*bitflags*/) {
	OdDbMLeader* pMLeader = OdDbMLeader::cast(entity);
	bool bConnectedAtDogLeg = true, bUseDogLegCenterGrip = true, bSkipForLastVertex = false, bEnableDogleg = IsDoglegEnabled(pMLeader, &bConnectedAtDogLeg, &bUseDogLegCenterGrip, &bSkipForLastVertex);
	OdIntArray leaderIndexes;
	pMLeader->getLeaderIndexes(leaderIndexes);
	for (OdUInt32 i = 0; i < paths.size(); i++) {
		auto pArr {paths[i].objectIds()};
		auto CurId {pArr[pArr.size() - 1]};
		OdDbObjectId ObjId = pMLeader->id();
		if (CurId != ObjId) {
			continue;
		}
		const auto gsMarker {(int) paths[i].subentId().index()};
		const auto iGripType {OdPtrToInt32(gripAppData.at(i))};
		if (gsMarker < OdDbMLeader::kLeaderLineMark || gsMarker >= OdDbMLeader::kBlockAttribute) {
			continue;
		}
		if (gsMarker >= OdDbMLeader::kDoglegMark && gsMarker < OdDbMLeader::kMTextMark) {
			for (OdUInt32 i = 0; i < leaderIndexes.size(); i++) {
				if (gsMarker != OdDbMLeader::kDoglegMark + leaderIndexes[i]) {
					continue;
				}
				OdGePoint3d ptConnect;
				OdGeVector3d vrDoglegDir;
				double dDoglegLength = 0, dScale = 1.0;
				const auto bRes {GetConnectionData(pMLeader, leaderIndexes[i], bEnableDogleg, bConnectedAtDogLeg, ptConnect, vrDoglegDir, dDoglegLength, &dScale)};
				if (bRes) {
					if (bEnableDogleg) {
						auto StartDoglegPt {ptConnect};
						auto EndDoglegPt(ptConnect + vrDoglegDir * dDoglegLength);
						if (iGripType == DOGLEG_START_GRIP) {
							if (dDoglegLength < 1e-8) {
								break;
							}
							auto tmpPt {ptConnect};
							tmpPt += offset;
							auto tmpNewPt {ProjectPointToLine(StartDoglegPt, EndDoglegPt, tmpPt)};
							const auto dNewLength {tmpNewPt.distanceTo(EndDoglegPt)};
							auto ZeroLength {true};
							if (IsOnSegment(StartDoglegPt, EndDoglegPt, tmpNewPt) || StartDoglegPt.distanceTo(tmpNewPt) < EndDoglegPt.distanceTo(tmpNewPt)) {
								if (dNewLength > 1e-8) {
									ZeroLength = false;
								}
							}
							if (!ZeroLength) {
								auto vrNewOffset {tmpNewPt - ptConnect};
								pMLeader->moveMLeader(vrNewOffset, OdDbMLeader::kMoveContentAndDoglegPoints, false);
								pMLeader->setDoglegLength(leaderIndexes[i], dNewLength / dScale);
							}
						} else if (bUseDogLegCenterGrip && iGripType == DOGLEG_CENTER_GRIP) {
							pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints, false);
						} else if (iGripType == DOGLEG_END_GRIP) {
							if (dDoglegLength < 1e-8) {
								break;
							}
							auto tmpPt {EndDoglegPt};
							tmpPt += offset;
							auto tmpNewPt {ProjectPointToLine(StartDoglegPt, EndDoglegPt, tmpPt)};
							auto tmpOffset {tmpNewPt - EndDoglegPt};
							const auto dNewLength {tmpNewPt.distanceTo(StartDoglegPt)};
							auto ZeroLength {true};
							if (IsOnSegment(StartDoglegPt, EndDoglegPt, tmpNewPt) || StartDoglegPt.distanceTo(tmpNewPt) > EndDoglegPt.distanceTo(tmpNewPt)) {
								if (dNewLength > 1e-8) {
									ZeroLength = false;
								}
							}
							if (!ZeroLength) {
								ODA_ASSERT_ONCE_X(MLEADER, iGripType == DOGLEG_END_GRIP)
								pMLeader->setDoglegLength(leaderIndexes[i], dNewLength / dScale);
							}
						}
					}
				}
			}
		} else if (gsMarker >= OdDbMLeader::kLeaderLineMark && gsMarker < OdDbMLeader::kDoglegMark) {
			auto MoveGripPoint {false};
			for (OdUInt32 i = 0; i < leaderIndexes.size(); i++) {
				OdIntArray leaderLineIndexes;
				pMLeader->getLeaderLineIndexes(leaderIndexes[i], leaderLineIndexes);
				for (OdUInt32 k = 0; k < leaderLineIndexes.size(); k++) {
					if (gsMarker != OdDbMLeader::kLeaderLineMark + leaderLineIndexes[k]) {
						continue;
					}
					auto nVertices {0};
					if (pMLeader->numVertices(leaderLineIndexes[k], nVertices) == eOk) {
						if (bSkipForLastVertex) {
							nVertices--;
						}
						if (iGripType == LINE_START_GRIP && !bEnableDogleg) {
							pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints, false);
						} else if (iGripType > LINE_START_GRIP && iGripType <= (OdInt32)(LINE_START_GRIP + nVertices)) {
							const auto iVertex {iGripType - LINE_START_GRIP - 1};
							OdGePoint3d ptVertex;
							pMLeader->getVertex(leaderLineIndexes[k], iVertex, ptVertex);
							ptVertex += offset;
							pMLeader->setVertex(leaderLineIndexes[k], iVertex, ptVertex);
							MoveGripPoint = true;
							break;
						}
					}
				}
				if (MoveGripPoint) {
					break;
				}
			}
		} else if (gsMarker >= OdDbMLeader::kMTextMark) {
			if (pMLeader->contentType() == OdDbMLeaderStyle::kMTextContent) {
				if (gsMarker == OdDbMLeader::kMTextMark && iGripType == TEXT_POS_GRIP) {
					pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints, false);
				}
			}
		}
	}
	return eOk;
}
