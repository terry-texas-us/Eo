#include <OdaCommon.h>
#include "DbDimGripPoints.h"
#include <DbRotatedDimension.h>
#include <DbAlignedDimension.h>
#include <DbRadialDimension.h>
#include <DbDiametricDimension.h>
#include <Db3PointAngularDimension.h>
#include <DbOrdinateDimension.h>
#include <Db2LineAngularDimension.h>
#include <DbRadialDimensionLarge.h>
#include <DbArcDimension.h>
#include <Ge/GeCircArc3d.h>
#include <Ge/GeMatrix3d.h>
#include <Ge/GeLine2d.h>
#include <Ge/GeLineSeg3d.h>
#define STL_USING_ALGORITHM
#define STL_USING_UTILITY
#include <OdaSTL.h>

OdResult OdDbRotatedDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbRotatedDimensionPtr pDimPtr = entity;
	const auto size {gripPoints.size()};
	if (pDimPtr->jogSymbolOn()) {
		gripPoints.resize(size + 6);
	} else {
		gripPoints.resize(size + 5);
	}
	const auto defPoint1 {pDimPtr->xLine1Point()};
	const auto defPoint2 {pDimPtr->xLine2Point()};
	const auto dimLineDefPt {pDimPtr->dimLinePoint()};
	const auto dimLineText {pDimPtr->textPosition()};
	const auto dRotAngle {pDimPtr->rotation()};
	auto Oblique {pDimPtr->oblique()};
	const auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
	auto ocsDefPt1 {defPoint1};
	auto ocsDimLinDefPt {dimLineDefPt};
	const auto vNorm {pDimPtr->normal()};
	auto bNeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		bNeedTransform = true;
		ocsDefPt1.transformBy(world2Plane);
		ocsDimLinDefPt.transformBy(world2Plane);
	}
	const auto savedZCoordinate {ocsDefPt1.z};
	ocsDefPt1.z = ocsDimLinDefPt.z = 0.0;
	auto v1 {OdGeVector3d::kXAxis};
	v1.rotateBy(dRotAngle, OdGeVector3d::kZAxis);
	auto v2 {OdGeVector3d::kYAxis};
	if (OdNonZero(Oblique)) {
		Oblique = Oblique - OdaPI2;
	}
	v2.rotateBy(dRotAngle + Oblique, OdGeVector3d::kZAxis);
	OdGeLine3d line1;
	OdGeLine3d line2;
	line1.set(ocsDimLinDefPt, v1);
	line2.set(ocsDefPt1, v2);
	OdGePoint3d extLineEnd;
	line1.intersectWith(line2, extLineEnd);
	extLineEnd.z = savedZCoordinate;
	if (bNeedTransform) {
		extLineEnd.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
	}
	gripPoints[size + 0] = defPoint1;
	gripPoints[size + 1] = defPoint2;
	gripPoints[size + 2] = extLineEnd;
	gripPoints[size + 3] = dimLineDefPt;
	gripPoints[size + 4] = dimLineText;
	if (pDimPtr->jogSymbolOn()) {
		gripPoints[size + 5] = pDimPtr->jogSymbolPosition();
	}
	return eOk;
}

OdResult OdDbRotatedDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbRotatedDimensionPtr pDimPtr = entity;
	const OdGePoint3d* pGripPoint3d; // = &gripPoints[indices[0]];
	OdGePoint3d dimNewPt;            // = *pGripPoint3d;
	auto savedZCoordinate {0.0};
	OdGeVector3d vMoveTxt;
	auto vNorm {pDimPtr->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
	}
	for (auto i = 0; i < (int)indices.size(); i++) {
		pGripPoint3d = &gripPoints[indices[i]];
		dimNewPt = *pGripPoint3d;
		switch (indices[i]) {
			case 0:
				pDimPtr->setXLine1Point(dimNewPt);
				break;
			case 1:
				pDimPtr->setXLine2Point(dimNewPt);
				break;
			case 2: {
				auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
				auto ocsDimNewPt(dimNewPt);
				auto dimLinePt {pDimPtr->dimLinePoint()};
				auto ocsDimLinePt(dimLinePt);
				auto defPoint1 {pDimPtr->xLine1Point()};
				auto ocsDefPt1(defPoint1);
				if (NeedTransform) {
					ocsDimNewPt.transformBy(world2Plane);
					ocsDimLinePt.transformBy(world2Plane);
					ocsDefPt1.transformBy(world2Plane);
				}
				savedZCoordinate = ocsDefPt1.z;
				ocsDimNewPt.z = 0.0;
				ocsDimLinePt.z = 0.0;
				ocsDefPt1.z = 0.0;
				auto Rotation {pDimPtr->rotation()};
				auto Oblique {pDimPtr->oblique()};
				auto v1 {OdGeVector3d::kXAxis};
				v1.rotateBy(Rotation, OdGeVector3d::kZAxis);
				auto v2 {OdGeVector3d::kYAxis};
				if (OdNonZero(Oblique)) {
					Oblique = Oblique - OdaPI2;
				}
				v2.rotateBy(Rotation + Oblique, OdGeVector3d::kZAxis);
				OdGeLine3d line1;
				OdGeLine3d line2;
				line1.set(ocsDimLinePt, v1);
				line2.set(ocsDefPt1, v2);
				OdGePoint3d extLineEnd;
				line1.intersectWith(line2, extLineEnd);
				vMoveTxt = extLineEnd - ocsDimNewPt;
				line1.set(ocsDimNewPt, v1);
				line2.set(ocsDimLinePt, v2);
				line1.intersectWith(line2, ocsDimNewPt);
				dimNewPt = ocsDimNewPt;
				dimNewPt.z = savedZCoordinate;
				if (NeedTransform) {
					dimNewPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
				}
			}
			case 3:
				if (!pDimPtr->isUsingDefaultTextPosition() && pDimPtr->dimtmove() == 0) {
					auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
					if (indices[0] == 3) {
						auto ocsDimLinePt(pDimPtr->dimLinePoint());
						auto ocsDimNewPt {dimNewPt};
						if (NeedTransform) {
							ocsDimLinePt.transformBy(world2Plane);
							ocsDimNewPt.transformBy(world2Plane);
						}
						ocsDimLinePt.z = 0.0;
						ocsDimNewPt.z = 0.0;
						vMoveTxt = ocsDimLinePt - ocsDimNewPt;
					}
					auto dimTextPt {gripPoints[4]};
					auto ocsDimTextPt {dimTextPt};
					if (NeedTransform) {
						ocsDimTextPt.transformBy(world2Plane);
					}
					savedZCoordinate = ocsDimTextPt.z;
					ocsDimTextPt.z = 0.0;
					auto ocsDimNewTextPt {ocsDimTextPt - vMoveTxt};
					auto dimNewTextPt {ocsDimNewTextPt};
					dimNewTextPt.z = savedZCoordinate;
					if (NeedTransform) {
						dimNewTextPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
					}
					pDimPtr->setTextPosition(dimNewTextPt);
				} else {
					pDimPtr->setDimLinePoint(dimNewPt);
				}
				break;
			case 4:
				pDimPtr->setTextPosition(dimNewPt);
				if (indices.size() == 1 || !bStretch) {
					pDimPtr->useSetTextPosition();
				}
				break;
			case 5:
				if (!pDimPtr->isUsingDefaultTextPosition() && pDimPtr->dimtmove() == 0) {
					auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
					auto ocsDimJogPt(pDimPtr->jogSymbolPosition());
					auto ocsDimNewPt {dimNewPt};
					auto p1 {pDimPtr->xLine2Point()};
					auto p2 {pDimPtr->dimLinePoint()};
					if (NeedTransform) {
						ocsDimJogPt.transformBy(world2Plane);
						ocsDimNewPt.transformBy(world2Plane);
						p1.transformBy(world2Plane);
						p2.transformBy(world2Plane);
					}
					ocsDimJogPt.z = 0.0;
					ocsDimNewPt.z = 0.0;
					p1.z = p2.z = 0.0;
					auto v {p2 - p1};
					vMoveTxt = ocsDimJogPt - ocsDimNewPt;
					if (v.length()) {
						v.normalize();
					}
					v *= vMoveTxt.length() * cos(v.angleTo(vMoveTxt));
					auto dimTextPt {gripPoints[4]};
					auto ocsDimTextPt {dimTextPt};
					if (NeedTransform) {
						ocsDimTextPt.transformBy(world2Plane);
					}
					savedZCoordinate = ocsDimTextPt.z;
					ocsDimTextPt.z = 0.0;
					auto ocsDimNewTextPt {ocsDimTextPt - v};
					auto dimNewTextPt {ocsDimNewTextPt};
					dimNewTextPt.z = savedZCoordinate;
					if (NeedTransform) {
						dimNewTextPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
					}
					pDimPtr->setTextPosition(dimNewTextPt);
				} else {
					pDimPtr->setDimLinePoint(dimNewPt);
				}
				pDimPtr->setJogSymbolPosition(dimNewPt);
				break;
			default:
				break;
		}
	}
	return eOk;
}

