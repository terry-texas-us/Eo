#pragma once

#include "EoGePoint3d.h"

typedef CArray<EoGePoint4d, EoGePoint4d&> EoGePoint4dArray;

class EoGePoint4d {
public:
	double x;
	double y;
	double z;
private:
	double w;
public:
	double W() const noexcept {return w;}

public: // Constructors and destructor
	
    EoGePoint4d();
	EoGePoint4d(const OdGePoint3d& initialPoint, double initialW) noexcept;

public: // Operators

    void operator/=(const double t) noexcept;

	EoGePoint4d operator+(const OdGeVector3d& vector);
	EoGePoint4d operator-(const OdGeVector3d& vector);

	OdGeVector3d operator-(const EoGePoint4d& ptQ);

public: // Methods
	OdGePoint3d Convert3d() const;
	/// <summary>Determines the xy distance between two points.</summary>
	double DistanceToPointXY(const EoGePoint4d& ptQ) const noexcept;
	/// <summary>Performs a containment test on a point.</summary>
	bool IsInView() noexcept;
	EoGePoint4d& TransformBy(const EoGeMatrix3d& matrix) noexcept;

public: // Static
	static bool ClipLine(EoGePoint4d& ptA, EoGePoint4d& ptB);
	/// <summary>Sutherland-hodgman-like polygon clip by view volume.</summary>
	static void	ClipPolygon(EoGePoint4dArray& pointsArray);
	/// <summary>Sutherland-hodgman-like polygon clip by clip plane.</summary>
	/// <remarks>Visibility determined using dot product.</remarks>
	static void IntersectionWithPln(EoGePoint4dArray& pointsArrayIn, const OdGePoint3d& pointOnPlane, const OdGeVector3d& planeNormal, EoGePoint4dArray& pointsArrayOut);
	/// <summary>
	///Calculates intersection point of line and a plane. Should only be used if points are known to be on opposite sides of the plane.
	/// </summary>
	/// <param name="startPoint">start point of line</param>
	/// <param name="endPoint">end point of line</param>
	/// <param name="pointOnPlane">any point on clip plane</param>
	/// <param name="planeNormal">clip plane normal vector</param>
	/// <returns>Intersection point. If line and plane are parallel start point of line is returned. Not good!</returns>
	static EoGePoint4d IntersectionWithPln4(EoGePoint4d& startPoint, EoGePoint4d& endPoint, const EoGePoint4d& pointOnPlane, const OdGeVector3d& planeNormal) noexcept;
	static EoGePoint4d Max(EoGePoint4d& ptA, EoGePoint4d& ptB);
	static EoGePoint4d Min(EoGePoint4d& ptA, EoGePoint4d& ptB);
};