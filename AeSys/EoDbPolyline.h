#pragma once
#include <DbPolyline.h>
/* <remarks>
Polyline primitive(never made it release : if already written flags not streamed)
  Type code <0x2002>                unsigned short[0-1]
  Pen color                         unsigned short[2-3]
  Line type                         unsigned short[4-5]
  Flags                             unsigned short[6-7]
  Constant width                    double[8-11]
  Elevation                         double[12-15]
  Thickness                         double[16-19]
  Normal                            OdGeVector3d[20-31]
  Number of points                  unsigned short[32-33]
  {0 or more Vertices} {Vertex, StartWidth, EndWidth, Bulge}
									{OdGePoint2d, double, double, double} [34- ]
</remarks> */
class EoDbPolyline final : public EoDbPrimitive {
DECLARE_DYNAMIC(EoDbPolyline)

private:
	static unsigned ms_EdgeToEvaluate;
	static unsigned ms_Edge;
	static unsigned ms_PivotVertex;
	static const unsigned short ms_Closed = 0x0001U;
	unsigned short m_Flags {0};
	double m_ConstantWidth {0.0};
	double m_Elevation {0.0};
	double m_Thickness {0.0};
	OdGeVector3d m_Normal {OdGeVector3d::kZAxis};
	OdGePoint2dArray m_Vertices;
	OdGeDoubleArray m_StartWidths;
	OdGeDoubleArray m_EndWidths;
	OdGeDoubleArray m_Bulges;
public:
	EoDbPolyline();

	EoDbPolyline(const EoDbPolyline& other);

	EoDbPolyline& operator=(const EoDbPolyline& other);

	~EoDbPolyline() = default;

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

	bool IsEqualTo(EoDbPrimitive* /*primitive*/) const noexcept override { return false; }

	bool IsInView(AeSysView* view) const override;

	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const noexcept override;

	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;

	bool SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const override;

	bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) override;

	bool SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;

	void TransformBy(const EoGeMatrix3d& transformMatrix) override;

	void TranslateUsingMask(const OdGeVector3d& translate, unsigned mask) override;

	bool Write(EoDbFile& file) const override;

	void Write(CFile& file, unsigned char* buffer) const noexcept override;

	void AppendVertex(const OdGePoint2d& vertex, double bulge = 0.0, double startWidth = 0.0, double endWidth = 0.0);

	void GetPointAt(unsigned vertexIndex, OdGePoint3d& point) const;

	[[nodiscard]] bool IsClosed() const noexcept;

	void SetClosed(bool closed) noexcept;

	bool PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) noexcept override;

	void SetConstantWidth(double constantWidth) noexcept;

	void SetElevation(const double elevation) noexcept { m_Elevation = elevation; }

	void SetNormal(const OdGeVector3d& normal);

	[[nodiscard]] unsigned SwingVertex() const;

	static unsigned Edge() noexcept;

	static void SetEdgeToEvaluate(unsigned edgeToEvaluate) noexcept;

	static EoDbPolyline* Create(OdDbPolylinePtr polyline);

	static OdDbPolylinePtr Create(OdDbBlockTableRecordPtr blockTableRecord);

	static OdDbPolylinePtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);
	// <tas="No construction from job files."/>
};
