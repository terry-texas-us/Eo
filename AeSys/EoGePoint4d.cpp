#include "stdafx.h"
#include "EoGeMatrix3d.h"
#include "EoGePoint4d.h"

EoGePoint4d::EoGePoint4d() {
	x = 0.0;
	y = 0.0;
	z = 0.0;
	w = 1.0;
}

EoGePoint4d::EoGePoint4d(const OdGePoint3d& initialPoint, const double initialW) noexcept {
	x = initialPoint.x;
	y = initialPoint.y;
	z = initialPoint.z;
	w = initialW;
}

void EoGePoint4d::operator/=(const double d) noexcept {
	x /= d;
	y /= d;
	z /= d;
	w /= d;
}

OdGeVector3d EoGePoint4d::operator-(const EoGePoint4d& point) const {
	return {x - point.x, y - point.y, z - point.z};
}

EoGePoint4d EoGePoint4d::operator-(const OdGeVector3d& vector) const {
	EoGePoint4d Point;
	Point.x = x - vector.x;
	Point.y = y - vector.y;
	Point.z = z - vector.z;
	Point.w = w;
	return Point;
}

EoGePoint4d EoGePoint4d::operator+(const OdGeVector3d& vector) const {
	EoGePoint4d Point;
	Point.x = x + vector.x;
	Point.y = y + vector.y;
	Point.z = z + vector.z;
	Point.w = w;
	return Point;
}

bool EoGePoint4d::ClipLine(EoGePoint4d& ptA, EoGePoint4d& ptB) {
	const double BoundaryCodeA[] = {
	ptA.w + ptA.x,
	ptA.w - ptA.x,
	ptA.w + ptA.y,
	ptA.w - ptA.y,
	ptA.w + ptA.z,
	ptA.w - ptA.z
	};
	const double BoundaryCodeB[] = {
	ptB.w + ptB.x,
	ptB.w - ptB.x,
	ptB.w + ptB.y,
	ptB.w - ptB.y,
	ptB.w + ptB.z,
	ptB.w - ptB.z
	};
	auto OutCodeA {0};
	auto OutCodeB {0};
	for (auto iBC = 0; iBC < 6; iBC++) {
		if (BoundaryCodeA[iBC] <= 0.0) { OutCodeA |= 1 << iBC; }
		if (BoundaryCodeB[iBC] <= 0.0) { OutCodeB |= 1 << iBC; }
	}
	if ((OutCodeA & OutCodeB) != 0) { return false; }
	if ((OutCodeA | OutCodeB) == 0) { return true; }
	auto dTIn {0.0};
	auto dTOut {1.0};
	double dTHit;
	for (auto i = 0; i < 6; i++) {
		if (BoundaryCodeB[i] < 0.0) {
			dTHit = BoundaryCodeA[i] / (BoundaryCodeA[i] - BoundaryCodeB[i]);
			dTOut = EoMin(dTOut, dTHit);
		} else if (BoundaryCodeA[i] < 0.0) {
			dTHit = BoundaryCodeA[i] / (BoundaryCodeA[i] - BoundaryCodeB[i]);
			dTIn = EoMax(dTIn, dTHit);
		}
		if (dTIn > dTOut) { return false; }
	}
	auto pt {ptA};
	if (OutCodeA != 0) {
		ptA = pt + (ptB - pt) * dTIn;
	}
	if (OutCodeB != 0) {
		ptB = pt + (ptB - pt) * dTOut;
	}
	return true;
}

void EoGePoint4d::ClipPolygon(EoGePoint4dArray& pointsArray) {
	static OdGePoint3d pointsOnClipPlanes[] = {
	OdGePoint3d(- 1.0, 0.0, 0.0),
	OdGePoint3d(1.0, 0.0, 0.0),
	OdGePoint3d(0.0, - 1.0, 0.0),
	OdGePoint3d(0.0, 1.0, 0.0),
	OdGePoint3d(0.0, 0.0, - 1.0),
	OdGePoint3d(0.0, 0.0, 1.0)
	};
	static OdGeVector3d vPln[] = {
	OdGeVector3d(1.0, 0.0, 0.0),
	OdGeVector3d(- 1.0, 0.0, 0.0),
	OdGeVector3d(0.0, 1.0, 0.0),
	OdGeVector3d(0.0, - 1.0, 0.0),
	OdGeVector3d(0.0, 0.0, 1.0),
	OdGeVector3d(0.0, 0.0, - 1.0)
	};
	EoGePoint4dArray PointsArrayOut;
	for (auto planeIndex = 0; planeIndex < 6; planeIndex++) {
		IntersectionWithPln(pointsArray, pointsOnClipPlanes[planeIndex], vPln[planeIndex], PointsArrayOut);
		const auto iPtsOut {static_cast<int>(PointsArrayOut.GetSize())};
		pointsArray.SetSize(iPtsOut);
		if (iPtsOut == 0) { break; }
		pointsArray.Copy(PointsArrayOut);
		PointsArrayOut.RemoveAll();
	}
}

