#pragma once

#include "EoDb.h"
#include "EoDbPrimitive.h"
#include "DbEllipse.h"

class EoDbEllipse : public EoDbPrimitive {
	OdGePoint3d	m_Center;
	OdGeVector3d m_MajorAxis;
	OdGeVector3d m_MinorAxis;
	double m_SweepAngle;

public: // Constructors and destructor

	EoDbEllipse() noexcept;
	/// <summary>Ellipse segment is constructed using a center point, a major and minor vector and a sweep ang.</summary>
	EoDbEllipse(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, double sweepAngle) noexcept;
	/// <summary>Ellipse is constructed using a center point and a radius about view plane normal</summary>
	EoDbEllipse(const OdGePoint3d& center, const OdGeVector3d& planeNormal, double radius);

	EoDbEllipse(const EoDbEllipse& other);
	const EoDbEllipse& operator=(const EoDbEllipse& other) noexcept;

	~EoDbEllipse();

public: // Methods - absolute virtuals

	void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;
	EoDbPrimitive* Clone(OdDbDatabasePtr& database) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	OdGePoint3d GetCtrlPt() const noexcept override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& geometry) const override;
	/// <summary>Determines the extent. Actually the extents of the bounding region of the arc.</summary>
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	OdGePoint3d GoToNxtCtrlPt() const override;
	bool Is(OdUInt16 type) const noexcept override { return type == EoDb::kEllipsePrimitive; }
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept override;
	bool IsInView(AeSysView* view) const override;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	/// <summary>Determines the best control point on arc within specified tolerance. Control points for arcs are the points at start and end of the sweep.</summary>
	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	/// <summary>Determines if a line crosses arc.</summary>
	bool SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections) override;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const override;
	void TransformBy(const EoGeMatrix3d& transformMatrix) override;
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD) override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, OdUInt8* buffer) const override;

public: // Methods
	OdGePoint3d Center() const noexcept;
	void CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) noexcept override;
	void CutAt2Points(OdGePoint3d* points, EoDbGroupList*, EoDbGroupList*, OdDbDatabasePtr& database) noexcept;
	/// <summary>Generates a set of points which may be used to represent a arc using a double angle algorithm.</summary>
	void GenPts(const OdGePlane& plane, double sweepAngle) const;
	/// <summary>Determines the bounding region. This is always a quad, but it may not be xy oriented.</summary>
	void GetBoundingBox(OdGePoint3dArray&) const;
	OdGePoint3d EndPoint() const;
	OdGeVector3d MajorAxis() const noexcept;
	OdGeVector3d MinorAxis() const noexcept;
	double SweepAngle() const noexcept;
	void GetXYExtents(OdGePoint3d, OdGePoint3d, OdGePoint3d*, OdGePoint3d*) noexcept;
	int IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) noexcept override;
	void SetCenter(const OdGePoint3d& center) noexcept;
	void SetMajorAxis(const OdGeVector3d& majorAxis) noexcept;
	void SetMinorAxis(const OdGeVector3d& minorAxis) noexcept;
	void SetSweepAngle(double angle) noexcept;
	/// <summary>Ellipse is set to non-radial elliptical segment defined by a center point, a major and minor vector and a sweep ang.</summary>
	EoDbEllipse& SetTo2(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, double sweepAngle);
	/// <summary>Ellipse if set to a radial arc defined by three points</summary>
	EoDbEllipse& SetTo3PointArc(const OdGePoint3d& startPoint, const OdGePoint3d& intermediatePoint, const OdGePoint3d& endPoint);
	EoDbEllipse& SetToCircle(const OdGePoint3d& center, const OdGeVector3d& unitNormal, double radius);
	OdGePoint3d StartPoint() const;
	double SwpAngToPt(const OdGePoint3d& point);

public: // Methods - static

	static EoDbEllipse* Create(OdDbEllipsePtr& ellipse);

	static OdDbEllipsePtr Create(OdDbBlockTableRecordPtr& blockTableRecord);
	static OdDbEllipsePtr Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file);
	static OdDbEllipsePtr Create(OdDbBlockTableRecordPtr blockTableRecord, OdUInt8* primitiveBufer, int versionNumber);

	static EoDbEllipse* Create3(const EoDbEllipse& ellipse, OdDbBlockTableRecordPtr& blockTableRecord);

	static OdDbEllipsePtr CreateCircle(OdDbBlockTableRecordPtr& blockTableRecord, const OdGePoint3d& center, const OdGeVector3d& normal, double radius);

};

OdGePoint3d pFndPtOnArc(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, const double);
int pFndSwpAngGivPlnAnd3Lns(const OdGeVector3d& planeNormal, const OdGePoint3d&, const OdGePoint3d&, const OdGePoint3d&, const OdGePoint3d&, double& sweepAngle);
/// <summary>Finds center point of a circle given radius and two tangent vectors.</summary>
/// <Notes>A radius and two lines define four center points. The center point selected is on the concave side of the angle formed by the two vectors defined by the line endpoints. These two vectors are oriented with the tail of the second vector at the head of the first.</notes>
/// <Returns>
/// true    center point determined
/// false   endpoints of first line coincide or endpoints of second line coincide or two lines are parallel or four points are not coplanar
/// </Returns>
bool pFndCPGivRadAnd4Pts(double radius, OdGePoint3d, OdGePoint3d, OdGePoint3d, OdGePoint3d, OdGePoint3d*);
