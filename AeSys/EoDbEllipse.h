#pragma once
#include "OdaCommon.h"
#include "EoDbPrimitive.h"
#include "DbEllipse.h"

class EoDbEllipse : public EoDbPrimitive {
DECLARE_DYNAMIC(EoDbEllipse)
private:
	OdGePoint3d m_Center {OdGePoint3d::kOrigin};
	OdGeVector3d m_MajorAxis {OdGeVector3d::kXAxis};
	OdGeVector3d m_MinorAxis {OdGeVector3d::kYAxis};
	double m_SweepAngle {Oda2PI};
public: // Constructors and destructor
	EoDbEllipse() = default;
	/// <summary>Ellipse segment is constructed using a center point, a major and minor vector and a sweep ang.</summary>
	EoDbEllipse(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, double sweepAngle) noexcept;
	/// <summary>Ellipse is constructed using a center point and a radius about view plane normal</summary>
	EoDbEllipse(const OdGePoint3d& center, const OdGeVector3d& planeNormal, double radius);
	EoDbEllipse(const EoDbEllipse& other);
	const EoDbEllipse& operator=(const EoDbEllipse& other) noexcept;
	~EoDbEllipse() = default;

	// Methods - absolute virtuals
	void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;
	[[nodiscard]] EoDbPrimitive* Clone(OdDbBlockTableRecordPtr blockTableRecord) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	[[nodiscard]] OdGePoint3d GetCtrlPt() const noexcept override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& geometry) const override;
	/// <summary>Determines the extent. Actually the extents of the bounding region of the arc.</summary>
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	[[nodiscard]] OdGePoint3d GoToNxtCtrlPt() const override;
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept override;
	bool IsInView(AeSysView* view) const override;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	/// <summary>Determines the best control point on arc within specified tolerance. Control points for arcs are the points at start and end of the sweep.</summary>
	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	/// <summary>Determines if a line crosses arc.</summary>
	bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) override;
	bool SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	bool SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const override;
	void TransformBy(const EoGeMatrix3d& transformMatrix) override;
	void TranslateUsingMask(const OdGeVector3d& translate, unsigned long mask) override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, unsigned char* buffer) const override;

	// Methods
	[[nodiscard]] OdGePoint3d Center() const noexcept;
	void CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) override;
	void CutAt2Points(OdGePoint3d* points, EoDbGroupList* groups, EoDbGroupList* newGroups, OdDbDatabasePtr database) override;
	/// <summary>Generates a set of points which may be used to represent a arc using a double angle algorithm.</summary>
	void GenPts(const OdGePlane& plane, double sweepAngle) const;
	/// <summary>Determines the bounding region. This is always a quad, but it may not be xy oriented.</summary>
	void GetBoundingBox(OdGePoint3dArray&) const;
	[[nodiscard]] OdGePoint3d EndPoint() const;
	[[nodiscard]] OdGeVector3d MajorAxis() const noexcept;
	[[nodiscard]] OdGeVector3d MinorAxis() const noexcept;
	[[nodiscard]] double SweepAngle() const noexcept;
	void GetXYExtents(OdGePoint3d, OdGePoint3d, OdGePoint3d*, OdGePoint3d*) noexcept;
	int IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) override;
	void SetCenter(const OdGePoint3d& center) noexcept;
	void SetMajorAxis(const OdGeVector3d& majorAxis) noexcept;
	void SetMinorAxis(const OdGeVector3d& minorAxis) noexcept;
	void SetSweepAngle(double angle) noexcept;
	/// <summary>Ellipse is set to non-radial elliptical segment defined by a center point, a major and minor vector and a sweep ang.</summary>
	EoDbEllipse& SetTo2(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, double sweepAngle);
	/// <summary>Ellipse if set to a radial arc defined by three points</summary>
	EoDbEllipse& SetTo3PointArc(const OdGePoint3d& startPoint, const OdGePoint3d& intermediatePoint, const OdGePoint3d& endPoint);
	EoDbEllipse& SetToCircle(const OdGePoint3d& center, const OdGeVector3d& planeNormal, double radius);
	[[nodiscard]] OdGePoint3d StartPoint() const;
	double SwpAngToPt(const OdGePoint3d& point);

	// Methods - static
	static EoDbEllipse* Create(OdDbEllipsePtr& ellipse);
	static OdDbEllipsePtr Create(OdDbBlockTableRecordPtr& blockTableRecord);
	static OdDbEllipsePtr Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file);
	static OdDbEllipsePtr Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, int versionNumber);
	static EoDbEllipse* Create3(const EoDbEllipse& ellipse, OdDbBlockTableRecordPtr& blockTableRecord);
	static OdDbEllipsePtr CreateCircle(OdDbBlockTableRecordPtr& blockTableRecord, const OdGePoint3d& center, const OdGeVector3d& normal, double radius);
};

OdGePoint3d pFndPtOnArc(const OdGePoint3d& center, const OdGeVector3d& majorAxis, const OdGeVector3d& minorAxis, double angle);
int pFndSwpAngGivPlnAnd3Lns(const OdGeVector3d& planeNormal, const OdGePoint3d&, const OdGePoint3d&, const OdGePoint3d&, const OdGePoint3d&, double& sweepAngle);