void EoGePoint4d::IntersectionWithPln(EoGePoint4dArray& pointsArrayIn, const OdGePoint3d& pointOnPlane, const OdGeVector3d& planeNormal, EoGePoint4dArray& pointsArrayOut) {
	if (pointsArrayIn.IsEmpty() != 0) { return; }
	EoGePoint4d pt;
	EoGePoint4d ptEdge[2];
	bool bEdgeVis[2];
	const auto bVisVer0 {OdGeVector3d(pointsArrayIn[0].Convert3d() - pointOnPlane).dotProduct(planeNormal) >= -DBL_EPSILON};
	ptEdge[0] = pointsArrayIn[0];
	bEdgeVis[0] = bVisVer0;
	if (bVisVer0) {
		pointsArrayOut.Add(pointsArrayIn[0]);
	}
	const auto iPtsIn {static_cast<int>(pointsArrayIn.GetSize())};
	for (auto i = 1; i < iPtsIn; i++) {
		ptEdge[1] = pointsArrayIn[i];
		bEdgeVis[1] = OdGeVector3d(ptEdge[1].Convert3d() - pointOnPlane).dotProduct(planeNormal) >= - DBL_EPSILON;
		if (bEdgeVis[0] != bEdgeVis[1]) { // Vertices of edge on opposite sides of clip plane
			pt = IntersectionWithPln4(ptEdge[0], ptEdge[1], EoGePoint4d(pointOnPlane, 1.0), planeNormal);
			pointsArrayOut.Add(pt);
		}
		if (bEdgeVis[1]) {
			pointsArrayOut.Add(pointsArrayIn[i]);
		}
		ptEdge[0] = ptEdge[1];
		bEdgeVis[0] = bEdgeVis[1];
	}
	if (pointsArrayOut.GetSize() != 0 && bEdgeVis[0] != bVisVer0) { // first and last vertices on opposite sides of clip plane
		pt = IntersectionWithPln4(ptEdge[0], pointsArrayIn[0], EoGePoint4d(pointOnPlane, 1.0), planeNormal);
		pointsArrayOut.Add(pt);
	}
}

EoGePoint4d EoGePoint4d::IntersectionWithPln4(EoGePoint4d& startPoint, EoGePoint4d& endPoint, const EoGePoint4d& pointOnPlane, const OdGeVector3d& planeNormal) noexcept {
	auto LineVector {endPoint.Convert3d() - startPoint.Convert3d()};
	const auto DotProduct {planeNormal.dotProduct(LineVector)};
	if (fabs(DotProduct) > DBL_EPSILON) {
		const auto vPtPt0 {startPoint.Convert3d() - pointOnPlane.Convert3d()};
		LineVector *= planeNormal.dotProduct(vPtPt0) / DotProduct;
	} else { // Line and the plane are parallel .. force return to start point
		LineVector *= 0.0;
	}
	return startPoint - LineVector;
}

OdGePoint3d EoGePoint4d::Convert3d() const {
	return OdGePoint3d(x / w, y / w, z / w);
}

double EoGePoint4d::DistanceToPointXY(const EoGePoint4d& ptQ) const noexcept {
	const auto X {ptQ.x / ptQ.w - x / w};
	const auto Y {ptQ.y / ptQ.w - y / w};
	return sqrt(X * X + Y * Y);
}

bool EoGePoint4d::IsInView() const noexcept {
	if (w + x <= 0. || w - x <= 0.0) { return false; }
	if (w + y <= 0. || w - y <= 0.0) { return false; }
	if (w + z <= 0. || w - z <= 0.0) { return false; }
	return true;
}

EoGePoint4d& EoGePoint4d::TransformBy(const EoGeMatrix3d& matrix) noexcept {
	EoGePoint4d Point;
	Point.x = x * matrix.entry[0][0] + y * matrix.entry[0][1] + z * matrix.entry[0][2] + w * matrix.entry[0][3];
	Point.y = x * matrix.entry[1][0] + y * matrix.entry[1][1] + z * matrix.entry[1][2] + w * matrix.entry[1][3];
	Point.z = x * matrix.entry[2][0] + y * matrix.entry[2][1] + z * matrix.entry[2][2] + w * matrix.entry[2][3];
	Point.w = x * matrix.entry[3][0] + y * matrix.entry[3][1] + z * matrix.entry[3][2] + w * matrix.entry[3][3];
	*this = Point;
	return *this;
}

EoGePoint4d EoGePoint4d::Max(EoGePoint4d& ptA, EoGePoint4d& ptB) {
	EoGePoint4d Point;
	Point.x = EoMax(ptA.x, ptB.x);
	Point.y = EoMax(ptA.y, ptB.y);
	Point.z = EoMax(ptA.z, ptB.z);
	Point.w = EoMax(ptA.w, ptB.w);
	return Point;
}

EoGePoint4d EoGePoint4d::Min(EoGePoint4d& ptA, EoGePoint4d& ptB) {
	EoGePoint4d Point;
	Point.x = EoMin(ptA.x, ptB.x);
	Point.y = EoMin(ptA.y, ptB.y);
	Point.z = EoMin(ptA.z, ptB.z);
	Point.w = EoMin(ptA.w, ptB.w);
	return Point;
}
