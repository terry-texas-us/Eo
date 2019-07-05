#pragma once
#include "EoDbPrimitive.h"
#include "DbHatch.h"

/* <remarks>
Hatch(Polygon) primitive
  Type code <0x0400>                EoUInt16[0-1]
  Pen color                         EoUInt16[2-3]
  Polygon style                     EoUInt16[4-5]
  Polygon Style Index               EoUInt16[6-7]
  Number of vertices                EoUInt16[8-9]
  Hatch origin                      EoGePoint3d[10-13][14-17][18-21]
  Hatch / pattern reference x-axis  EoGeVector3d[22-25][26-29][30-33]
  Hatch / pattern reference y-axis  EoGeVector3d[34-37][38-41][42-45]
  {0 or more points}                EoGePoint3d[46- ]
</remarks> */
class EoDbHatch final : public EoDbPrimitive {
DECLARE_DYNAMIC(EoDbHatch)

private:
	static unsigned ms_EdgeToEvaluate;
	static unsigned ms_Edge;
	static unsigned ms_PivotVertex;
public:
	static double patternAngle;
	static double patternScaleX;
	static double patternScaleY;

	enum InteriorStyle { kHollow, kSolid, kPattern, kHatch };

private:
	short m_InteriorStyle {kHatch};
	unsigned m_InteriorStyleIndex {1};
	OdGePoint3d m_HatchOrigin {OdGePoint3d::kOrigin};
	OdGeVector3d m_HatchXAxis {OdGeVector3d::kXAxis};
	OdGeVector3d m_HatchYAxis {OdGeVector3d::kYAxis};
	int m_NumberOfLoops {0};
	OdGePoint3dArray m_Vertices;
	OdGePoint2dArray m_Vertices2d;
	OdGeDoubleArray m_Bulges;
public:
	EoDbHatch() noexcept;

	EoDbHatch(const EoDbHatch& other);

	EoDbHatch& operator=(const EoDbHatch& other);

	~EoDbHatch() = default;

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

	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const override;

	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;

	bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) override;

	bool SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;

	bool SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const override;

	void TransformBy(const EoGeMatrix3d& transformMatrix) override;

	void TranslateUsingMask(const OdGeVector3d& translate, unsigned mask) override;

	bool Write(EoDbFile& file) const override;

	void Write(CFile& file, unsigned char* buffer) const override;

	int Append(const OdGePoint3d& vertex);
	/// <summary>A Hatch is generated using line patterns.</summary>
	void DisplayHatch(AeSysView* view, CDC* deviceContext) const;

	void DisplaySolid(AeSysView* view, CDC* deviceContext) const;

	[[nodiscard]] CString FormatInteriorStyle() const;

	OdGePoint3d GetPointAt(unsigned pointIndex);

	void ModifyState() noexcept override;

	[[nodiscard]] int NumberOfVertices() const;

	bool PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) noexcept override;

	OdGeVector3d RecomputeReferenceSystem();

	void SetHatchOrigin(const OdGePoint3d& origin) noexcept;

	void SetHatchXAxis(const OdGeVector3d& xAxis) noexcept;

	void SetHatchYAxis(const OdGeVector3d& yAxis) noexcept;

	void SetHatchReferenceAxes(double patternAngle, double patternScaleX, double patternScaleY);

	void SetInteriorStyle(short interiorStyle) noexcept;

	void SetInteriorStyleIndex(const unsigned styleIndex) noexcept { m_InteriorStyleIndex = styleIndex; }

	void SetInteriorStyleIndex2(unsigned styleIndex);

	void SetLoopAt(int loopIndex, const OdDbHatchPtr& hatchEntity);

	void SetPatternReferenceSystem(const OdGePoint3d& origin, const OdGeVector3d& normal, double patternAngle, double patternScale);

	[[nodiscard]] unsigned SwingVertex() const;

	static unsigned Edge() noexcept;

	static void SetEdgeToEvaluate(unsigned edgeToEvaluate) noexcept;

	static void ConvertPolylineType(int loopIndex, const OdDbHatchPtr& hatchEntity, EoDbHatch* hatchPrimitive);

	static void ConvertCircularArcEdge(OdGeCurve2d* edge) noexcept;

	static void ConvertEllipticalArcEdge(OdGeCurve2d* edge) noexcept;

	static void ConvertNurbCurveEdge(OdGeCurve2d* edge) noexcept;

	static void ConvertEdgesType(int loopIndex, const OdDbHatchPtr& hatchEntity, EoDbHatch* hatchPrimitive);

	static void AppendLoop(const OdGePoint3dArray& vertices, OdDbHatchPtr& hatch);

	static EoDbHatch* Create(const OdDbHatchPtr& hatch);

	static OdDbHatchPtr Create(OdDbBlockTableRecordPtr blockTableRecord);

	static OdDbHatchPtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);

	static OdDbHatchPtr Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, int versionNumber);
};
