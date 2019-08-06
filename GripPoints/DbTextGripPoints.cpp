#include <OdaCommon.h>
#include "DbTextGripPoints.h"
#include <Ge/GeLine3d.h>
#include <AbstractViewPE.h>
#include <DbViewportTableRecord.h>
#include "OdGripPointsModule.h"

// Returns GripPoints
OdResult OdDbTextGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbTextPtr pText = entity;
	gripPoints.append(pText->position()); // left lower corner of Text
	if (!is_Justify_Left(pText)) { // if in AutoCad properties Justify != Left
		gripPoints.append(pText->alignmentPoint());
	}
	// OdDbText has two grip points: position() and alignmentPoint
	const auto dThickness {pText->thickness()};
	if (!OdZero(dThickness)) {
		const auto vExtrusion {pText->normal() * dThickness};
		gripPoints.append(pText->position() + vExtrusion);
		if (!is_Justify_Left(pText)) {
			gripPoints.append(pText->alignmentPoint() + vExtrusion);
		}
	}
	return eOk;
}

// Move text
OdResult OdDbTextGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& vOffset) {
	const auto indicesSize {indices.size()};
	if (indicesSize == 0) {
		return eOk;
	}
	OdDbTextPtr pText = entity;
	auto MovePosition {false};
	auto MoveAlignmentPoint {false};
	auto offset {vOffset};
	// Project offset on entity's plane in view direction
	const auto bPlaneChanges {!projectOffset(pText->database(), pText->normal(), offset)};
	if (is_Justify_Left(pText)) {
		MovePosition = true;
	} else if (is_Justify_Aligned(pText) || is_Justify_Fit(pText)) {
		// Both points can be moved
		if (bPlaneChanges) {
			//Move both points
			MovePosition = true;
			MoveAlignmentPoint = true;
		} else {
			for (unsigned i = 0; i < indicesSize; ++i) {
				const auto ind {indices[i]};
				switch (ind) {
					case 0: case 2:
						MovePosition = true;
						break;
					case 1: case 3:
						MoveAlignmentPoint = true;
						break;
				}
			}
		}
	} else {
		MoveAlignmentPoint = true;
	}
	try {
		if (MovePosition) {
			pText->setPosition(pText->position() + offset);
		}
		if (MoveAlignmentPoint) {
			pText->setAlignmentPoint(pText->alignmentPoint() + offset);
		}
		pText->adjustAlignment();
	} catch (const OdError& e) {
		return e.code();
	}
	return eOk;
}

// Stretched
OdResult OdDbTextGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDbTextPtr pText = entity;
	const auto dThickness {pText->thickness()};
	const auto vExtrusion {pText->normal() * dThickness};
	if (is_Justify_Left(pText) || is_Justify_Aligned(pText) || is_Justify_Fit(pText)) {
		stretchPoints.append(pText->position()); // left lower corner of Text
		if (!OdZero(dThickness)) {
			stretchPoints.append(pText->position() + vExtrusion);
		}
	}
	if (!is_Justify_Left(pText)) {
		stretchPoints.append(pText->alignmentPoint());
		// OdDbText (except Justify_Left) always has stretchPoint alignmentPoint
		if (!OdZero(dThickness)) {
			stretchPoints.append(pText->alignmentPoint() + vExtrusion);
		}
	}
	return eOk;
}

OdResult OdDbTextGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

/**
 * \brief  Return snap Points into snapPoints, depending on type on snapMode
 * \param entity 
 * \param objectSnapMode 
 * \param gsSelectionMark 
 * \param pickPoint  Point, which moves
 * \param lastPoint  Point, from which draw line
 * \param xWorldToEye 
 * \param snapPoints 
 * \return 
 */
OdResult OdDbTextGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*gsSelectionMark*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*xWorldToEye*/, OdGePoint3dArray& snapPoints) const {
	OdDbTextPtr Text = entity;
	const auto Thickness {Text->thickness()};
	const auto Extrusion {Text->normal() * Thickness};
	switch (objectSnapMode) {
		case OdDb::kOsModeNode: // Node: cursor as cross in a square
			if (is_Justify_Aligned(Text) || is_Justify_Fit(Text)) {
				snapPoints.append(Text->alignmentPoint());
				if (!OdZero(Thickness)) {
					snapPoints.append(Text->alignmentPoint() + Extrusion);
				}
			} else if (!is_Justify_Left(Text)) {
				snapPoints.append(Text->position());
				if (!OdZero(Thickness)) {
					snapPoints.append(Text->position() + Extrusion);
				}
			}
			break;
		case OdDb::kOsModeIns: // Insertion: cursor as intersection in a square
			if (is_Justify_Left(Text) || is_Justify_Aligned(Text) || is_Justify_Fit(Text)) {
				snapPoints.append(Text->position());
				if (!OdZero(Thickness)) {
					snapPoints.append(Text->position() + Extrusion);
				}
			} else {
				snapPoints.append(Text->alignmentPoint());
				if (!OdZero(Thickness)) {
					snapPoints.append(Text->alignmentPoint() + Extrusion);
				}
			}
			break;
		case OdDb::kOsModeEnd:  // Endpoint: cursor as square
		case OdDb::kOsModeCen:  // Center: draw cross
		case OdDb::kOsModeQuad: // Quadrant: cursor as square turned on 45 degrees
		case OdDb::kOsModePerp: // Perpendicular: cursor as angle 90
		case OdDb::kOsModeNear: // Nearest: cursor ~ hourglasses
		case OdDb::kOsModeTan:  // Tangent
		case OdDb::kOsModeMid:  // Midpoint:     cursor as triangle
		case OdDb::kOsModeIntersec:
			// Intersection: cursor as intersection in a square
		case OdDb::kOsModePar:   // Parallel:
		case OdDb::kOsModeApint: // Apparent intersection:
			break;                 //               isn't necessary to do
		default:
			break;
	}
	return eOk;
}

// Additional service functions
bool OdDbTextGripPointsPE::is_Justify_Left(const OdDbText* pText) const {
	return pText->horizontalMode() == OdDb::kTextLeft &&
		// in AutoCad properties Justify = Left,
		pText->verticalMode() == OdDb::kTextBase;
	// text has only one gripPoint
}

bool OdDbTextGripPointsPE::is_Justify_Aligned(const OdDbText* pText) const {
	return pText->horizontalMode() == OdDb::kTextAlign && // Justify = Aligned,
		pText->verticalMode() == OdDb::kTextBase;
	// we have to do stretch text
}

bool OdDbTextGripPointsPE::is_Justify_Fit(const OdDbText* pText) const {
	return pText->horizontalMode() == OdDb::kTextFit && // Justify = Fit,
		pText->verticalMode() == OdDb::kTextBase;
	// we have to do stretch text
}
