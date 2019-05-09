#include "stdafx.h"
#include "AeSysView.h"

#include "EoGePolyline.h"

EoGeLineSeg3d::EoGeLineSeg3d()
	: OdGeLineSeg3d() {
}
EoGeLineSeg3d::EoGeLineSeg3d(const EoGeLineSeg3d& line) 
	: OdGeLineSeg3d(line) {
}
EoGeLineSeg3d::EoGeLineSeg3d(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint) 
	: OdGeLineSeg3d(startPoint, endPoint) {
}
EoGeLineSeg3d::~EoGeLineSeg3d() {
}
double EoGeLineSeg3d::AngleBetween_xy(const EoGeLineSeg3d& line) const {
	OdGeVector3d v1(endPoint() - startPoint()); 
	v1.z = 0.;
	OdGeVector3d v2(line.endPoint() - line.startPoint());
	v2.z = 0.;

	const double dSumProd = v1.lengthSqrd() * v2.lengthSqrd();

	if (dSumProd > DBL_EPSILON) {
		double dVal = v1.dotProduct(v2) / sqrt(dSumProd);

		dVal = EoMax(- 1., EoMin(1., dVal));

		return (acos(dVal));
	}
	return (0.);
}
double EoGeLineSeg3d::AngleFromXAxis_xy() const {
	const OdGeVector3d Vector(endPoint() - startPoint());

	double Angle = 0.;

	if (fabs(Vector.x) > DBL_EPSILON || fabs(Vector.y) > DBL_EPSILON) {
		Angle = atan2(Vector.y, Vector.x);

		if (Angle < 0.)
			Angle += TWOPI;
	}
	return (Angle);
}
OdGePoint3d EoGeLineSeg3d::ConstrainToAxis(double influenceAngle, double axisOffsetAngle) const {
	EoGeMatrix3d TransformMatrix;
	TransformMatrix.setToTranslation(- startPoint().asVector());

	EoGeMatrix3d RotationMatrix;
	RotationMatrix.setToRotation(- EoToRadian(axisOffsetAngle), OdGeVector3d::kZAxis);

	TransformMatrix.preMultBy(RotationMatrix);

	OdGePoint3d pt = endPoint();

	pt.transformBy(TransformMatrix);

	const double dX = pt.x * pt.x;
	const double dY = pt.y * pt.y;
	const double dZ = pt.z * pt.z;

	double dLen = sqrt(dX + dY + dZ);

	if (dLen > DBL_EPSILON) { // Not a zero length line
		if (dX >= EoMax(dY, dZ)) { // Major component of line is along x-axis
			dLen = sqrt(dY + dZ);
			if (dLen > DBL_EPSILON) 				// Not already on the x-axis
				if (dLen / fabs(pt.x) < tan(EoToRadian(influenceAngle))) { // Within cone of influence .. snap to x-axis
					pt.y = 0.;
					pt.z = 0.;
				}
		}
		else if (dY >= dZ) { // Major component of line is along y-axis
			dLen = sqrt(dX + dZ);
			if (dLen > DBL_EPSILON)					// Not already on the y-axis
				if (dLen / fabs(pt.y) < tan(EoToRadian(influenceAngle))) { // Within cone of influence .. snap to y-axis
					pt.x = 0.;
					pt.z = 0.;
				}
		}
		else {
			dLen = sqrt(dX + dY);
			if (dLen > DBL_EPSILON)					// Not already on the z-axis
				if (dLen / fabs(pt.z) < tan(EoToRadian(influenceAngle))) { // Within cone of influence .. snap to z-axis
					pt.x = 0.;
					pt.y = 0.;
				}
		}
	}
	TransformMatrix.invert();
	pt.transformBy(TransformMatrix);
	return (pt);
}
// <tas="CutAt point does not do on the line checks"</tas>
OdUInt16 EoGeLineSeg3d::CutAt(const OdGePoint3d& point, EoGeLineSeg3d& line) {
	OdUInt16 wRet = 0;

	line = *this;

	if (point != startPoint() && point != endPoint()) {
		line.SetEndPoint(point);
		SetStartPoint(point);

		wRet++;
	}
	return (wRet);
}
int EoGeLineSeg3d::DirectedRelationshipOf(const OdGePoint3d& point) const {
	const double Determinant = startPoint().x * (endPoint().y - point.y) - endPoint().x * (startPoint().y - point.y) + point.x * (startPoint().y - endPoint().y);

	if (Determinant > DBL_EPSILON)
		return (1);
	else if (Determinant < - DBL_EPSILON)
		return (- 1);
	else
		return 0;
}
void EoGeLineSeg3d::Display(AeSysView* view, CDC* deviceContext) {
	const OdInt16 LinetypeIndex = pstate.LinetypeIndex();

	if (EoDbPrimitive::IsSupportedLinetype(LinetypeIndex)) {
		EoGePoint4d pt[] = {EoGePoint4d(startPoint(), 1.), EoGePoint4d(endPoint(), 1.)};

		view->ModelViewTransformPoints(2, pt);

		if (EoGePoint4d::ClipLine(pt[0], pt[1])) {
			CPoint pnt[2];
			view->DoViewportProjection(pnt, 2, pt);
			deviceContext->Polyline(pnt, 2);
		}
	}
	else {
		polyline::BeginLineStrip();
		polyline::SetVertex(startPoint());
		polyline::SetVertex(endPoint());
		polyline::__End(view, deviceContext, LinetypeIndex);
	}
}
void EoGeLineSeg3d::Extents(OdGePoint3d& minimum, OdGePoint3d& maximum) {
	minimum.x = EoMin(startPoint().x, endPoint().x);
	minimum.y = EoMin(startPoint().y, endPoint().y);
	minimum.z = EoMin(startPoint().z, endPoint().z);

	maximum.x = EoMax(startPoint().x, endPoint().x);
	maximum.y = EoMax(startPoint().y, endPoint().y);
	maximum.z = EoMax(startPoint().z, endPoint().z);
}
bool EoGeLineSeg3d::GetParallels(double distanceBetweenLines, double eccentricity, EoGeLineSeg3d& leftLine, EoGeLineSeg3d& rightLine) const {
	leftLine = *this;
	rightLine = *this;

	const double LengthOfLines = length();

	if (LengthOfLines > FLT_EPSILON) {
		const double X = (endPoint().y - startPoint().y) * distanceBetweenLines / LengthOfLines;
		const double Y = (endPoint().x - startPoint().x) * distanceBetweenLines / LengthOfLines;

		leftLine.translateBy(OdGeVector3d(- X * eccentricity, Y * eccentricity, 0.));
		rightLine.translateBy(OdGeVector3d(X * (1. - eccentricity), - Y * (1. - eccentricity), 0.));

		return true;
	}
	return false;
}
bool EoGeLineSeg3d::IntersectWith_xy(const EoGeLineSeg3d& line, OdGePoint3d& intersection) const {
	OdGeVector3d Start1End1(endPoint() - startPoint());
	const OdGeVector3d Start2End2(line.endPoint() - line.startPoint());

	const double Determinant = Start1End1.x * Start2End2.y - Start2End2.x * Start1End1.y;

	if (fabs(Determinant) > DBL_EPSILON) {
		const OdGeVector3d Start1Start2(line.startPoint() - startPoint());

		const double dT = (Start1Start2.y * Start2End2.x - Start2End2.y * Start1Start2.x) / Determinant;

		Start1End1 *= dT;
		intersection = startPoint() - Start1End1;
		return true;
	}
	return false;
}