OdResult OdDbAlignedDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbAlignedDimensionPtr pDimPtr = entity;
	const auto size {gripPoints.size()};
	if (pDimPtr->jogSymbolOn()) {
		gripPoints.resize(size + 6);
	} else {
		gripPoints.resize(size + 5);
	}
	const auto defPoint1 {pDimPtr->xLine1Point()};
	const auto defPoint2 {pDimPtr->xLine2Point()};
	const auto dimLineDefPt {pDimPtr->dimLinePoint()};
	const auto dimLineText {pDimPtr->textPosition()};
	auto dOblique {pDimPtr->oblique()};
	const auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
	auto ocsDefPt1 {defPoint1};
	auto ocsDefPt2 {defPoint2};
	auto ocsDimLinDefPt {dimLineDefPt};
	const auto vNorm {pDimPtr->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDefPt1.transformBy(world2Plane);
		ocsDefPt2.transformBy(world2Plane);
		ocsDimLinDefPt.transformBy(world2Plane);
	}
	const auto SavedZCoordinate {ocsDefPt1.z};
	ocsDefPt1.z = ocsDefPt1.z = ocsDimLinDefPt.z = 0.0;
	auto v1 {OdGeVector3d::kXAxis};
	auto v2 {OdGeVector3d::kYAxis};
	if (OdNonZero(dOblique)) {
		dOblique = Oda2PI - dOblique;
	} else {
		dOblique = OdaPI2;
	}
	v2 = ocsDimLinDefPt - ocsDefPt2;
	if (OdZero(v2.length())) {
		v2 = OdGeVector3d::kYAxis;
		const auto dlv {defPoint2 - defPoint1};
		if (OdNonZero(dlv.length())) {
			v2 = dlv.perpVector();
		}
	}
	v1 = v2;
	v1.rotateBy(dOblique, OdGeVector3d::kZAxis);
	OdGeLine3d line1;
	OdGeLine3d line2;
	line1.set(ocsDimLinDefPt, v1);
	line2.set(ocsDefPt1, v2);
	OdGePoint3d extLineEnd;
	line1.intersectWith(line2, extLineEnd);
	extLineEnd.z = SavedZCoordinate;
	if (NeedTransform) {
		extLineEnd.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
	}
	gripPoints[size + 0] = defPoint1;
	gripPoints[size + 1] = defPoint2;
	gripPoints[size + 2] = extLineEnd;
	gripPoints[size + 3] = dimLineDefPt;
	gripPoints[size + 4] = dimLineText;
	if (pDimPtr->jogSymbolOn()) {
		gripPoints[size + 5] = pDimPtr->jogSymbolPosition();
	}
	return eOk;
}

