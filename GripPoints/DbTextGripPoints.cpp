#include <OdaCommon.h>
#include "DbTextGripPoints.h"
#include <Ge/GeLine3d.h>
#include <AbstractViewPE.h>
#include <DbViewportTableRecord.h>
#include "OdGripPointsModule.h"

// Returns GripPoints
OdResult OdDbTextGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbTextPtr Text {entity};
	gripPoints.append(Text->position()); // left lower corner of Text
	if (!IsJustifyLeft(Text)) { // if in AutoCad properties Justify != Left
		gripPoints.append(Text->alignmentPoint());
	}
	// OdDbText has two grip points: position() and alignmentPoint
	const auto Thickness {Text->thickness()};
	if (!OdZero(Thickness)) {
		const auto Extrusion {Text->normal() * Thickness};
		gripPoints.append(Text->position() + Extrusion);
		if (!IsJustifyLeft(Text)) {
			gripPoints.append(Text->alignmentPoint() + Extrusion);
		}
	}
	return eOk;
}

// Move text
OdResult OdDbTextGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbTextPtr Text {entity};
	auto MovePosition {false};
	auto MoveAlignmentPoint {false};
	auto Offset {offset};
	// Project offset on entity's plane in view direction
	const auto PlaneChanges {!ProjectOffset(Text->database(), Text->normal(), Offset)};
	if (IsJustifyLeft(Text)) {
		MovePosition = true;
	} else if (IsJustifyAligned(Text) || IsJustifyFit(Text)) {
		// Both points can be moved
		if (PlaneChanges) {
			//Move both points
			MovePosition = true;
			MoveAlignmentPoint = true;
		} else {
			for (auto Index : indices) {
				switch (Index) {
					case 0: case 2:
						MovePosition = true;
						break;
					case 1: case 3:
						MoveAlignmentPoint = true;
						break;
					default: ;
				}
			}
		}
	} else {
		MoveAlignmentPoint = true;
	}
	try {
		if (MovePosition) {
			Text->setPosition(Text->position() + Offset);
		}
		if (MoveAlignmentPoint) {
			Text->setAlignmentPoint(Text->alignmentPoint() + Offset);
		}
		Text->adjustAlignment();
	} catch (const OdError& e) {
		return e.code();
	}
	return eOk;
}

// Stretched
OdResult OdDbTextGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	OdDbTextPtr Text {entity};
	const auto Thickness {Text->thickness()};
	const auto Extrusion {Text->normal() * Thickness};
	if (IsJustifyLeft(Text) || IsJustifyAligned(Text) || IsJustifyFit(Text)) {
		stretchPoints.append(Text->position()); // left lower corner of Text
		if (!OdZero(Thickness)) {
			stretchPoints.append(Text->position() + Extrusion);
		}
	}
	if (!IsJustifyLeft(Text)) {
		stretchPoints.append(Text->alignmentPoint());
		// OdDbText (except Justify_Left) always has stretchPoint alignmentPoint
		if (!OdZero(Thickness)) {
			stretchPoints.append(Text->alignmentPoint() + Extrusion);
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
 * \param
 * \param
 * \param
 * \param
 * \param snapPoints 
 * \return 
 */
OdResult OdDbTextGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& snapPoints) const {
	OdDbTextPtr Text {entity};
	const auto Thickness {Text->thickness()};
	const auto Extrusion {Text->normal() * Thickness};
	switch (objectSnapMode) {
		case OdDb::kOsModeNode: // Node: cursor as cross in a square
			if (IsJustifyAligned(Text) || IsJustifyFit(Text)) {
				snapPoints.append(Text->alignmentPoint());
				if (!OdZero(Thickness)) {
					snapPoints.append(Text->alignmentPoint() + Extrusion);
				}
			} else if (!IsJustifyLeft(Text)) {
				snapPoints.append(Text->position());
				if (!OdZero(Thickness)) {
					snapPoints.append(Text->position() + Extrusion);
				}
			}
			break;
		case OdDb::kOsModeIns: // Insertion: cursor as intersection in a square
			if (IsJustifyLeft(Text) || IsJustifyAligned(Text) || IsJustifyFit(Text)) {
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
		case OdDb::kOsModeEnd: // Endpoint: cursor as square
		case OdDb::kOsModeCen: // Center: draw cross
		case OdDb::kOsModeQuad: // Quadrant: cursor as square turned on 45 degrees
		case OdDb::kOsModePerp: // Perpendicular: cursor as angle 90
		case OdDb::kOsModeNear: // Nearest: cursor ~ hourglasses
		case OdDb::kOsModeTan: // Tangent
		case OdDb::kOsModeMid: // Midpoint: cursor as triangle
		case OdDb::kOsModeIntersec: // Intersection: cursor as intersection in a square
		case OdDb::kOsModePar: // Parallel:
		case OdDb::kOsModeApint: // Apparent intersection:
			break;
		default:
			break;
	}
	return eOk;
}

bool OdDbTextGripPointsPE::IsJustifyLeft(const OdDbText* text) {
	return text->horizontalMode() == OdDb::kTextLeft && text->verticalMode() == OdDb::kTextBase;
}

bool OdDbTextGripPointsPE::IsJustifyAligned(const OdDbText* text) {
	return text->horizontalMode() == OdDb::kTextAlign && text->verticalMode() == OdDb::kTextBase;
}

bool OdDbTextGripPointsPE::IsJustifyFit(const OdDbText* text) {
	return text->horizontalMode() == OdDb::kTextFit && text->verticalMode() == OdDb::kTextBase;
}
