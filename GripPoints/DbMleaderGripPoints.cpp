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

static OdGePoint3d ProjectPointToLine(const OdGePoint3d lineStartPoint, const OdGePoint3d lineEndPoint, const OdGePoint3d basePoint) {
	const auto a {basePoint.distanceTo(lineStartPoint)};
	const auto b {basePoint.distanceTo(lineEndPoint)};
	const auto c {lineStartPoint.distanceTo(lineEndPoint)};
	if (c < 1e-8) {
		return lineStartPoint;
	}
	const auto d {(a * a + c * c - b * b) / (2 * c)};
	OdGePoint3d ReturnPoint;
	ReturnPoint.x = lineStartPoint.x + (lineEndPoint.x - lineStartPoint.x) * d / c;
	ReturnPoint.y = lineStartPoint.y + (lineEndPoint.y - lineStartPoint.y) * d / c;
	ReturnPoint.z = lineStartPoint.z + (lineEndPoint.z - lineStartPoint.z) * d / c;
	return ReturnPoint;
}

static bool IsOnSegment(const OdGePoint3d segmentStartPoint, const OdGePoint3d segmentEndPoint, const OdGePoint3d checkPoint) {
	const OdGeLineSeg3d LineSeg(segmentStartPoint, segmentEndPoint);
	return LineSeg.isOn(checkPoint, 0);
}

/**
 * \brief 
 * \param pMLeader 
 * \param connectedAtDogleg	"Horizontal Landing" in ACAD property
 * \param useDoglegCenterGrip use DOGLEG_CENTER_GRIP
 * \param pbSkipForLastVertex 
 * \return 
 */
static bool IsDoglegEnabled(OdDbMLeader* pMLeader, bool* connectedAtDogleg = nullptr, bool* useDoglegCenterGrip = nullptr, bool* pbSkipForLastVertex = nullptr) {
	if (connectedAtDogleg) {
		*connectedAtDogleg = true;
	}
	if (useDoglegCenterGrip) { // = true // old way
		*useDoglegCenterGrip = false;
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
					if (connectedAtDogleg) {
						*connectedAtDogleg = false;
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
						if (connectedAtDogleg) {
							*connectedAtDogleg = false;
						}
						if (pbSkipForLastVertex) {
							*pbSkipForLastVertex = false;
						}
						break;
				}
				break;
			default: ;
		}
	}
	return EnableDogleg;
}