OdResult OdDbAlignedDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbAlignedDimensionPtr pDimPtr = entity;
	auto pGripPoint3d {&gripPoints[indices[0]]};
	auto dimNewPt {*pGripPoint3d};
	auto SavedZCoordinate {0.0};
	OdGeVector3d vMoveTxt;
	auto vNorm {pDimPtr->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
	}
	for (auto i = 0; i < (int)indices.size(); i++) {
		pGripPoint3d = &gripPoints[indices[i]];
		dimNewPt = *pGripPoint3d;
		switch (indices[i]) {
			case 0:
				pDimPtr->setXLine1Point(dimNewPt);
				break;
			case 1:
				pDimPtr->setXLine2Point(dimNewPt);
				break;
			case 2: {
				auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
				auto ocsDimNewPt(dimNewPt);
				auto xLinePt1 {pDimPtr->xLine1Point()};
				auto xLinePt2 {pDimPtr->xLine2Point()};
				auto dimLinePt {pDimPtr->dimLinePoint()};
				auto ocsXLinePt1(xLinePt1);
				auto ocsXLinePt2(xLinePt2);
				auto ocsDimLinePt(dimLinePt);
				if (NeedTransform) {
					ocsDimNewPt.transformBy(world2Plane);
					ocsXLinePt1.transformBy(world2Plane);
					ocsXLinePt2.transformBy(world2Plane);
					ocsDimLinePt.transformBy(world2Plane);
				}
				auto SavedZCoordinate {ocsXLinePt1.z};
				ocsDimNewPt.z = 0.0;
				ocsXLinePt1.z = ocsXLinePt2.z = ocsDimLinePt.z = 0.0;
				auto Oblique {pDimPtr->oblique()};
				auto v1 {OdGeVector3d::kXAxis};
				auto v2 {OdGeVector3d::kYAxis};
				if (!OdNonZero(Oblique)) {
					Oblique = OdaPI2;
				}
				v1 = v2 = ocsXLinePt2 - ocsXLinePt1;
				v2.rotateBy(Oblique, OdGeVector3d::kZAxis);
				OdGeLine3d line1;
				OdGeLine3d line2;
				line1.set(ocsDimLinePt, v1);
				line2.set(ocsXLinePt1, v2);
				OdGePoint3d extLineEnd;
				line1.intersectWith(line2, extLineEnd);
				vMoveTxt = extLineEnd - ocsDimNewPt;
				line1.set(ocsDimNewPt, v1);
				line2.set(ocsDimLinePt, v2);
				line1.intersectWith(line2, ocsDimLinePt);
				dimNewPt = ocsDimLinePt;
				dimNewPt.z = SavedZCoordinate;
				if (NeedTransform) {
					dimNewPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
				}
			}
			case 3:
				if (!pDimPtr->isUsingDefaultTextPosition() && pDimPtr->dimtmove() == 0) {
					auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
					if (indices[0] == 3) {
						auto ocsDimLinePt(pDimPtr->dimLinePoint());
						auto ocsDimNewPt {dimNewPt};
						if (NeedTransform) {
							ocsDimLinePt.transformBy(world2Plane);
							ocsDimLinePt.transformBy(world2Plane);
						}
						ocsDimNewPt.z = 0.0;
						ocsDimNewPt.z = 0.0;
						vMoveTxt = ocsDimLinePt - ocsDimNewPt;
					}
					auto dimTextPt {gripPoints[4]};
					auto ocsDimTextPt {dimTextPt};
					if (NeedTransform) {
						ocsDimTextPt.transformBy(world2Plane);
					}
					auto SavedZCoordinate {ocsDimTextPt.z};
					ocsDimTextPt.z = 0.0;
					auto ocsDimNewTextPt {ocsDimTextPt - vMoveTxt};
					auto dimNewTextPt {ocsDimNewTextPt};
					dimNewTextPt.z = SavedZCoordinate;
					if (NeedTransform) {
						dimNewTextPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
					}
					pDimPtr->setTextPosition(dimNewTextPt);
				} else {
					pDimPtr->setDimLinePoint(dimNewPt);
				}
				break;
			case 4:
				pDimPtr->setTextPosition(dimNewPt);
				if (indices.size() == 1 || !bStretch) {
					pDimPtr->useSetTextPosition();
				}
				break;
			case 5:
				if (!pDimPtr->isUsingDefaultTextPosition() && pDimPtr->dimtmove() == 0) {
					auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
					auto ocsDimJogPt(pDimPtr->jogSymbolPosition());
					auto ocsDimNewPt {dimNewPt};
					auto p1 {pDimPtr->xLine2Point()};
					auto p2 {pDimPtr->dimLinePoint()};
					if (NeedTransform) {
						ocsDimJogPt.transformBy(world2Plane);
						ocsDimNewPt.transformBy(world2Plane);
						p1.transformBy(world2Plane);
						p2.transformBy(world2Plane);
					}
					ocsDimJogPt.z = 0.0;
					ocsDimNewPt.z = 0.0;
					p1.z = p2.z = 0.0;
					auto v {p2 - p1};
					vMoveTxt = ocsDimJogPt - ocsDimNewPt;
					if (v.length()) {
						v.normalize();
					}
					v *= vMoveTxt.length() * cos(v.angleTo(vMoveTxt));
					auto dimTextPt {gripPoints[4]};
					auto ocsDimTextPt {dimTextPt};
					if (NeedTransform) {
						ocsDimTextPt.transformBy(world2Plane);
					}
					SavedZCoordinate = ocsDimTextPt.z;
					ocsDimTextPt.z = 0.0;
					auto ocsDimNewTextPt {ocsDimTextPt - v};
					auto dimNewTextPt {ocsDimNewTextPt};
					dimNewTextPt.z = SavedZCoordinate;
					if (NeedTransform) {
						dimNewTextPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
					}
					pDimPtr->setTextPosition(dimNewTextPt);
				} else {
					pDimPtr->setDimLinePoint(dimNewPt);
				}
				pDimPtr->setJogSymbolPosition(dimNewPt);
				break;
			default:
				break;
		}
	}
	return eOk;
}

OdResult OdDbRadialDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto size {gripPoints.size()};
	gripPoints.resize(size + 3);
	OdDbRadialDimensionPtr pDimPtr = entity;
	gripPoints[size + 0] = pDimPtr->center();
	gripPoints[size + 1] = pDimPtr->chordPoint();
	gripPoints[size + 2] = pDimPtr->textPosition();
	return eOk;
}

OdResult OdDbRadialDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool /*bStretch*/) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbRadialDimensionPtr pDimPtr = entity;
	const auto pGripPoint3d {&gripPoints[indices[0]]};
	auto dimNewPt {*pGripPoint3d};
	const auto vNorm {pDimPtr->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
	}
	switch (indices[0]) {
		case 0: {
			// radius is constant
			// chord point is calculated using center and text points
			// text point may be changed in recompute dimension block
			const auto dimCenterPt {pDimPtr->center()};
			auto dimChordPt {pDimPtr->chordPoint()};
			const auto radius {dimCenterPt.distanceTo(dimChordPt)};
			const auto textPt {pDimPtr->textPosition()};
			auto v {textPt - dimNewPt};
			v.normalize();
			dimChordPt = dimNewPt + v * radius;
			pDimPtr->setChordPoint(dimChordPt);
			pDimPtr->setCenter(dimNewPt);
			break;
		}
		case 1: {
			// radius is constant, center point is constant
			// text points is calculated and may be changed in recompute dimension block
			const auto dimCenterPt {pDimPtr->center()};
			auto dimChordPt {pDimPtr->chordPoint()};
			// double dist = dimChordPt.distanceTo(pDimPtr->textPosition());
			const auto dist {dimCenterPt.distanceTo(pDimPtr->textPosition())};
			const auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
			auto ocsDimCenterPt {dimCenterPt};
			auto ocsDimChordPt {dimChordPt};
			OdGeLine3d line1;
			OdGeLine3d line2;
			auto ocsDimNewPt(dimNewPt);
			if (NeedTransform) {
				ocsDimCenterPt.transformBy(world2Plane);
				ocsDimChordPt.transformBy(world2Plane);
				ocsDimNewPt.transformBy(world2Plane);
			}
			const auto SavedZCoordinate {ocsDimCenterPt.z};
			ocsDimCenterPt.z = ocsDimChordPt.z = 0.0;
			ocsDimNewPt.z = 0.0;
			const auto vX {ocsDimCenterPt - ocsDimNewPt};
			auto vY {ocsDimCenterPt - ocsDimChordPt};
			const auto angle {vY.angleTo(vX, OdGeVector3d::kZAxis)};
			// if( !angle ) return eOk;
			vY.rotateBy(Oda2PI - (OdaPI - angle) / 2, OdGeVector3d::kZAxis);
			line1.set(ocsDimChordPt, vY);
			line2.set(ocsDimCenterPt, vX);
			line1.intersectWith(line2, ocsDimChordPt);
			dimNewPt = ocsDimChordPt;
			dimNewPt.z = SavedZCoordinate;
			if (NeedTransform) {
				dimNewPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
			}
			auto v {dimNewPt - dimCenterPt};
			v.normalize();
			const auto textPt {dimCenterPt + v * dist};
			pDimPtr->setTextPosition(textPt);
			pDimPtr->setChordPoint(dimNewPt);
			break;
		}
		case 2: {
			// radius is constant, center point is constant
			// chord point is calculated using center and text points
			const auto dimCenterPt {pDimPtr->center()};
			auto dimChordPt {pDimPtr->chordPoint()};
			const auto radius {dimCenterPt.distanceTo(dimChordPt)};
			const auto textPt {dimNewPt};
			auto v {textPt - dimCenterPt};
			v.normalize();
			dimChordPt = dimCenterPt + v * radius;
			pDimPtr->setChordPoint(dimChordPt);
			pDimPtr->setTextPosition(dimNewPt); //pDimPtr->useSetTextPosition();
			break;
		}
		default:
			break;
	}
	return eOk;
}

