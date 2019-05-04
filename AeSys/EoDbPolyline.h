#pragma once

#include "EoDb.h"
#include "EoDbPrimitive.h"

/* <remarks>
Polyline primitive(never made it release : if already written flags not streamed)
  Type code <0x2002>                OdUInt16[0-1]
  Pen color                         OdUInt16[2-3]
  Line type                         OdUInt16[4-5]
  Flags                             OdUInt16[6-7]
  Constant width                    double[8-11]
  Elevation                         double[12-15]
  Thickness                         double[16-19]
  Normal                            OdGeVector3d[20-31]
  Number of points                  OdUInt16[32-33]
  {0 or more Vertices} {Vertex, StartWidth, EndWidth, Bulge}
									{OdGePoint2d, double, double, double} [34- ]
</remarks> */

class EoDbPolyline : public EoDbPrimitive {
	static size_t sm_EdgeToEvaluate;
	static size_t sm_Edge;
	static size_t sm_PivotVertex;

public:
	static const OdUInt16 sm_Closed = 0x0001;

private:
	OdUInt16 m_Flags;
	double m_ConstantWidth;
	double m_Elevation;
	double m_Thickness;
	OdGeVector3d m_Normal;
	OdGePoint2dArray m_Vertices;
	OdGeDoubleArray m_StartWidths;
	OdGeDoubleArray m_EndWidths;
	OdGeDoubleArray m_Bulges;

public:	// Constructors and destructor

	EoDbPolyline();
	EoDbPolyline(const EoDbPolyline& other);
	const EoDbPolyline& operator=(const EoDbPolyline& other);

	~EoDbPolyline();

public: // Methods - absolute virtuals

	void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;
	EoDbPrimitive* Clone(OdDbDatabasePtr& database) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& geometry) const override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	OdGePoint3d	GetCtrlPt() const override;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	OdGePoint3d	GoToNxtCtrlPt() const override;
	bool Is(OdUInt16 type) const noexcept override { return type == EoDb::kPolylinePrimitive; }
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept override { return false; }
	bool IsInView(AeSysView* view) const override;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const noexcept override;
	OdGePoint3d	SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const override;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	void TransformBy(const EoGeMatrix3d& transformMatrix) override;
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD) override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, OdUInt8* buffer) const noexcept override;

public: // Methods

	void AppendVertex(const OdGePoint2d& vertex, double bulge = 0., double startWidth = 0., double endWidth = 0.);
	void GetPointAt(int vertexIndex, OdGePoint3d& point) const;
	bool IsClosed() const noexcept;
	void SetClosed(bool closed) noexcept;
	bool PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) noexcept override;
	void SetConstantWidth(double constantWidth) noexcept;
	void SetElevation(double elevation) noexcept { m_Elevation = elevation; }
	void SetNormal(const OdGeVector3d& normal);
	size_t SwingVertex() const;

public: // Methods - static

	static size_t Edge() noexcept;
	static void SetEdgeToEvaluate(size_t edgeToEvaluate) noexcept;

	static EoDbPolyline* Create(OdDbPolylinePtr polyline);

	static OdDbPolylinePtr Create(OdDbBlockTableRecordPtr blockTableRecord);
	static OdDbPolylinePtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);
	// <tas="No contruction from job files."/>
};
