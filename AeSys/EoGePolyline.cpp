#include "stdafx.h"

#include "AeSysDoc.h"
#include "AeSysView.h"

namespace polyline {
	EoGePoint4dArray	pts_;
	bool LoopLine;

void BeginLineLoop() {
	pts_.SetSize(0);
	LoopLine = true;
}
void BeginLineStrip() {
	pts_.SetSize(0);
	LoopLine = false;
}
bool AnyPointsInView(EoGePoint4dArray& pointsArray) {
	for (auto i = 0; i < pointsArray.GetSize(); i++) {
		if (pointsArray[i].IsInView()) return true;
	}
	return false;
}
void __Display(AeSysView* view, CDC* deviceContext, EoGePoint4dArray& pointsArray, OdDbLinetypeTableRecordPtr linetype) {
	const auto NumberOfDashes {linetype->numDashes()};
	if (NumberOfDashes == 0) return;

	EoGePoint4d ln[2];
	CPoint pnt[2];
	OdGePoint3d pt[2];
	auto DashIndex {0};
	auto SectionLength {EoMax(.025 * 96., fabs(linetype->dashLengthAt(DashIndex)))};

	for (auto i = 0; i < pointsArray.GetSize() - 1; i++) {
		const auto vLn {pointsArray[i + 1].Convert3d() - pointsArray[i].Convert3d()};
		pt[0] = pointsArray[i].Convert3d();

		const auto dVecLen {vLn.length()};
		auto dRemDisToEnd {dVecLen};

		while (SectionLength <= dRemDisToEnd + DBL_EPSILON) {
			auto vDash {vLn};
			vDash *= SectionLength / dVecLen;
			pt[1] = pt[0] + vDash;
			dRemDisToEnd -= SectionLength;
			if (linetype->dashLengthAt(DashIndex) >= 0.0) {
				ln[0] = EoGePoint4d(pt[0], 1.0);
				ln[1] = EoGePoint4d(pt[1], 1.0);

				view->ModelViewTransformPoints(2, ln);

				if (EoGePoint4d::ClipLine(ln[0], ln[1])) {
					view->DoViewportProjection(pnt, 2, &ln[0]);
					deviceContext->Polyline(pnt, 2);
				}
			}
			pt[0] = pt[1];
			DashIndex = (DashIndex + 1) % NumberOfDashes;
			SectionLength = EoMax(.025/* * 96.*/, fabs(linetype->dashLengthAt(DashIndex)));
		}
		if (dRemDisToEnd > DBL_EPSILON) { // Partial component of dash section must produced
			if (linetype->dashLengthAt(DashIndex) >= 0.0) {
				pt[1] = pointsArray[i + 1].Convert3d();

				ln[0] = EoGePoint4d(pt[0], 1.0);
				ln[1] = EoGePoint4d(pt[1], 1.0);

				view->ModelViewTransformPoints(2, ln);

				if (EoGePoint4d::ClipLine(ln[0], ln[1])) {
					view->DoViewportProjection(pnt, 2, &ln[0]);
					deviceContext->Polyline(pnt, 2);
				}
			}
		}
		// Length of dash remaining
		SectionLength -= dRemDisToEnd;
	}
}
void __End(AeSysView* view, CDC* deviceContext, short linetypeIndex) {
	if (EoDbPrimitive::IsSupportedLinetype(linetypeIndex)) {
		const auto Size {pts_.GetSize()};
		if (Size > 1) {
			view->ModelViewTransformPoints(pts_);

			if (AnyPointsInView(pts_)) {
				auto pnt {view->DoViewportProjection(pts_[0])};
				deviceContext->MoveTo(pnt);

				for (auto i = 1; i < Size; i++) {
					pnt = view->DoViewportProjection(pts_[i]);
					deviceContext->LineTo(pnt);
				}
				if (LoopLine) {
					pnt = view->DoViewportProjection(pts_[0]);
					deviceContext->LineTo(pnt);
				}
			}
		}
	} else {
		auto Name {EoDbLinetypeTable::LegacyLinetypeName(linetypeIndex)};
		auto Database {AeSysDoc::GetDoc()->m_DatabasePtr};

		OdDbLinetypeTablePtr Linetypes {Database->getLinetypeTableId().safeOpenObject(OdDb::kForRead)};
		OdDbLinetypeTableRecordPtr Linetype {Linetypes->getAt(Name).safeOpenObject(OdDb::kForRead)};

		pstate.SetLinetypeIndexPs(deviceContext, 1);
		__Display(view, deviceContext, pts_, Linetype);
		pstate.SetLinetypeIndexPs(deviceContext, linetypeIndex);
	}
}

void GeneratePointsForNPoly(const OdGePoint3d& centerPoint, const OdGeVector3d& planeNormal, double radius, unsigned numberOfPoints, OdGePoint3dArray& points) {
	auto MajorAxis {ComputeArbitraryAxis(planeNormal)};
	MajorAxis.normalize();
	auto MinorAxis {planeNormal.crossProduct(MajorAxis)};
	
	MajorAxis *= radius;
	MinorAxis *= radius;

	OdGeMatrix3d ScaleMatrix;
	ScaleMatrix.setToScaling(OdGeScale3d(MajorAxis.length(), MinorAxis.length(), 1.0));

	EoGeMatrix3d PlaneToWorldTransform;
	PlaneToWorldTransform.setToPlaneToWorld(OdGePlane(centerPoint, MajorAxis, MinorAxis));
	PlaneToWorldTransform.postMultBy(ScaleMatrix);

	// Determine the parameter (angular increment)
	const auto AngleIncrement {Oda2PI / double(numberOfPoints)};
	const auto CosIncrement {cos(AngleIncrement)};
	const auto SinIncrement {sin(AngleIncrement)};
	points.setLogicalLength(numberOfPoints);
	points[0].set(1.0, 0.0, 0.0);

	for (unsigned PointIndex = 0; PointIndex < numberOfPoints - 1; PointIndex++) {
		points[PointIndex + 1].x = points[PointIndex].x * CosIncrement - points[PointIndex].y * SinIncrement;
		points[PointIndex + 1].y = points[PointIndex].y * CosIncrement + points[PointIndex].x * SinIncrement;
		points[PointIndex + 1].z = 0.0;
	}
	for (unsigned PointIndex = 0; PointIndex < numberOfPoints; PointIndex++) {
		points[PointIndex].transformBy(PlaneToWorldTransform);
	}
}
bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) {
	auto StartPoint {pts_[0]};
	view->ModelViewTransformPoint(StartPoint);

	for (unsigned short w = 1; w < pts_.GetSize(); w++) {
		auto EndPoint {EoGePoint4d(pts_[w])};
		view->ModelViewTransformPoint(EndPoint);

		OdGePoint3d Intersection;
		if (lineSeg.IntersectWith_xy(EoGeLineSeg3d(StartPoint.Convert3d(), EndPoint.Convert3d()), Intersection)) {
			double Relationship;
			lineSeg.ParametricRelationshipOf(Intersection, Relationship);

			if (Relationship >= - DBL_EPSILON && Relationship <= 1. + DBL_EPSILON) {
				EoGeLineSeg3d(StartPoint.Convert3d(), EndPoint.Convert3d()).ParametricRelationshipOf(Intersection, Relationship);

				if (Relationship >= - DBL_EPSILON && Relationship <= 1. + DBL_EPSILON) {
					Intersection.z = StartPoint.z + Relationship * (EndPoint.z - StartPoint.z);
					intersections.append(Intersection);
				}
			}
		}
		StartPoint = EndPoint;
	}
	return !intersections.empty();
}