OdResult OdDbDiametricDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto size {gripPoints.size()};
	gripPoints.resize(size + 3);
	OdDbDiametricDimensionPtr pDimPtr = entity;
	gripPoints[size + 0] = pDimPtr->chordPoint();
	gripPoints[size + 1] = pDimPtr->farChordPoint();
	gripPoints[size + 2] = pDimPtr->textPosition();
	return eOk;
}

OdResult OdDbDiametricDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool /*bStretch*/) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbDiametricDimensionPtr pDimPtr = entity;
	const auto dimChordPt {pDimPtr->chordPoint()};
	const auto dimFarChordPt {pDimPtr->farChordPoint()};
	auto dimTextPt {pDimPtr->textPosition()};
	const auto dimMidPt {dimFarChordPt + (dimChordPt - dimFarChordPt) / 2};
	const auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
	auto ocsDimChordPt {dimChordPt};
	auto ocsDimFarChordPt {dimFarChordPt};
	auto ocsDimTextPt {dimTextPt};
	auto ocsDimMidPt {dimMidPt};
	const auto vNorm {pDimPtr->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimChordPt.transformBy(world2Plane);
		ocsDimFarChordPt.transformBy(world2Plane);
		ocsDimTextPt.transformBy(world2Plane);
		ocsDimMidPt.transformBy(world2Plane);
	}
	const auto SavedZCoordinate {ocsDimChordPt.z};
	ocsDimChordPt.z = ocsDimFarChordPt.z = ocsDimTextPt.z = ocsDimMidPt.z = 0.0;
	auto vLen {ocsDimFarChordPt - ocsDimChordPt};
	const OdGePoint3d* pGripPoint3d = nullptr;
	OdGeLine3d line1, line2, lineText;
	pGripPoint3d = &gripPoints[indices[0]];
	auto ocsDimNewPt {*pGripPoint3d};
	auto dimNewPt {ocsDimNewPt};
	if (NeedTransform) {
		ocsDimNewPt.transformBy(world2Plane);
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
		pDimPtr->useSetTextPosition();
		if (ocsDimTextPt.distanceTo(ocsDimFarChordPt) < ocsDimTextPt.distanceTo(ocsDimChordPt)) {
			std::swap(ocsDimFarChordPt, ocsDimChordPt);
		}
	}
	ocsDimFarChordPt.z = SavedZCoordinate;
	ocsDimChordPt.z = SavedZCoordinate;
	ocsDimTextPt.z = SavedZCoordinate;
	if (NeedTransform) {
		ocsDimFarChordPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
		ocsDimChordPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
		ocsDimTextPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
	}
	pDimPtr->setFarChordPoint(ocsDimFarChordPt);
	pDimPtr->setChordPoint(ocsDimChordPt);
	pDimPtr->setTextPosition(ocsDimTextPt);
	return eOk;
}

OdResult OdDb3PointAngularDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto size {gripPoints.size()};
	gripPoints.resize(size + 5);
	OdDb3PointAngularDimensionPtr pDimPtr = entity;
	gripPoints[size + 0] = pDimPtr->xLine1Point();
	gripPoints[size + 1] = pDimPtr->xLine2Point();
	gripPoints[size + 2] = pDimPtr->centerPoint();
	gripPoints[size + 3] = pDimPtr->arcPoint();
	gripPoints[size + 4] = pDimPtr->textPosition();
	return eOk;
}

OdResult OdDb3PointAngularDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDb3PointAngularDimensionPtr pDimPtr = entity;
	auto dimLine1Pt {pDimPtr->xLine1Point()};
	auto dimLine2Pt {pDimPtr->xLine2Point()};
	auto dimCenterPt {pDimPtr->centerPoint()};
	auto dimArcPt {pDimPtr->arcPoint()};
	auto dimTextPt {pDimPtr->textPosition()};
	auto dimArcNewPt {dimArcPt};
	auto world2Plane(OdGeMatrix3d::worldToPlane(pDimPtr->normal()));
	auto ocsDimLine1Pt {dimLine1Pt};
	auto ocsDimLine2Pt {dimLine2Pt};
	auto ocsDimCenterPt {dimCenterPt};
	auto ocsDimArcPt {dimArcPt};
	auto ocsDimTextPt {dimTextPt};
	auto ocsDimArcNewPt {dimArcNewPt};
	auto vNorm {pDimPtr->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimLine1Pt.transformBy(world2Plane);
		ocsDimLine2Pt.transformBy(world2Plane);
		ocsDimCenterPt.transformBy(world2Plane);
		ocsDimArcPt.transformBy(world2Plane);
		ocsDimTextPt.transformBy(world2Plane);
		ocsDimTextPt.transformBy(world2Plane);
		ocsDimArcNewPt.transformBy(world2Plane);
	}
	auto SavedZCoordinate {ocsDimLine1Pt.z};
	ocsDimLine1Pt.z = ocsDimLine2Pt.z = ocsDimCenterPt.z = ocsDimArcPt.z = ocsDimTextPt.z = ocsDimArcNewPt.z = 0.0;
	OdGeVector3d vX1;
	OdGeVector3d vX2;
	OdGeVector3d vArc;
	OdGeVector3d vTxt;
	const OdGePoint3d* pGripPoint3d = nullptr;
	OdGeLine3d line1, line2, lineText;
	pGripPoint3d = &gripPoints[indices[0]];
	auto ocsDimNewPt {*pGripPoint3d};
	auto dimNewPt {ocsDimNewPt};
	if (NeedTransform) {
		ocsDimNewPt.transformBy(world2Plane);
	}
	ocsDimNewPt.z = 0.0;
	for (auto i = 0; i < (int)indices.size(); i++) {
		pGripPoint3d = &gripPoints[indices[i]];
		dimNewPt = *pGripPoint3d;
		ocsDimNewPt = dimNewPt;
		if (indices[i] < 4 && !pDimPtr->isUsingDefaultTextPosition()) {
			pDimPtr->useDefaultTextPosition();
		}
		switch (indices[i]) {
			case 0:
				pDimPtr->setXLine1Point(dimNewPt);
				break;
			case 1:
				pDimPtr->setXLine2Point(dimNewPt);
				break;
			case 2:
				pDimPtr->setCenterPoint(dimNewPt);
				break;
			case 4: {
				auto v4 {ocsDimCenterPt - ocsDimArcNewPt};
				ocsDimArcNewPt = ocsDimCenterPt - v4;
				ocsDimTextPt = ocsDimNewPt;
				ocsDimTextPt.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsDimTextPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
				}
				if (indices.size() == 1 || !bStretch) {
					pDimPtr->useSetTextPosition();
				}
				pDimPtr->setTextPosition(ocsDimTextPt);
				break;
			}
			case 3: {
				pDimPtr->setArcPoint(dimNewPt);
				break;
			}
			default:
				break;
		}
	}
	return eOk;
}

