#pragma once
#include "EoGeNurbCurve3d.h"
#include "EoDbPrimitive.h"

/* <remarks>
Spline primitive
  Type code <0x2000>                unsigned short[0-1]
  Pen color                         unsigned short[2-3]
  Line type                         unsigned short[4-5]
  Number of control points          unsigned short[6-7]
  {0 or more control points}        OdGePoint3d[8- ]
</remarks> */
class EoDbSpline : public EoDbPrimitive {
	EoGeNurbCurve3d m_Spline;
public:	// Constructors and destructor
	EoDbSpline() = default;
	EoDbSpline(const EoDbSpline& other);
	const EoDbSpline& operator=(const EoDbSpline& other);
	~EoDbSpline() = default;

	void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;
	[[nodiscard]] EoDbPrimitive* Clone(OdDbBlockTableRecordPtr blockTableRecord) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& geometry) const override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	[[nodiscard]] OdGePoint3d GetCtrlPt() const override;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	[[nodiscard]] OdGePoint3d GoToNxtCtrlPt() const override;
	bool IsEqualTo(EoDbPrimitive* other) const override;
	bool IsInView(AeSysView* view) const override;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const noexcept override;
	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) override;
	bool SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	bool SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const override;
	void TransformBy(const EoGeMatrix3d& transformMatrix) override;
	void TranslateUsingMask(const OdGeVector3d& translate, unsigned long mask) override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, unsigned char* buffer) const override;

	// Methods
	void Set(int degree, const OdGeKnotVector& knots, const OdGePoint3dArray& controlPoints, const OdGeDoubleArray& weights, bool isPeriodic = false);

	// Methods - static
	static EoDbSpline* Create(OdDbSplinePtr& spline);
	static OdDbSplinePtr Create(OdDbBlockTableRecordPtr& blockTableRecord);
	static OdDbSplinePtr Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file);
	static OdDbSplinePtr Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, int versionNumber);
};
