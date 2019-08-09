#include <OdaCommon.h>
#include <DbDiametricDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include "DbDiametricDimGripPoints.h"

OdResult OdDbDiametricDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto GripPointsSize {gripPoints.size()};
	gripPoints.resize(GripPointsSize + 3);
	OdDbDiametricDimensionPtr Dimension {entity};
	gripPoints[GripPointsSize + 0] = Dimension->chordPoint();
	gripPoints[GripPointsSize + 1] = Dimension->farChordPoint();
	gripPoints[GripPointsSize + 2] = Dimension->textPosition();
	return eOk;
}

OdResult OdDbDiametricDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool /*bStretch*/) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbDiametricDimensionPtr Dimension {entity};
	const auto ChordPoint {Dimension->chordPoint()};
	const auto dimFarChordPt {Dimension->farChordPoint()};
	auto TextPosition {Dimension->textPosition()};
	const auto dimMidPt {dimFarChordPt + (ChordPoint - dimFarChordPt) / 2};
	const auto WorldToPlaneTransform {OdGeMatrix3d::worldToPlane(Dimension->normal())};
	auto ocsDimChordPt {ChordPoint};
	auto ocsDimFarChordPt {dimFarChordPt};
	auto ocsDimTextPt {TextPosition};
	auto ocsDimMidPt {dimMidPt};
	const auto vNorm {Dimension->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimChordPt.transformBy(WorldToPlaneTransform);
		ocsDimFarChordPt.transformBy(WorldToPlaneTransform);
		ocsDimTextPt.transformBy(WorldToPlaneTransform);
		ocsDimMidPt.transformBy(WorldToPlaneTransform);
	}
	const auto SavedZCoordinate {ocsDimChordPt.z};
	ocsDimChordPt.z = ocsDimFarChordPt.z = ocsDimTextPt.z = ocsDimMidPt.z = 0.0;
	auto vLen {ocsDimFarChordPt - ocsDimChordPt};
	const auto GripPoint = &gripPoints[indices[0]];
	auto ocsDimNewPt {*GripPoint};
	if (NeedTransform) {
		ocsDimNewPt.transformBy(WorldToPlaneTransform);
	}
	ocsDimNewPt.z = 0.0;
	const auto vX {ocsDimMidPt - ocsDimNewPt};
	const auto vY {ocsDimMidPt - ocsDimChordPt};
	auto angle {vY.angleTo(vX, OdGeVector3d::kZAxis)};
	if (indices[0] == 1) {
		angle += OdaPI;
	}
	vLen.rotateBy(angle, OdGeVector3d::kZAxis);
	if (indices[0] == 1) {
		const auto newLen {dimMidPt.distanceTo(ocsDimNewPt)};
		vLen.normalize();
		vLen *= newLen;
	} else {
		vLen *= 0.5;
	}
	ocsDimChordPt = dimMidPt - vLen;
	ocsDimFarChordPt = dimMidPt + vLen;
	if (OdNonZero(angle)) {
		ocsDimTextPt.rotateBy(angle, OdGeVector3d::kZAxis, dimMidPt);
	}
	if (indices[0] == 2) {
		ocsDimTextPt = ocsDimNewPt;
		Dimension->useSetTextPosition();
		if (ocsDimTextPt.distanceTo(ocsDimFarChordPt) < ocsDimTextPt.distanceTo(ocsDimChordPt)) {
			std::swap(ocsDimFarChordPt, ocsDimChordPt);
		}
	}
	ocsDimFarChordPt.z = SavedZCoordinate;
	ocsDimChordPt.z = SavedZCoordinate;
	ocsDimTextPt.z = SavedZCoordinate;
	if (NeedTransform) {
		ocsDimFarChordPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
		ocsDimChordPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
		ocsDimTextPt.transformBy(OdGeMatrix3d::planeToWorld(Dimension->normal()));
	}
	Dimension->setFarChordPoint(ocsDimFarChordPt);
	Dimension->setChordPoint(ocsDimChordPt);
	Dimension->setTextPosition(ocsDimTextPt);
	return eOk;
}