OdResult OdDbOrdinateDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto size {gripPoints.size()};
	gripPoints.resize(size + 4);
	OdDbOrdinateDimensionPtr pDimPtr = entity;
	gripPoints[size + 0] = pDimPtr->definingPoint();
	gripPoints[size + 1] = pDimPtr->leaderEndPoint();
	gripPoints[size + 2] = pDimPtr->origin();
	gripPoints[size + 3] = pDimPtr->textPosition();
	return eOk;
}

OdResult OdDbOrdinateDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool /*bStretch*/) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbOrdinateDimensionPtr pDimPtr = entity;
	auto dimDefiningPt {pDimPtr->definingPoint()};
	auto dimLeaderEndPt {pDimPtr->leaderEndPoint()};
	auto dimOriginPt {pDimPtr->origin()};
	auto dimTextPt {pDimPtr->textPosition()};
	const auto world2Plane {OdGeMatrix3d::worldToPlane(pDimPtr->normal())};
	auto ocsDimDefiningPt {dimDefiningPt};
	auto ocsDimLeaderEndPt {dimLeaderEndPt};
	auto ocsDimTextPt {dimTextPt};
	const auto vNorm {pDimPtr->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimDefiningPt.transformBy(world2Plane);
		ocsDimLeaderEndPt.transformBy(world2Plane);
		ocsDimTextPt.transformBy(world2Plane);
	}
	const auto SavedZCoordinate {ocsDimDefiningPt.z};
	ocsDimDefiningPt.z = ocsDimLeaderEndPt.z = ocsDimTextPt.z = 0.0;
	const OdGePoint3d* pGripPoint3d = nullptr;
	pGripPoint3d = &gripPoints[indices[0]];
	auto ocsDimNewPt {*pGripPoint3d};
	auto dimNewPt {ocsDimNewPt};
	if (NeedTransform) {
		ocsDimNewPt.transformBy(world2Plane);
	}
	ocsDimNewPt.z = 0.0;
	auto v1 {ocsDimTextPt - ocsDimLeaderEndPt};
	for (auto i = 0; i < (int)indices.size(); i++) {
		pGripPoint3d = &gripPoints[indices[i]];
		dimNewPt = *pGripPoint3d;
		ocsDimNewPt = dimNewPt;
		switch (indices[i]) {
			case 0:
				pDimPtr->setDefiningPoint(dimNewPt);
				break;
			case 1:
				v1.normalize();
				v1 *= OdGeVector3d(ocsDimTextPt - ocsDimLeaderEndPt).length();
				ocsDimTextPt = ocsDimNewPt + v1;
				ocsDimTextPt.z = SavedZCoordinate;
				ocsDimNewPt.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsDimTextPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
					ocsDimNewPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
				}
				if (!pDimPtr->isUsingDefaultTextPosition() && pDimPtr->dimtmove() == 2) {
					pDimPtr->setLeaderEndPoint(ocsDimNewPt);
				} else {
					pDimPtr->setTextPosition(ocsDimTextPt);
				}
				break;
			case 2:
				pDimPtr->setOrigin(dimNewPt);
				break;
			case 3:
				v1.normalize();
				v1 *= OdGeVector3d(ocsDimTextPt - ocsDimLeaderEndPt).length();
				ocsDimLeaderEndPt = ocsDimNewPt - v1;
				ocsDimNewPt.z = SavedZCoordinate;
				ocsDimLeaderEndPt.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsDimNewPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
					ocsDimLeaderEndPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
				}
				pDimPtr->setTextPosition(ocsDimNewPt); //pDimPtr->useSetTextPosition();
				if (pDimPtr->isUsingDefaultTextPosition() && pDimPtr->dimtmove() != 2) {
					pDimPtr->setLeaderEndPoint(ocsDimLeaderEndPt);
				}
				break;
		}
	}
	return eOk;
}

OdResult OdDb2LineAngularDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto size {gripPoints.size()};
	gripPoints.resize(size + 6);
	OdDb2LineAngularDimensionPtr pDimPtr = entity;
	gripPoints[size + 0] = pDimPtr->xLine1Start();
	gripPoints[size + 1] = pDimPtr->xLine1End();
	gripPoints[size + 2] = pDimPtr->xLine2Start();
	gripPoints[size + 3] = pDimPtr->xLine2End();
	gripPoints[size + 4] = pDimPtr->arcPoint();
	gripPoints[size + 5] = pDimPtr->textPosition();
	return eOk;
}

