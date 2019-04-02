#pragma once

class EoDbEllipse : public EoDbPrimitive {
	OdGePoint3d	m_Center;
	OdGeVector3d m_MajorAxis;
	OdGeVector3d m_MinorAxis;
	double m_SweepAngle;

public: // Constructors and destructor
	EoDbEllipse();
	/// <summary>Ellipse segment is constructed using a center point, a major and minor vector and a sweep ang.</summary>
	EoDbEllipse(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, double sweepAngle);
	/// <summary>Ellipse is constructed using a center point and a radius about view plane normal</summary>
	EoDbEllipse(const OdGePoint3d& center, const OdGeVector3d& planeNormal, double radius);

	EoDbEllipse(const EoDbEllipse& ellipse);

	~EoDbEllipse();
public: // Operators
	const EoDbEllipse& operator=(const EoDbEllipse&) noexcept;

public: // Methods - absolute virtuals
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept;
	void AssociateWith(OdDbBlockTableRecordPtr blockTableRecord);
	EoDbPrimitive* Clone(OdDbDatabasePtr database) const;
	void Display(AeSysView* view, CDC* deviceContext);
	void AddReportToMessageList(const OdGePoint3d& point) const;
	void GetAllPoints(OdGePoint3dArray& points) const;
	OdGePoint3d GetCtrlPt() const noexcept;
	void FormatExtra(CString& extra) const;
	void FormatGeometry(CString& geometry) const;
	/// <summary>Determines the extent. Actually the extents of the bounding region of the arc.</summary>
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const;
	OdGePoint3d GoToNxtCtrlPt() const;
	bool Is(OdUInt16 type) const noexcept;
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept;
	bool IsInView(AeSysView* view) const;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	/// <summary>Determines the best control point on arc within specified tolerance. Control points for arcs are the points at start and end of the sweep.</summary>
	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	/// <summary>Determines if a line crosses arc.</summary>
	bool SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections);
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const;
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD);
	bool Write(EoDbFile& file) const;
	void Write(CFile& file, OdUInt8* buffer) const;

public: // Methods
	OdGePoint3d Center() const noexcept;
	void CutAt(const OdGePoint3d& point, EoDbGroup*, OdDbDatabasePtr database) noexcept;
	void CutAt2Points(OdGePoint3d* points, EoDbGroupList*, EoDbGroupList*, OdDbDatabasePtr database) noexcept;
	/// <summary>Generates a set of points which may be used to represent a arc using a double angle algorithm.</summary>
	void GenPts(const OdGePlane& plane, double sweepAngle) const;
	/// <summary>Determines the bounding region. This is always a quad, but it may not be xy oriented.</summary>
	void GetBoundingBox(OdGePoint3dArray&) const;
	OdGePoint3d EndPoint() const;
	OdGeVector3d MajorAxis() const noexcept;
	OdGeVector3d MinorAxis() const noexcept;
	double SweepAngle() const noexcept;
	void GetXYExtents(OdGePoint3d, OdGePoint3d, OdGePoint3d*, OdGePoint3d*) noexcept;
	int IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) noexcept;
	void SetCenter(const OdGePoint3d& center) noexcept;
	void SetMajorAxis(const OdGeVector3d& majorAxis) noexcept;
	void SetMinorAxis(const OdGeVector3d& minorAxis) noexcept;
	void SetSweepAngle(double angle) noexcept;
	/// <summary>Ellipse is set to non-radial elliptical segment defined by a center point, a major and minor vector and a sweep ang.</summary>
	EoDbEllipse& SetTo(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, double sweepAngle);
	/// <summary>Ellipse if set to a radial arc defined by three points</summary>
	EoDbEllipse& SetTo3PointArc(const OdGePoint3d& startPoint, const OdGePoint3d& intermediatePoint, const OdGePoint3d& endPoint);
	EoDbEllipse& SetToCircle(const OdGePoint3d& center, const OdGeVector3d& unitNormal, double radius);
	OdGePoint3d StartPoint() const;
	double SwpAngToPt(const OdGePoint3d& point);

public: // Methods - static
	static EoDbEllipse* ConstructFrom(EoDbFile& file);
	static EoDbEllipse* ConstructFrom(OdUInt8* primitiveBufer, int versionNumber);
	static EoDbEllipse* Create(OdDbDatabasePtr database);
	static EoDbEllipse* Create(const EoDbEllipse& ellipse, OdDbDatabasePtr database);
};

OdGePoint3d pFndPtOnArc(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, const double);
int pFndSwpAngGivPlnAnd3Lns(const OdGeVector3d& planeNormal, const OdGePoint3d&, const OdGePoint3d&, const OdGePoint3d&, const OdGePoint3d&, double& sweepAngle);
/// <summary>Finds center point of a circle given radius and two tangent vectors.</summary>

// Notes:	A radius and two lines define four center points.  The center point
//			selected is on the concave side of the angle formed by the two vectors
//			defined by the line endpoints.	These two vectors are oriented with
//			the tail of the second vector at the head of the first.

// Returns: TRUE	center point determined
//			FALSE	endpoints of first line coincide or endpoints of second line coincide or
//					two lines are parallel or four points are not coplanar
bool pFndCPGivRadAnd4Pts(double radius, OdGePoint3d, OdGePoint3d, OdGePoint3d, OdGePoint3d, OdGePoint3d*);