bool SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, double& dRel, OdGePoint3d& ptProj) {
	auto Result {false};
	auto StartPoint {pts_[0]};
	view->ModelViewTransformPoint(StartPoint);

	for (auto i = 1; i < pts_.GetSize(); i++) {
		auto EndPoint {EoGePoint4d(pts_[i])};
		view->ModelViewTransformPoint(EndPoint);
		EoGeLineSeg3d LineSegment(StartPoint.Convert3d(), EndPoint.Convert3d());
		
		if (LineSegment.IsSelectedBy_xy(point.Convert3d(), view->SelectApertureSize(), ptProj, dRel)) {
			ptProj.z = StartPoint.z + dRel * (EndPoint.z - StartPoint.z);
			Result = true;
			break;
		}
		StartPoint = EndPoint;
	}
	return Result;
}

bool SelectUsingRectangle(const OdGePoint3d& lowerLeftPoint, const OdGePoint3d& upperRightPoint, AeSysView* view) {
	auto StartPoint {pts_[0]};
	view->ModelViewTransformPoint(StartPoint);

	for (unsigned short w = 1; w < pts_.GetSize(); w++) {
		auto EndPoint(pts_[w]);
		view->ModelViewTransformPoint(EndPoint);

		EoGeLineSeg3d LineSegment(StartPoint.Convert3d(), EndPoint.Convert3d());
		if (LineSegment.IsContainedBy_xy(lowerLeftPoint, upperRightPoint))
			return true;

		StartPoint = EndPoint;
	}
	return false;
}
// <tas="Not considering possible closure"</tas>
bool SelectUsingRectangle(AeSysView* view, const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, const OdGePoint3dArray& points) {
	EoGePoint4d ptBeg(points[0], 1.0);
	view->ModelViewTransformPoint(ptBeg);

	for (unsigned w = 1; w < points.size(); w++) {
		EoGePoint4d ptEnd(points[w], 1.0);
		view->ModelViewTransformPoint(ptEnd);

		EoGeLineSeg3d LineSegment(ptBeg.Convert3d(), ptEnd.Convert3d());

		if (LineSegment.IsContainedBy_xy(lowerLeftCorner, upperRightCorner)) { return true; }

		ptBeg = ptEnd;
	}
	return false;
}
void SetVertex(const OdGePoint3d& point) {
	EoGePoint4d Point4(point, 1.0);
	pts_.Add(Point4);
}

}