OdResult OdDb2LineAngularDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDb2LineAngularDimensionPtr pDimPtr = entity;
	auto dimLine1StartPt {pDimPtr->xLine1Start()};
	auto dimLine1EndPt {pDimPtr->xLine1End()};
	auto dimLine2StartPt {pDimPtr->xLine2Start()};
	auto dimLine2EndPt {pDimPtr->xLine2End()};
	auto dimArcPt {pDimPtr->arcPoint()};
	auto dimTextPt {pDimPtr->textPosition()};
	OdGePoint3d dimCenterPt;
	auto dimArcNewPt {dimArcPt};
	OdGePoint3d dimNewTextPt;
	auto world2Plane(OdGeMatrix3d::worldToPlane(pDimPtr->normal()));
	auto ocsDimLine1StartPt {dimLine1StartPt};
	auto ocsDimLine1EndPt {dimLine1EndPt};
	auto ocsDimLine2StartPt {dimLine2StartPt};
	auto ocsDimLine2EndPt {dimLine2EndPt};
	auto ocsDimArcPt {dimArcPt};
	auto ocsDimTextPt {dimTextPt};
	auto ocsDimArcNewPt {dimArcNewPt};
	auto vNorm {pDimPtr->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimLine1StartPt.transformBy(world2Plane);
		ocsDimLine1EndPt.transformBy(world2Plane);
		ocsDimLine2StartPt.transformBy(world2Plane);
		ocsDimLine2EndPt.transformBy(world2Plane);
		ocsDimArcPt.transformBy(world2Plane);
		ocsDimTextPt.transformBy(world2Plane);
		ocsDimArcNewPt.transformBy(world2Plane);
	}
	auto SavedZCoordinate {ocsDimLine1StartPt.z};
	ocsDimLine1StartPt.z = ocsDimLine1EndPt.z = ocsDimLine2StartPt.z = ocsDimLine2EndPt.z = ocsDimArcPt.z = ocsDimTextPt.z = ocsDimArcNewPt.z = 0.0;
	OdGeVector3d vX1, vX2, vArc, vTxt;
	const OdGePoint3d* pGripPoint3d = nullptr;
	OdGeLine3d line1, line2, lineText;
	pGripPoint3d = &gripPoints[indices[0]];
	OdGePoint3d ocsDimNewPt; // = *pGripPoint3d;
	OdGePoint3d dimNewPt;    // = ocsDimNewPt;
	if (NeedTransform) {
		ocsDimNewPt.transformBy(world2Plane);
	}
	ocsDimNewPt.z = 0.0;
	for (auto i = 0; i < (int)indices.size(); i++) {
		pGripPoint3d = &gripPoints[indices[i]];
		dimNewPt = *pGripPoint3d;
		ocsDimNewPt = dimNewPt;
		if (indices[i] < 5 && !pDimPtr->isUsingDefaultTextPosition()) {
			pDimPtr->useDefaultTextPosition();
		}
		switch (indices[i]) {
			case 0:
				pDimPtr->setXLine1Start(dimNewPt);
				break;
			case 1:
				pDimPtr->setXLine1End(dimNewPt);
				break;
			case 2:
				pDimPtr->setXLine2Start(dimNewPt);
				break;
			case 3:
				pDimPtr->setXLine2End(dimNewPt);
				break;
			case 4: case 5: {
				vX1 = ocsDimLine1EndPt - ocsDimLine1StartPt;
				vX2 = ocsDimLine2StartPt - ocsDimLine2EndPt;
				line1.set(ocsDimLine1EndPt, vX1);
				line2.set(ocsDimLine2StartPt, vX2);
				line1.intersectWith(line2, dimCenterPt);
				auto angle1 {vX2.angleTo(vX1)};
				if (indices[0] == 4) {
					ocsDimArcNewPt = ocsDimNewPt;
				}
				if (indices[0] == 5) {
					auto vT {dimCenterPt - ocsDimArcNewPt};
					vT.normalize();
					vT *= fabs(OdGeVector3d(dimCenterPt - ocsDimNewPt).length());
					ocsDimArcNewPt = dimCenterPt - vT;
					ocsDimTextPt = ocsDimNewPt;
				}
				vArc = dimCenterPt - ocsDimArcNewPt;
				auto angle2 {vX1.angleTo(vArc)};
				auto angle3 {vX2.angleTo(vArc)};
				OdGeVector3d vXA;
				if (OdEqual(angle3 - angle2, angle1, OdGeContext::gTol.equalPoint())) {
					vX2 = ocsDimLine2EndPt - ocsDimLine2StartPt;
				}
				if (OdEqual(angle2 - angle3, angle1, OdGeContext::gTol.equalPoint())) {
					vX1 = ocsDimLine1StartPt - ocsDimLine1EndPt;
				}
				if (OdEqual(Oda2PI - (angle3 + angle2), angle1, OdGeContext::gTol.equalPoint())) {
					vX2 = ocsDimLine2EndPt - ocsDimLine2StartPt;
					vX1 = ocsDimLine1StartPt - ocsDimLine1EndPt;
				}
				angle1 = vX2.angleTo(vX1);
				vXA = !isAngleDirectionBetweenVectors(vArc, vX1) ? vX1 : vX2;
				vTxt = vXA;
				vTxt.rotateBy(vX1.angleTo(vX2) / 2, OdGeVector3d::kZAxis);
				vXA.rotateBy(angle1 / 3, OdGeVector3d::kZAxis);
				auto vY {vXA};
				angle1 = vArc.angleTo(vXA, OdGeVector3d::kZAxis);
				vY.rotateBy((OdaPI - angle1) / 2, OdGeVector3d::kZAxis);
				line1.set(ocsDimArcNewPt, vY);
				line2.set(dimCenterPt, vXA);
				OdGePoint3d IntersectPoint;
				line1.intersectWith(line2, IntersectPoint);
				ocsDimArcPt = IntersectPoint;
				auto vT1 {vTxt};
				vT1.normalize();
				vT1 *= OdGeVector3d(dimCenterPt - ocsDimArcPt).length();
				dimNewTextPt = dimCenterPt - vT1;
				if (indices[0] == 5) {
					dimNewTextPt = ocsDimTextPt;
				}
				ocsDimArcPt.z = SavedZCoordinate;
				dimNewTextPt.z = SavedZCoordinate;
				if (NeedTransform) {
					ocsDimArcPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
					dimNewTextPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
				}
				if (indices[0] == 4 || pDimPtr->dimtmove() == 0) {
					pDimPtr->setArcPoint(ocsDimArcPt);
				}
				if (indices[0] == 5) {
					if (indices.size() == 1 || !bStretch) {
						pDimPtr->useSetTextPosition();
					}
					pDimPtr->setTextPosition(dimNewTextPt);
				}
			}
			default:
				break;
		}
	} //pDimPtr->recordGraphicsModified(false);
	return eOk;
}

OdResult OdDbArcDimGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto size {gripPoints.size()};
	gripPoints.resize(size + 4);
	OdDbArcDimensionPtr pDimPtr = entity;
	gripPoints[size + 0] = pDimPtr->xLine1Point();
	gripPoints[size + 1] = pDimPtr->xLine2Point();
	gripPoints[size + 2] = pDimPtr->arcPoint();
	gripPoints[size + 3] = pDimPtr->textPosition();
	return eOk;
}

