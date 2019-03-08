#pragma once

#include "Ge/GeNurbCurve3d.h"

class EoGeNurbCurve3d : public OdGeNurbCurve3d {

public:	// Constructors and destructor
	EoGeNurbCurve3d();
	EoGeNurbCurve3d(const EoGeNurbCurve3d& spline);
	
	virtual ~EoGeNurbCurve3d();

public: // Methods

public: // Methods - static
	/// <summary>
	/// Generates the required B-spline curves of various order (capped to 4th) using the Cox and de Boor algorithm.
	/// </summary>
	/// <remarks>
	///	If the order equals the number of vertices, and there are no multiple vertices, a Bezier curve will
	///	be generated. As the order decreases the curve produced will lie closer to the defining polyline.
	///	When the order is two the generated curve is a series of straight lines which are identical to the
	///	defining polyline.  Increasing the order "tightens" the curve.	Additional shape control can be obtained by
	///	use of repeating vertices. The degree of the spline, which is one less than the order.
	/// </remarks>
	static int GeneratePoints(const EoGeNurbCurve3d& spline);
	static void SetDefaultKnotVector(int degree, const OdGePoint3dArray& controlPoints, OdGeKnotVector& knots);
};