bool EoGeLineSeg3d::IntersectWithInfinite(const EoGeLineSeg3d& line, OdGePoint3d& intersection) {
	OdGeLine3d InfiniteFirstLine;
	getLine(InfiniteFirstLine);
	OdGeLine3d InfiniteSecondLine;
	line.getLine(InfiniteSecondLine);

	return InfiniteFirstLine.intersectWith(InfiniteSecondLine, intersection);
}

bool EoGeLineSeg3d::IsContainedBy_xy(const OdGePoint3d& lowerLeftPoint, const OdGePoint3d& upperRightPoint) const {
	OdGePoint3d	pt[2];
	pt[0] = startPoint();
	pt[1] = endPoint();

	const double dX = endPoint().x - startPoint().x;
	const double dY = endPoint().y - startPoint().y;
	int  i = 1;

	int iOut[2];
	iOut[0] = RelationshipToRectangleOf(pt[0], lowerLeftPoint, upperRightPoint);

	for (;;) {
		iOut[i] = RelationshipToRectangleOf(pt[i], lowerLeftPoint, upperRightPoint);

		if (iOut[0] == 0 && iOut[1] == 0)
			return true;
		if ((iOut[0] & iOut[1]) != 0)
			return false;
		i = (iOut[0] == 0) ? 1 : 0;

		if ((iOut[i] & 1) == 1) { // Above window
			pt[i].x = pt[i].x + dX * (upperRightPoint.y - pt[i].y) / dY;
			pt[i].y = upperRightPoint.y;
		}
		else if ((iOut[i] & 2) == 2) { // Below window
			pt[i].x = pt[i].x + dX * (lowerLeftPoint.y - pt[i].y) / dY;
			pt[i].y = lowerLeftPoint.y;
		}
		else if ((iOut[i] & 4) == 4) {
			pt[i].y = pt[i].y + dY * (upperRightPoint.x - pt[i].x) / dX;
			pt[i].x = upperRightPoint.x;
		}
		else {
			pt[i].y = pt[i].y + dY * (lowerLeftPoint.x - pt[i].x) / dX;
			pt[i].x = lowerLeftPoint.x;
		}
	}
}
bool EoGeLineSeg3d::IsSelectedBy_xy(const OdGePoint3d& point, const double apert, OdGePoint3d& ptProj, double& relationship) const {
	if (point.x < EoMin(startPoint().x, endPoint().x) - apert) return false;
	if (point.x > EoMax(startPoint().x, endPoint().x) + apert) return false;
	if (point.y < EoMin(startPoint().y, endPoint().y) - apert) return false;
	if (point.y > EoMax(startPoint().y, endPoint().y) + apert) return false;

	double dPBegX = startPoint().x - point.x;
	double dPBegY = startPoint().y - point.y;

	double dBegEndX = endPoint().x - startPoint().x;
	double dBegEndY = endPoint().y - startPoint().y;

	double dDivr = dBegEndX * dBegEndX + dBegEndY * dBegEndY;
	double DistanceSquared;

	if (dDivr <= DBL_EPSILON) {
		relationship = 0.;
		DistanceSquared = dPBegX * dPBegX + dPBegY * dPBegY;
	}
	else {
		relationship = - (dPBegX * dBegEndX + dPBegY * dBegEndY) / dDivr;
		relationship = EoMax(0., EoMin(1., relationship));
		const double dx = dPBegX + relationship * dBegEndX;
		const double dy = dPBegY + relationship * dBegEndY;
		DistanceSquared = dx * dx + dy * dy;
	}
	if (DistanceSquared > apert * apert)
		return false;

	ptProj.x = startPoint().x + (relationship * dBegEndX);
	ptProj.y = startPoint().y + (relationship * dBegEndY);

	return true;
}
bool EoGeLineSeg3d::ParametricRelationshipOf(const OdGePoint3d& point, double& relationship) const {
	OdGeVector3d Vector(endPoint() - startPoint());

	if (fabs(Vector.x) > DBL_EPSILON) {
		relationship = (point.x - startPoint().x) / Vector.x;
		return true;
	}
	if (fabs(Vector.y) > DBL_EPSILON) {
		relationship = (point.y - startPoint().y) / Vector.y;
		return true;
	}
	if (fabs(Vector.z) > DBL_EPSILON) {
		relationship = (point.z - startPoint().z) / Vector.z;
		return true;
	}
	return false;
}
OdGePoint3d EoGeLineSeg3d::ProjPt(const OdGePoint3d& point) const {
	OdGeVector3d vBegEnd(endPoint() - startPoint());

	const double dSum = vBegEnd.lengthSqrd();

	if (dSum > DBL_EPSILON) {
		const OdGeVector3d vBegPt(point - startPoint());

		const double dScale = vBegPt.dotProduct(vBegEnd) / dSum;

		vBegEnd *= dScale;
	}
	return (startPoint() + vBegEnd);
}
int EoGeLineSeg3d::ProjPtFrom_xy(double parallelDistance, double perpendicularDistance, OdGePoint3d& projectedPoint) {
	double dX = endPoint().x - startPoint().x;
	double dY = endPoint().y - startPoint().y;

	double dLen = sqrt(dX * dX + dY * dY);

	if (dLen <= DBL_EPSILON)
		return (FALSE);

	double dRatio;
	projectedPoint = startPoint();
	if (fabs(parallelDistance) > DBL_EPSILON) {
		dRatio = parallelDistance / dLen;
		dLen = parallelDistance;
		dX = dRatio * dX;
		dY = dRatio * dY;
		projectedPoint.x = startPoint().x + dX;
		projectedPoint.y = startPoint().y + dY;
	}
	if (fabs(perpendicularDistance) > DBL_EPSILON) {
		dRatio = perpendicularDistance / dLen;
		projectedPoint.x -= dRatio * dY;
		projectedPoint.y += dRatio * dX;
	}
	return (TRUE);
}
OdGePoint3d EoGeLineSeg3d::ProjToBegPt(double distance) {
	OdGeVector3d vEndBeg(startPoint() - endPoint());

	const double dLen = vEndBeg.length();

	if (dLen > DBL_EPSILON)
		vEndBeg *= distance / dLen;

	return (endPoint() + vEndBeg);
}
OdGePoint3d EoGeLineSeg3d::ProjToEndPt(double distance) {
	OdGeVector3d vBegEnd(endPoint() - startPoint());

	const double dLen = vBegEnd.length();

	if (dLen > DBL_EPSILON)
		vBegEnd *= distance / dLen;

	return (startPoint() + vBegEnd);
}
void EoGeLineSeg3d::SetEndPoint(const OdGePoint3d& endPoint) {
	set(startPoint(), endPoint);
}
void EoGeLineSeg3d::SetStartPoint(const OdGePoint3d& startPoint) {
	set(startPoint, endPoint());
}