OdResult OdDbArcDimGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbArcDimensionPtr pDimPtr = entity;
	auto dimLine1Pt {pDimPtr->xLine1Point()};
	auto dimLine2Pt {pDimPtr->xLine2Point()};
	auto dimArcPt {pDimPtr->arcPoint()};
	auto dimTextPt {pDimPtr->textPosition()};
	auto dimCenterPt {pDimPtr->centerPoint()};
	auto dimArcNewPt {dimArcPt};
	OdGePoint3d dimNewTextPt;
	auto world2Plane(OdGeMatrix3d::worldToPlane(pDimPtr->normal()));
	auto ocsDimLine1Pt {dimLine1Pt};
	auto ocsDimLine2Pt {dimLine2Pt};
	auto ocsDimArcPt {dimArcPt};
	auto ocsDimTextPt {dimTextPt};
	auto ocsDimArcNewPt {dimArcNewPt};
	auto ocsDimCenterPt {dimCenterPt};
	auto Normal {pDimPtr->normal()};
	auto NeedTransform {false};
	if (Normal != OdGeVector3d::kZAxis) {
		NeedTransform = true;
		ocsDimLine1Pt.transformBy(world2Plane);
		ocsDimLine2Pt.transformBy(world2Plane);
		ocsDimArcPt.transformBy(world2Plane);
		ocsDimTextPt.transformBy(world2Plane);
		ocsDimArcNewPt.transformBy(world2Plane);
		ocsDimCenterPt.transformBy(world2Plane);
	}
	auto SavedZCoordinate {ocsDimLine1Pt.z};
	ocsDimLine1Pt.z = ocsDimLine2Pt.z = ocsDimArcPt.z = ocsDimTextPt.z = ocsDimArcNewPt.z = ocsDimCenterPt.z = 0.0;
	OdGeVector3d vX1;
	OdGeVector3d vX2;
	OdGeVector3d vArc;
	OdGeVector3d vTxt;
	const OdGePoint3d* pGripPoint3d = nullptr;
	OdGeLine3d line1, line2, lineText;
	pGripPoint3d = &gripPoints[indices[0]];
	auto ocsDimNewPt {*pGripPoint3d};
	auto dimNewPt {ocsDimNewPt};
	if (NeedTransform) {
		ocsDimNewPt.transformBy(world2Plane);
	}
	ocsDimNewPt.z = 0.0;
	for (auto i = 0; i < (int)indices.size(); i++) {
		pGripPoint3d = &gripPoints[indices[i]];
		dimNewPt = *pGripPoint3d;
		ocsDimNewPt = dimNewPt;
		if (indices[i] < 3 && !pDimPtr->isUsingDefaultTextPosition()) {
			pDimPtr->useDefaultTextPosition();
		}
		if (indices[i] == 0) {
			pDimPtr->setXLine1Point(*pGripPoint3d); // ocsDimLine1Pt = ocsDimNewPt;
			// ocsDimArcNewPt = ocsDimArcPt;
			// return eOk;
			continue;
		}
		if (indices[i] == 1) {
			pDimPtr->setXLine2Point(*pGripPoint3d); // ocsDimLine2Pt = ocsDimNewPt;
			// ocsDimArcNewPt = ocsDimArcPt;
			continue;
		}
		vX1 = ocsDimCenterPt - ocsDimLine1Pt;
		vX2 = ocsDimCenterPt - ocsDimLine2Pt;
		auto angle1 {vX2.angleTo(vX1)};
		if (indices[i] == 2) {
			ocsDimArcNewPt = ocsDimNewPt;
		}
		if (indices[i] == 3) {
			if (indices.size() == 1 || !bStretch) {
				pDimPtr->useSetTextPosition();
			}
			auto vT {ocsDimCenterPt - ocsDimArcNewPt};
			vT.normalize();
			vT *= fabs(OdGeVector3d(ocsDimCenterPt - ocsDimNewPt).length());
			ocsDimArcNewPt = ocsDimCenterPt - vT;
			ocsDimTextPt = ocsDimNewPt;
		}
		vArc = dimCenterPt - ocsDimArcNewPt;
		auto angle2 {vX1.angleTo(vArc)};
		auto angle3 {vX2.angleTo(vArc)};
		OdGeVector3d vXA;
		if (OdEqual(angle3 - angle2, angle1, OdGeContext::gTol.equalPoint())) {
			vX2 = ocsDimLine2Pt - ocsDimCenterPt;
		}
		if (OdEqual(angle2 - angle3, angle1, OdGeContext::gTol.equalPoint())) {
			vX1 = ocsDimLine1Pt - ocsDimCenterPt;
		}
		if (OdEqual(Oda2PI - (angle3 + angle2), angle1, OdGeContext::gTol.equalPoint())) {
			vX2 = ocsDimLine2Pt - ocsDimCenterPt;
			vX1 = ocsDimLine1Pt - ocsDimCenterPt;
		}
		angle1 = vX2.angleTo(vX1);
		vXA = !isAngleDirectionBetweenVectors(vArc, vX1) ? vX1 : vX2;
		vTxt = vXA;
		vTxt.rotateBy(vX1.angleTo(vX2) / 2, OdGeVector3d::kZAxis);
		vXA.rotateBy(angle1 / 3, OdGeVector3d::kZAxis);
		auto vY {vXA};
		angle1 = vArc.angleTo(vXA, OdGeVector3d::kZAxis);
		vY.rotateBy((OdaPI - angle1) / 2, OdGeVector3d::kZAxis);
		line1.set(ocsDimArcNewPt, vY);
		line2.set(dimCenterPt, vXA);
		OdGePoint3d IntersectPoint;
		line1.intersectWith(line2, IntersectPoint);
		ocsDimArcPt = IntersectPoint;
		auto vT1 {vTxt};
		vT1.normalize();
		vT1 *= OdGeVector3d(dimCenterPt - ocsDimArcPt).length();
		dimNewTextPt = dimCenterPt - vT1;
		if (indices[i] == 5) {
			dimNewTextPt = ocsDimTextPt;
		}
		ocsDimArcPt.z = SavedZCoordinate;
		dimNewTextPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimArcPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
			dimNewTextPt.transformBy(OdGeMatrix3d::planeToWorld(pDimPtr->normal()));
		}
		if (indices[i] == 2 || pDimPtr->dimtmove() == 0) {
			pDimPtr->setArcPoint(ocsDimArcPt);
		}
		pDimPtr->setTextPosition(dimNewTextPt);
	} //pDimPtr->recordGraphicsModified(false);
	return eOk;
}

OdResult OdDbRadialDimLargeGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	const auto size {gripPoints.size()};
	gripPoints.resize(size + 4);
	OdDbRadialDimensionLargePtr pDimPtr = entity;
	gripPoints[size + 0] = pDimPtr->chordPoint();
	gripPoints[size + 1] = pDimPtr->overrideCenter();
	gripPoints[size + 2] = pDimPtr->jogPoint();
	gripPoints[size + 3] = pDimPtr->textPosition();
	return eOk;
}

