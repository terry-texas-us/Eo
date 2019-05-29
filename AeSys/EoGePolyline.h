#pragma once

#include "EoGePoint4d.h"

class AeSysView;
class EoGeLineSeg3d;


namespace polyline {
	void BeginLineStrip();
	void BeginLineLoop();
	void __Display(AeSysView* view, CDC* deviceContext, EoGePoint4dArray& pointsArray, OdDbLinetypeTableRecordPtr linetype);
	void __End(AeSysView* view, CDC* deviceContext, short linetypeIndex);
	/// <summary>Determines points necessary to represent an N-Polygon with line segments.</summary>
	void GeneratePointsForNPoly(const OdGePoint3d& centerPoint, const OdGeVector3d& planeNormal, double radius, unsigned numberOfPoints, OdGePoint3dArray& points);
	/// <summary>Determines how many times (if any), a line segment intersects with polyline.</summary>
	bool SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections);
	bool SelectBy(const OdGePoint3d& lowerLeftPoint, const OdGePoint3d& upperRightPoint, AeSysView* view);
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, double& dRel, OdGePoint3d& ptProj);
	/// <summary>Determines if the polyline is within the rectangle defined by the input points</summary>
	bool SelectUsingRectangle(AeSysView* view, const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, const OdGePoint3dArray& points);
	void SetVertex(const OdGePoint3d& point);
}