static long GripTypeByIndex(const OdDbEntity* entity, int iIndex) {
	auto pMLeader {OdDbMLeader::cast(entity).get()};
	ODA_ASSERT_ONCE_X(MLEADER, iIndex >= 0);
	auto ConnectedAtDogleg {true};
	auto UseDoglegCenterGrip {true};
	auto SkipForLastVertex {false};
	const auto EnableDogleg = IsDoglegEnabled(pMLeader, &ConnectedAtDogleg, &UseDoglegCenterGrip, &SkipForLastVertex);
	auto index {0};
	OdIntArray LeaderIndexes;
	pMLeader->getLeaderIndexes(LeaderIndexes);
	for (auto LeaderIndex : LeaderIndexes) {
		if (EnableDogleg) {
			switch (iIndex - index) {
				case 0:
					return DOGLEG_START_GRIP;
				case 1:
					return UseDoglegCenterGrip ? DOGLEG_CENTER_GRIP : DOGLEG_END_GRIP;
				case 2:
					if (UseDoglegCenterGrip) {
						return DOGLEG_END_GRIP;
					}
				default: ;
			}
			index += UseDoglegCenterGrip ? 3 : 2;
		} else if (ConnectedAtDogleg) {
			if (iIndex == index) {
				return DOGLEG_END_GRIP;
			}
			index++;
		}
		OdIntArray LeaderLineIndexes;
		pMLeader->getLeaderLineIndexes(LeaderIndex, LeaderLineIndexes);
		for (auto LeaderLineIndex : LeaderLineIndexes) {
			auto nVertices {0};
			pMLeader->numVertices(LeaderLineIndex, nVertices);
			if (SkipForLastVertex) {
				nVertices--;
			}
			if (iIndex < index + nVertices) {
				return LINE_START_GRIP;
			}
			index += nVertices;
		}
	}
	if (pMLeader->contentType() == OdDbMLeaderStyle::kMTextContent && (pMLeader->mtext().get() && !pMLeader->mtext()->contents().isEmpty() || !LeaderIndexes.isEmpty())) {
		if (iIndex == index) {
			return TEXT_POS_GRIP;
		}
		index++;
	} else if (pMLeader->contentType() == OdDbMLeaderStyle::kBlockContent && ! ConnectedAtDogleg) {
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
	auto Scale {pMLeader->scale()};
	if (Scale < 1e-8) { // is zero for annotative (ObjectArx) 
		Scale = 1.0;
		auto AnnotativeObject {OdDbAnnotativeObjectPE::cast(pMLeader)};
		auto IsAnnotative {AnnotativeObject.get() && AnnotativeObject->annotative(pMLeader)};
		ODA_ASSERT_ONCE_X(MLEADER, IsAnnotative);
		OdGiAnnoScaleSet AnnotationScaleSet;
		auto BaseDatabase {OdDbBaseDatabasePE::cast(pMLeader->database()).get()};
		if (BaseDatabase && !BaseDatabase->getAnnoScaleSet(pMLeader->objectId(), AnnotationScaleSet)) {
			IsAnnotative = false;
		}
		if (IsAnnotative) {
			const auto Database {pMLeader->database()};
			ODA_ASSERT_ONCE_X(MLEADER, Database);
			OdDbAnnotationScalePtr AnnotationScale;
			OdDbAnnotScaleObjectContextDataPtr AnnotationScaleObjectContextData;
			if (Database->getActiveLayoutBTRId() == Database->getModelSpaceId()) {
				AnnotationScale = Database->getCANNOSCALE();
			} else {
				auto BlockTableRecord {OdDbBlockTableRecord::cast(Database->getActiveLayoutBTRId().safeOpenObject())};
				auto Layout {OdDbLayout::cast(BlockTableRecord->getLayoutId().safeOpenObject(OdDb::kForRead))};
				auto Viewport {OdDbViewport::cast(Layout->activeViewportId().safeOpenObject())};
				AnnotationScale = Viewport->annotationScale();
			}
			OdDbObjectContextInterfacePtr op(pMLeader);
			if (!op->hasContext(pMLeader, *AnnotationScale)) { // is accessible via OdDbObjectContextPE only
				OdDbObjectContextPEPtr pCtxPE {op};
				auto pCtxDef {pCtxPE->getDefaultContextData(pMLeader, ODDB_ANNOTATIONSCALES_COLLECTION)};
				AnnotationScaleObjectContextData = OdDbAnnotScaleObjectContextData::cast(pCtxDef);
				ODA_ASSERT_ONCE_X(MLEADER, AnnotationScaleObjectContextData.get());
				if (AnnotationScaleObjectContextData.get()) {
					AnnotationScale = nullptr;
				}
			}
			auto Result {eInvalidContext};
			if (AnnotationScale.get()) {
				Result = AnnotationScale->getScale(Scale);
			} else if (AnnotationScaleObjectContextData.get()) {
				Result = AnnotationScaleObjectContextData->getScale(Scale);
			}
			ODA_ASSERT_ONCE_X(MLEADER, Result == eOk && !OdZero(Scale));
			if (Result == eOk && !OdZero(Scale)) {
				Scale = 1.0 / Scale;
			} else {
				Scale = 1.0;
			}
		}
	}
	return Scale;
}

static bool GetConnectionData(OdDbMLeader* pMLeader, int leaderIndex, bool enableDogleg, bool connectedAtDogleg, OdGePoint3d& connectPoint, OdGeVector3d& doglegDirection, double& doglegLength, double* scale = nullptr) {
	doglegDirection = OdGeVector3d();
	connectPoint = OdGePoint3d();
	doglegLength = 0;
	const auto Scale {getScale(pMLeader)};
	if (scale) {
		*scale = Scale;
	}
	const auto tp {pMLeader->textAttachmentDirection()};
	if (!enableDogleg && !connectedAtDogleg) // vertical
	{
		ODA_ASSERT_ONCE_X(MLEADER, pMLeader->contentType() != OdDbMLeaderStyle::kMTextContent || tp == OdDbMLeaderStyle::kAttachmentVertical || ! enableDogleg);
		// there is no reason in connection date for such mleader
		return true;
	}
	ODA_ASSERT_ONCE_X(MLEADER, tp != OdDbMLeaderStyle::kAttachmentVertical);
	if (tp == OdDbMLeaderStyle::kAttachmentVertical) {
		return true;
	}
	pMLeader->getDoglegDirection(leaderIndex, doglegDirection);
	doglegDirection = - doglegDirection;
	if (pMLeader->connectionPoint(doglegDirection, connectPoint) != eOk) {
		// via text location point
		return false;
	}
	auto ptCnt {connectPoint};
	if (pMLeader->connectionPoint(OdGeVector3d::kIdentity, ptCnt, tp) == eOk && ptCnt != connectPoint) { // via current value of connection point
		connectPoint = ptCnt;
	}
	doglegLength = pMLeader->doglegLength(leaderIndex) * Scale;
	if (pMLeader->contentType() != OdDbMLeaderStyle::kMTextContent || ! enableDogleg) {
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
	const auto ptConnectAlt {ptConnectOpposite - doglegDirection * doglegLength};
	if (connectPoint == ptConnectAlt) {
		return true;
	}
	// this offset (exists in some files) will disappear after recomputing
	const auto vOffs {ptConnectAlt - connectPoint};
	double dProjX = vOffs.dotProduct(doglegDirection), dProjAbsX = fabs(dProjX), dProjY = vOffs.dotProduct(doglegDirection.crossProduct(OdGeVector3d::kZAxis)), dProjAbsY = fabs(dProjY), dLen = vOffs.length();
	if (dProjAbsX * 100.0 < dLen) {
		connectPoint = ptConnectAlt;
	} else if (dProjAbsY * 25.0 < dLen) {
		connectPoint += doglegDirection * dProjX;
	}
	return true;
}

OdResult OdDbMleaderGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbMLeader* pMLeader {OdDbMLeader::cast(entity)};
	if (pMLeader->leaderLineType() == OdDbMLeaderStyle::kInVisibleLeader) {
		return eOk;
	}
	auto ConnectedAtDogleg {true};
	auto UseDoglegCenterGrip {true};
	auto SkipForLastVertex {false};
	const auto EnableDogleg = IsDoglegEnabled(pMLeader, &ConnectedAtDogleg, &UseDoglegCenterGrip, &SkipForLastVertex);
	OdIntArray LeaderIndexes;
	pMLeader->getLeaderIndexes(LeaderIndexes);
	ODA_ASSERT_VAR(OdInt32 idxGrip = 0;)
	ODA_ASSERT_VAR(OdInt32 iGripType = 0;)
	for (auto LeaderIndex : LeaderIndexes) {
		OdGePoint3d ConnectPoint;
		OdGeVector3d DoglegDirection;
		auto DoglegLength {0.0};
		const auto bRes {GetConnectionData(pMLeader, LeaderIndex, EnableDogleg, ConnectedAtDogleg, ConnectPoint, DoglegDirection, DoglegLength)};
		if (bRes) {
			if (EnableDogleg) {
				ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == DOGLEG_START_GRIP);
				gripPoints.append(ConnectPoint);
				if (UseDoglegCenterGrip) {
					auto tmpPt1(ConnectPoint + DoglegDirection * DoglegLength / 2);
					ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == DOGLEG_CENTER_GRIP);
					gripPoints.append(tmpPt1);
				}
				auto tmpPt2(ConnectPoint + DoglegDirection * DoglegLength);
				ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == DOGLEG_END_GRIP);
				gripPoints.append(tmpPt2);
			} else if (ConnectedAtDogleg) {
				auto tmpPt2(ConnectPoint + DoglegDirection * DoglegLength);
				ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == DOGLEG_END_GRIP);
				gripPoints.append(tmpPt2);
			}
			OdIntArray LeaderLineIndexes;
			pMLeader->getLeaderLineIndexes(LeaderIndex, LeaderLineIndexes);
			for (auto LeaderLineIndex : LeaderLineIndexes) {
				auto nVertices {0};
				if (pMLeader->numVertices(LeaderLineIndex, nVertices) == eOk) {
					for (auto j = 0; j < (SkipForLastVertex ? nVertices - 1 : nVertices); j++) {
						OdGePoint3d ptVertex;
						pMLeader->getVertex(LeaderLineIndex, j, ptVertex);
						ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == LINE_START_GRIP);
						gripPoints.append(ptVertex);
					}
				}
			}
		}
	}
	if (pMLeader->contentType() == OdDbMLeaderStyle::kBlockContent && ! ConnectedAtDogleg) {
		ODA_ASSERT_ONCE_X(MLEADER, (iGripType = GripTypeByIndex(entity, idxGrip++)) == BLOCK_POS_GRIP);
		OdGePoint3d ptBlockPos;
		pMLeader->getBlockPosition(ptBlockPos);
		gripPoints.append(ptBlockPos);
	} else if (pMLeader->contentType() == OdDbMLeaderStyle::kMTextContent) {
		if (pMLeader->mtext().get() && !pMLeader->mtext()->contents().isEmpty() || !LeaderIndexes.isEmpty()) {
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
	if (indices.empty()) {
		return eOk;
	}
	auto pMLeader {OdDbMLeader::cast(entity)};
	auto ConnectedAtDogleg {true};
	auto UseDoglegCenterGrip {true};
	auto SkipForLastVertex {false};
	const auto EnableDogleg {IsDoglegEnabled(pMLeader, &ConnectedAtDogleg, &UseDoglegCenterGrip, &SkipForLastVertex)};
	OdIntArray LeaderIndexes;
	pMLeader->getLeaderIndexes(LeaderIndexes);
	for (auto Index : indices) {
		auto CurrentIndex {0};
		auto MoveGripPoint {false};
		for (auto LeaderIndex : LeaderIndexes) {
			OdGePoint3d ConnectPoint;
			OdGeVector3d DoglegDirection;
			auto DoglegLength {0.0};
			auto Scale {1.0};
			const auto bRes {GetConnectionData(pMLeader, LeaderIndex, EnableDogleg, ConnectedAtDogleg, ConnectPoint, DoglegDirection, DoglegLength, &Scale)};
			if (bRes) {
				if (EnableDogleg) {
					if (Index - CurrentIndex < (UseDoglegCenterGrip ? 3 : 2)) {
						// Dogleg
						auto StartDoglegPt {ConnectPoint};
						auto EndDoglegPt(ConnectPoint + DoglegDirection * DoglegLength);
						if (Index == CurrentIndex) {
							if (DoglegLength < 1e-8) {
								break;
							}
							auto tmpPt {ConnectPoint};
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
								ODA_ASSERT_VAR(OdInt32 GripType = GripTypeByIndex(entity, Index);)
								ODA_ASSERT_ONCE_X(MLEADER, GripType == DOGLEG_START_GRIP)
								auto Offset {-DoglegDirection * (dNewLength - DoglegLength)};
								const auto ContentType {pMLeader->contentType()};
								switch (ContentType) {
									case OdDbMLeaderStyle::kNoneContent:
										break;
									case OdDbMLeaderStyle::kMTextContent:
										break;
									default:
										pMLeader->moveMLeader(Offset, OdDbMLeader::kMoveAllExceptArrowHeaderPoints);
										break;
								}
								pMLeader->setDoglegLength(LeaderIndex, dNewLength / Scale);
								if (!pMLeader->isDBRO()) {
									OdDbObject::cast(pMLeader)->subClose();
								}
								// force recompute of clone object
							}
						} else if (UseDoglegCenterGrip && Index == CurrentIndex + 1) {
							ODA_ASSERT_VAR(OdInt32 GripType = GripTypeByIndex(entity, Index);)
							ODA_ASSERT_ONCE_X(MLEADER, GripType == DOGLEG_CENTER_GRIP)
							pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints);
						} else {
							if (DoglegLength < 1e-8) {
								break;
							}
							auto tmpPt {EndDoglegPt};
							tmpPt += offset;
							auto tmpNewPt {ProjectPointToLine(StartDoglegPt, EndDoglegPt, tmpPt)};
							ODA_ASSERT_VAR(OdInt32 GripType = GripTypeByIndex(entity, Index);)
							ODA_ASSERT_ONCE_X(MLEADER, GripType == DOGLEG_END_GRIP)
							pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints); // INT-6499
						}
						MoveGripPoint = true;
						break;
					}
					CurrentIndex += UseDoglegCenterGrip ? 3 : 2;
				} else if (ConnectedAtDogleg) {
					if (Index == CurrentIndex) {
						ODA_ASSERT_VAR(OdInt32 GripType = GripTypeByIndex(entity, Index);)
						ODA_ASSERT_ONCE_X(MLEADER, GripType == DOGLEG_END_GRIP);
						pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints);
						MoveGripPoint = true;
						break;
					}
					CurrentIndex++;
				}
			}
			if (MoveGripPoint) {
				break;
			}
			OdIntArray LeaderLineIndexes;
			pMLeader->getLeaderLineIndexes(LeaderIndex, LeaderLineIndexes);
			for (auto LeaderLineIndex : LeaderLineIndexes) {
				auto nVertices {0};
				if (pMLeader->numVertices(LeaderLineIndex, nVertices) == eOk) {
					for (auto j = 0; j < (SkipForLastVertex ? nVertices - 1 : nVertices); j++) {
						if (Index == CurrentIndex) {
							MoveGripPoint = true;
							ODA_ASSERT_VAR(OdInt32 GripType = GripTypeByIndex(entity, Index);)
							ODA_ASSERT_ONCE_X(MLEADER, GripType == LINE_START_GRIP);
							if (j == nVertices - 1) {
								if (!EnableDogleg && !ConnectedAtDogleg && pMLeader->contentType() == OdDbMLeaderStyle::kMTextContent) {
									pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints, false);
									MoveGripPoint = true;
									return eOk;
								}
								// last vertex will be moved via DOGLEG_END_GRIP (is a duplicate of last grip point on arrow) 
								ODA_FAIL_X(MLEADER);
								break;
							} // line pt
							OdGePoint3d ptVertex;
							pMLeader->getVertex(LeaderLineIndex, j, ptVertex);
							ptVertex += offset;
							pMLeader->setVertex(LeaderLineIndex, j, ptVertex);
							break;
						}
						CurrentIndex++;
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
		if (pMLeader->contentType() == OdDbMLeaderStyle::kBlockContent && ! ConnectedAtDogleg) {
			if (Index == CurrentIndex) {
				ODA_ASSERT_VAR(OdInt32 GripType = GripTypeByIndex(entity, Index);)
				ODA_ASSERT_ONCE_X(MLEADER, GripType == BLOCK_POS_GRIP); // block pt
				pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints);
				MoveGripPoint = true;
				break;
			}
			CurrentIndex++;
		} else if (pMLeader->contentType() == OdDbMLeaderStyle::kMTextContent) {
			if (Index == CurrentIndex) {
				ODA_ASSERT_VAR(OdInt32 GripType = GripTypeByIndex(entity, Index);)
				ODA_ASSERT_ONCE_X(MLEADER, GripType == TEXT_POS_GRIP);
				pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints);
				MoveGripPoint = true;
				break;
			}
			CurrentIndex++;
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
	OdRxObjectPtrArray ExplodedObjects;
	const auto Result {entity->explode(ExplodedObjects)};
	if (Result != eOk) {
		return Result;
	}
	for (auto& ExplodedObject : ExplodedObjects) {
		auto Entity {OdDbEntity::cast(ExplodedObject)};
		if (!Entity.isNull()) {
			Entity->getOsnapPoints(objectSnapMode, selectionMarker, pickPoint, lastPoint, worldToEyeTransform, snapPoints);
		}
	}
	return eOk;
}