OdResult OdDbRadialDimLargeGripPointsPE::moveGripPoint(OdDbEntity* entity, const OdGePoint3dArray& gripPoints, const OdIntArray& indices, bool bStretch) {
	if (indices.empty()) {
		return eOk;
	}
	OdDbRadialDimensionLargePtr pDimPtr = entity;
	auto dimChordPt {pDimPtr->chordPoint()};
	auto dimOverrideCenterPt {pDimPtr->overrideCenter()};
	auto dimJogPt {pDimPtr->jogPoint()};
	auto dimTextPt {pDimPtr->textPosition()};
	auto dimCenterPt {pDimPtr->center()};
	auto jogAngle {pDimPtr->jogAngle()};
	auto vNorm {pDimPtr->normal()};
	auto NeedTransform {false};
	if (vNorm != OdGeVector3d::kZAxis) {
		NeedTransform = true;
	}
	auto world2Plane(OdGeMatrix3d::worldToPlane(vNorm));
	auto plane2World(OdGeMatrix3d::planeToWorld(vNorm));
	auto ocsDimChordPt {dimChordPt};
	auto ocsDimOverrideCenterPt {dimOverrideCenterPt};
	auto ocsDimJogPt {dimJogPt};
	auto ocsDimTextPt {dimTextPt};
	auto ocsDimCenterPt {dimCenterPt};
	OdGeVector3d vX1;
	OdGeVector3d vX2;
	OdGeVector3d vArc;
	OdGeVector3d vTxt;
	const OdGePoint3d* pGripPoint3d = nullptr;
	OdGeLine3d line1, line2, lineText;
	pGripPoint3d = &gripPoints[indices[0]];
	auto ocsDimNewPt {*pGripPoint3d};
	auto dimNewPt {ocsDimNewPt};
	if (NeedTransform) {
		ocsDimChordPt.transformBy(world2Plane);
		ocsDimOverrideCenterPt.transformBy(world2Plane);
		ocsDimJogPt.transformBy(world2Plane);
		ocsDimTextPt.transformBy(world2Plane);
		ocsDimCenterPt.transformBy(world2Plane);
		ocsDimNewPt.transformBy(world2Plane);
	}
	auto SavedZCoordinate {ocsDimChordPt.z};
	ocsDimChordPt.z = ocsDimOverrideCenterPt.z = ocsDimJogPt.z = ocsDimTextPt.z = ocsDimCenterPt.z = 0.0;
	ocsDimNewPt.z = 0.0;
	if (indices[0] == 0) {
		auto dimLRadius {fabs(OdGeVector3d(ocsDimCenterPt - ocsDimChordPt).length())};
		auto vR {ocsDimCenterPt - ocsDimChordPt};
		vR = ocsDimCenterPt - ocsDimNewPt;
		vR.normalize();
		vR *= dimLRadius;
		ocsDimNewPt = ocsDimCenterPt - vR;
		ocsDimChordPt = ocsDimNewPt;
		ocsDimNewPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimNewPt.transformBy(plane2World);
		}
		pDimPtr->setChordPoint(ocsDimNewPt); // correct text point
		vR.normalize();
		vR *= ocsDimTextPt.distanceTo(ocsDimCenterPt);
		ocsDimTextPt = ocsDimCenterPt - vR;
		ocsDimTextPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimTextPt.transformBy(plane2World);
		}
		pDimPtr->setTextPosition(ocsDimTextPt);
	}
	if (indices[0] == 1) {
		ocsDimOverrideCenterPt = ocsDimNewPt;
		if (NeedTransform) {
			ocsDimNewPt.transformBy(plane2World);
		}
		pDimPtr->setOverrideCenter(ocsDimNewPt);
	}
	if (indices[0] == 2) {
		auto vR {ocsDimCenterPt - ocsDimChordPt};
		vX1 = vR;
		vX1.rotateBy(jogAngle, OdGeVector3d::kZAxis);
		line1.set(ocsDimCenterPt, vR);
		line2.set(ocsDimNewPt, vX1);
		OdGePoint3d IntersectPoint;
		line1.intersectWith(line2, IntersectPoint);
		line1.set(ocsDimOverrideCenterPt, vR);
		OdGePoint3d intersec2Pt;
		line1.intersectWith(line2, intersec2Pt);
		auto dimLen {fabs(OdGeVector3d(IntersectPoint - intersec2Pt).length())};
		auto vR2 {IntersectPoint - intersec2Pt};
		vR2.normalize();
		vR2 *= dimLen / 2;
		ocsDimJogPt = IntersectPoint - vR2;
		ocsDimJogPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimJogPt.transformBy(plane2World);
		}
		pDimPtr->setJogPoint(ocsDimJogPt);
	}
	if (indices[0] == 3) {
		auto dimLRadius {fabs(OdGeVector3d(ocsDimCenterPt - ocsDimChordPt).length())};
		auto vR {ocsDimCenterPt - ocsDimChordPt};
		vR = ocsDimCenterPt - ocsDimNewPt;
		vR.normalize();
		vR *= dimLRadius;
		ocsDimChordPt = ocsDimCenterPt - vR;
		ocsDimChordPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimChordPt.transformBy(plane2World);
		}
		pDimPtr->setChordPoint(ocsDimChordPt);
		ocsDimTextPt = ocsDimNewPt;
		ocsDimTextPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimTextPt.transformBy(plane2World);
		}
		pDimPtr->setTextPosition(ocsDimTextPt); // correct jog point
		if (indices.size() == 1 || !bStretch) {
			pDimPtr->useSetTextPosition();
		}
	}
	if (indices[0] != 2) {
		// correct jog point
		auto dimLen {fabs(OdGeVector3d(ocsDimChordPt - ocsDimOverrideCenterPt).length())};
		auto vR2 {ocsDimChordPt - ocsDimOverrideCenterPt};
		vR2.normalize();
		vR2 *= dimLen / 2;
		ocsDimJogPt = ocsDimChordPt - vR2;
		ocsDimJogPt.z = SavedZCoordinate;
		if (NeedTransform) {
			ocsDimJogPt.transformBy(plane2World);
		}
		pDimPtr->setJogPoint(ocsDimJogPt);
	}
	return eOk;
}

const bool OdDbDimGripPointsPE::isAngleDirectionBetweenVectors(OdGeVector3d v1, OdGeVector3d v2) {
	return v1.angleTo(v2, OdGeVector3d::kZAxis) > OdaPI ? false : true;
}

OdResult OdDbDimGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbDimGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	if (indices.empty()) {
		return eOk;
	}
	try {
		OdGePoint3dArray gripPoints;
		getGripPoints(entity, gripPoints);
		for (unsigned i = 0; i < indices.size(); ++i) {
			if (indices[i] < (int)gripPoints.size()) {
				gripPoints[indices[i]] += offset;
			}
		}
		moveGripPoint(entity, gripPoints, indices, true);
		OdDbDimensionPtr pDimPtr = entity;
		pDimPtr->recomputeDimBlock();
	} catch (const OdError& e) {
		return e.code();
	}
	return eOk;
}

OdResult OdDbDimGripPointsPE::getOsnapPoints(const OdDbEntity* /*entity*/, OdDb::OsnapMode /*objectSnapMode*/, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& /*snapPoints*/) const {
	return eOk;
}
