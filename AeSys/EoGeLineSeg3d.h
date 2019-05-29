#pragma once

#include "Ge/GeLineSeg3d.h"

class AeSysView;
class EoGeLineSeg3d : public OdGeLineSeg3d {

public:	// Constructors and destructor
	EoGeLineSeg3d();
	EoGeLineSeg3d(const EoGeLineSeg3d& line);
	EoGeLineSeg3d(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint);
	virtual ~EoGeLineSeg3d();

public: // Methods
	/// <summary>Determines the angle between two lines.</summary>
	/// <notes>
	///	Angle is found using the inner product.
	///						  v1 dot v2
	///		 ang = acos (-------------------)
	///					  len(v1) * len(v2)
	///	   Angle is between 0 and 2 pi. If angle is 0 lines are in same
	///	   direction and if angle is pi lines are in opposite direction.
	///	   To get acute angle, all angles greater than half pi must be
	///	   subtracted from pi.
	/// </notes>
	/// <returns>angle between lines (in radians)</returns>
	/// <param name="line">other line</param>
	double AngleBetween_xy(const EoGeLineSeg3d& line) const;
	/// <summary> Determines the angle of a line defined by 2 points. </summary>
	/// <remarks> /// If null length or parallel to z-axis, angle is 0. </remarks>
	/// <returns> The angle (in radians) from the X axis (0 to TWOPI) to a point (x,y). </returns>
	double AngleFromXAxis_xy() const;
	/// <summary> Constrains a line to nearest axis pivoting on first endpoint.</summary>
	/// <remarks> Offset angle only support about z-axis </remarks>
	/// <returns> Point after snap </returns>
	OdGePoint3d ConstrainToAxis(double influenceAngle, double offsetAngle) const;
	/// <summary>Cuts a line a point.</summary>
	unsigned short CutAt(const OdGePoint3d& point, EoGeLineSeg3d& line);
	/// <summary>Determines which side of a directed line a point is on.</summary>
	/// <remarks>
	///Relation is found using determinant (3rd order).
	///   d  = begx * (endy - y) - endx * (begy - y) + x * (begy - endy)
	/// </remarks>
	/// <returns>
	/// 1 point is to left of line
	/// 0 point is on line
	/// - 1 point is to right of line
	/// </returns>
	int DirectedRelationshipOf(const OdGePoint3d& point) const;
	void Display(AeSysView* view, CDC* deviceContext);
	/// <summary>Determines the extents of a line.</summary>
	void Extents(OdGePoint3d& minimum, OdGePoint3d& maximum);
	/// <summary>Generates coordinate sets for parallel lines.</summary>
	/// <remarks>
	/// Eccentricity is a function of the distance between the lines.
	/// The first of the two parallel lines lies to the left of line, and the second to the right.
	/// </remarks>
	/// <param name="eccentricity">
	/// In general; left is (eccentricity * distanceBetweenLines) to the left of this line.
	///			 right is distanceBetweenLines to the right of the left line
	/// Left Justifification (0.0) left line on this line and right line is distanceBetweenLines to right of this line
	/// Center Justification (.5) left and right lines the same distance from this line
	/// Right Justifification (1.0) right line on this line and left line is distanceBetweenLines to left of this line
	/// </param>
	bool GetParallels(double dDis, double eccentricity, EoGeLineSeg3d& leftLine, EoGeLineSeg3d& rightLine) const;
	/// <summary>Determines intersection of two lines.</summary>
	/// <returns>true successful completion, false ohterwise (parallel lines)</returns>
	bool IntersectWith_xy(const EoGeLineSeg3d& line, OdGePoint3d& intersection) const;
	/// <summary> Determines if line segment in wholly or partially contained within window passed.</summary>
	/// <remarks> Assumes window passed with min/max corners correct.</remarks>
	/// <returns> true line is wholly or partially within window, false otherwise</returns>

	bool IntersectWithInfinite(const EoGeLineSeg3d& line, OdGePoint3d& intersection);

	bool IsContainedBy_xy(const OdGePoint3d& lowerLeftPoint, const OdGePoint3d& upperRightPoint) const;
	/// <summary>
	///Evaluates the proximity of a point to a line segment.
	/// </summary>
	/// <remarks>
	///The value of parameter t is found from:
	///		   Rel = -[(Begx - Px)(Endx - Begx) + (Begy - Py)(Endy - Begy)]
	///				 ------------------------------------------------------
	///						(Endx - Begx)^2 + (Endy - Begy)^2
	/// </remarks>
	/// <returns>
	/// true if point is within acceptance aperture of line segment
	///	false otherwise
	/// </returns>
	bool IsSelectedBy_xy(const OdGePoint3d& point, const double apert, OdGePoint3d& ptProj, double& relationship) const;
	/// <summary>Projects a point onto line.</summary>
	OdGePoint3d	ProjPt(const OdGePoint3d& point) const;
	/// <summary>Determines the coordinates of point projected along a line.</summary>
	/// <remarks>
	///t = 0 point is the start point
	///t = 1 point is the end point
	///The range of values for t is not clamped to this interval
	/// </remarks>
	/// <returns>
	///The start point of the line projected toward (if positive t) the end point of the line.
	/// </returns>
	/// <summary>
	///Projects a point from start point toward end point the parallel projection distance.
	///The resulting point is then projected perpendicular to the line defined by the two points the
	///perpendicular projection distance.
	/// </summary>
	/// <remarks>
	///	A positive perpendicular projection distance will result in a point to
	/// the left of the direction vector defined by the two points.
	/// Projected point is undefined if point one and point two coincide.
	/// </remarks>
	/// <returns>TRUE  successful completion and FALSE failure (p1 and p2 coincide)</returns>
	int ProjPtFrom_xy(double parallelDistance, double perpendicularDistance, OdGePoint3d& projectedPoint);
	/// <summary>Projects end point toward or beyond the start point of line.</summary>
	OdGePoint3d	ProjToBegPt(double distance);
	/// <summary>Projects start point toward or beyond the end point of line.</summary>
	OdGePoint3d	ProjToEndPt(double distance);
	/// <summary>Determines the relationship of a point on a line to the endpoints defining the line.</summary>
	/// <param name="relationship">parametric relationship of point to line endpoints
	///	 less than 0 - point to left of directed segment
	///	 equal to 0 - point same as first endpoint of line
	///	 between 0 and 1 - point between endpoints of line
	///	 equal to 1 - point same as second endpoint of line
	///	 greater than 1 - point to right of directed segment
	/// Results unpredictable if point does not lie on the line.
	/// </param>
	/// <returns> true  successful completion, false coincidental endpoints .. relationship undefined</returns>
	bool ParametricRelationshipOf(const OdGePoint3d& point, double& relationship) const;
	void SetEndPoint(const OdGePoint3d& endPoint);
	void SetStartPoint(const OdGePoint3d& startPoint);
};