OdResult OdDbMleaderGripPointsPE::getGripPointsAtSubentPath(const OdDbEntity* entity, const OdDbFullSubentPath& path, OdDbGripDataPtrArray& grips, const double /*currentViewUnitSize*/, const int /*gripSize*/, const OdGeVector3d& /*currentViewDirection*/, const unsigned long /*bitFlags*/) const {
	OdDbMLeader* pMLeader {OdDbMLeader::cast(entity)};
	const auto gsMarker {static_cast<int>(path.subentId().index())};
	if (gsMarker < OdDbMLeader::kLeaderLineMark || gsMarker >= OdDbMLeader::kBlockAttribute) {
		return eOk;
	}
	auto ConnectedAtDogleg {true};
	auto UseDoglegCenterGrip {true};
	auto SkipForLastVertex {false};
	const auto EnableDogleg {IsDoglegEnabled(pMLeader, &ConnectedAtDogleg, &UseDoglegCenterGrip, &SkipForLastVertex)};
	OdIntArray LeaderIndexes;
	pMLeader->getLeaderIndexes(LeaderIndexes);
	if (EnableDogleg && gsMarker >= OdDbMLeader::kDoglegMark && gsMarker < OdDbMLeader::kMTextMark) {
		for (auto LeaderIndex : LeaderIndexes) {
			if (gsMarker != OdDbMLeader::kDoglegMark + LeaderIndex) {
				continue;
			}
			OdGePoint3d ConnectPoint;
			OdGeVector3d DoglegDirection;
			auto DoglegLength {0.0};
			const auto bRes {GetConnectionData(pMLeader, LeaderIndex, EnableDogleg, ConnectedAtDogleg, ConnectPoint, DoglegDirection, DoglegLength)};
			if (bRes) {
				auto Grip1 {new OdDbGripData()};
				Grip1->setAppData(OdIntToPtr(DOGLEG_START_GRIP));
				Grip1->setGripPoint(ConnectPoint);
				grips.append(Grip1);
				if (UseDoglegCenterGrip) {
					const auto tmpPt1 {ConnectPoint + DoglegDirection * DoglegLength / 2.0};
					auto Grip2 {new OdDbGripData()};
					Grip2->setAppData(OdIntToPtr(DOGLEG_CENTER_GRIP));
					Grip2->setGripPoint(tmpPt1);
					grips.append(Grip2);
				}
				const auto tmpPt2 {ConnectPoint + DoglegDirection * DoglegLength};
				auto Grip3 {new OdDbGripData()};
				Grip3->setAppData(OdIntToPtr(DOGLEG_END_GRIP));
				Grip3->setGripPoint(tmpPt2);
				grips.append(Grip3);
			}
			break;
		}
	} else if (gsMarker >= OdDbMLeader::kLeaderLineMark && gsMarker < OdDbMLeader::kDoglegMark) {
		auto AddGrips {false};
		for (int LeaderIndex : LeaderIndexes) {
			OdGePoint3d ConnectPoint;
			OdGePoint3d FirstPoint;
			OdGeVector3d DoglegDirection;
			auto DoglegLength {0.0};
			const auto bRes {GetConnectionData(pMLeader, LeaderIndex, EnableDogleg, ConnectedAtDogleg, ConnectPoint, DoglegDirection, DoglegLength)};
			if (bRes) {
				if (EnableDogleg) {
					FirstPoint = ConnectPoint;
				} else {
					FirstPoint = ConnectPoint + DoglegDirection * DoglegLength;
				}
				OdIntArray LeaderLineIndexes;
				pMLeader->getLeaderLineIndexes(LeaderIndex, LeaderLineIndexes);
				for (auto LeaderLineIndex : LeaderLineIndexes) {
					if (gsMarker != OdDbMLeader::kLeaderLineMark + LeaderLineIndex) {
						continue;
					}
					auto nVertices {0};
					if (pMLeader->numVertices(LeaderLineIndex, nVertices) == eOk) {
						auto Grip {new OdDbGripData()};
						Grip->setAppData(OdIntToPtr(LINE_START_GRIP));
						Grip->setGripPoint(FirstPoint);
						grips.append(Grip);
						for (auto j = 0; j < (SkipForLastVertex ? nVertices - 1 : nVertices); j++) {
							OdGePoint3d ptVertex;
							pMLeader->getVertex(LeaderLineIndex, j, ptVertex);
							auto Grip1 {new OdDbGripData()};
							Grip1->setAppData(OdIntToPtr(LINE_START_GRIP + j + 1));
							Grip1->setGripPoint(ptVertex);
							grips.append(Grip1);
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
				OdGePoint3d TextLocation;
				pMLeader->getTextLocation(TextLocation);
				auto Grip {new OdDbGripData()};
				Grip->setAppData(OdIntToPtr(TEXT_POS_GRIP));
				Grip->setGripPoint(TextLocation);
				grips.append(Grip);
			}
		}
	}
	return eOk;
}

OdResult OdDbMleaderGripPointsPE::moveGripPointsAtSubentPaths(OdDbEntity* entity, const OdDbFullSubentPathArray& paths, const OdDbVoidPtrArray& gripAppData, const OdGeVector3d& offset, const unsigned long /*bitFlags*/) {
	OdDbMLeader* pMLeader {OdDbMLeader::cast(entity)};
	auto ConnectedAtDogleg {true};
	auto UseDoglegCenterGrip {true};
	auto SkipForLastVertex {false};
	const auto EnableDogleg {IsDoglegEnabled(pMLeader, &ConnectedAtDogleg, &UseDoglegCenterGrip, &SkipForLastVertex)};
	OdIntArray LeaderIndexes;
	pMLeader->getLeaderIndexes(LeaderIndexes);
	for (unsigned i = 0; i < paths.size(); i++) {
		auto pArr {paths[i].objectIds()};
		auto CurId {pArr[pArr.size() - 1]};
		OdDbObjectId ObjId {pMLeader->id()};
		if (CurId != ObjId) {
			continue;
		}
		const auto gsMarker {static_cast<int>(paths[i].subentId().index())};
		const auto iGripType {OdPtrToInt32(gripAppData.at(i))};
		if (gsMarker < OdDbMLeader::kLeaderLineMark || gsMarker >= OdDbMLeader::kBlockAttribute) {
			continue;
		}
		if (gsMarker >= OdDbMLeader::kDoglegMark && gsMarker < OdDbMLeader::kMTextMark) {
			for (auto LeaderIndex : LeaderIndexes) {
				if (gsMarker != OdDbMLeader::kDoglegMark + LeaderIndex) {
					continue;
				}
				OdGePoint3d ConnectPoint;
				OdGeVector3d DoglegDirection;
				auto DoglegLength {0.0};
				auto Scale {1.0};
				const auto bRes {GetConnectionData(pMLeader, LeaderIndex, EnableDogleg, ConnectedAtDogleg, ConnectPoint, DoglegDirection, DoglegLength, &Scale)};
				if (bRes) {
					if (EnableDogleg) {
						auto StartDoglegPt {ConnectPoint};
						auto EndDoglegPt(ConnectPoint + DoglegDirection * DoglegLength);
						if (iGripType == DOGLEG_START_GRIP) {
							if (DoglegLength < 1e-8) {
								break;
							}
							auto tmpPt {ConnectPoint};
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
								auto vrNewOffset {tmpNewPt - ConnectPoint};
								pMLeader->moveMLeader(vrNewOffset, OdDbMLeader::kMoveContentAndDoglegPoints, false);
								pMLeader->setDoglegLength(LeaderIndex, dNewLength / Scale);
							}
						} else if (UseDoglegCenterGrip && iGripType == DOGLEG_CENTER_GRIP) {
							pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints, false);
						} else if (iGripType == DOGLEG_END_GRIP) {
							if (DoglegLength < 1e-8) {
								break;
							}
							auto tmpPt {EndDoglegPt};
							tmpPt += offset;
							auto tmpNewPt {ProjectPointToLine(StartDoglegPt, EndDoglegPt, tmpPt)};
							const auto dNewLength {tmpNewPt.distanceTo(StartDoglegPt)};
							auto ZeroLength {true};
							if (IsOnSegment(StartDoglegPt, EndDoglegPt, tmpNewPt) || StartDoglegPt.distanceTo(tmpNewPt) > EndDoglegPt.distanceTo(tmpNewPt)) {
								if (dNewLength > 1e-8) {
									ZeroLength = false;
								}
							}
							if (!ZeroLength) {
								ODA_ASSERT_ONCE_X(MLEADER, iGripType == DOGLEG_END_GRIP)
								pMLeader->setDoglegLength(LeaderIndex, dNewLength / Scale);
							}
						}
					}
				}
			}
		} else if (gsMarker >= OdDbMLeader::kLeaderLineMark && gsMarker < OdDbMLeader::kDoglegMark) {
			auto MoveGripPoint {false};
			for (auto LeaderIndex : LeaderIndexes) {
				OdIntArray LeaderLineIndexes;
				pMLeader->getLeaderLineIndexes(LeaderIndex, LeaderLineIndexes);
				for (auto LeaderLineIndex : LeaderLineIndexes) {
					if (gsMarker != OdDbMLeader::kLeaderLineMark + LeaderLineIndex) {
						continue;
					}
					auto nVertices {0};
					if (pMLeader->numVertices(LeaderLineIndex, nVertices) == eOk) {
						if (SkipForLastVertex) {
							nVertices--;
						}
						if (iGripType == LINE_START_GRIP && !EnableDogleg) {
							pMLeader->moveMLeader(offset, OdDbMLeader::kMoveContentAndDoglegPoints, false);
						} else if (iGripType > LINE_START_GRIP && iGripType <= (OdInt32)(LINE_START_GRIP + nVertices)) {
							const auto iVertex {iGripType - LINE_START_GRIP - 1};
							OdGePoint3d ptVertex;
							pMLeader->getVertex(LeaderLineIndex, iVertex, ptVertex);
							ptVertex += offset;
							pMLeader->setVertex(LeaderLineIndex, iVertex, ptVertex);
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
