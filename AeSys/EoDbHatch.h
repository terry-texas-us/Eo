#pragma once

#include "EoDb.h"
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

class EoDbHatch : public EoDbPrimitive {
	static size_t sm_EdgeToEvaluate;
	static size_t sm_Edge;
	static size_t sm_PivotVertex;

public:
	static double sm_PatternAngle;
	static double sm_PatternScaleX;
	static double sm_PatternScaleY;

public:
	enum InteriorStyle { kHollow, kSolid, kPattern, kHatch };

private:
	OdInt16	m_InteriorStyle;
	size_t m_InteriorStyleIndex;
	OdGePoint3d	m_HatchOrigin;
	OdGeVector3d m_HatchXAxis;
	OdGeVector3d m_HatchYAxis;
	OdGePoint3dArray m_Vertices;

	int m_NumberOfLoops;
	OdGePoint2dArray m_Vertices2d;
	OdGeDoubleArray m_Bulges;

public:	// Constructors and destructor

    EoDbHatch();
	EoDbHatch(const EoDbHatch& other);
    const EoDbHatch& operator=(const EoDbHatch& other);

	~EoDbHatch();

public: // Methods - absolute virtuals

    void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AssociateWith(OdDbBlockTableRecordPtr& blockTableRecord) override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;
	EoDbPrimitive* Clone(OdDbDatabasePtr& database) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& geometry) const override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	OdGePoint3d	GetCtrlPt() const override;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	OdGePoint3d	GoToNxtCtrlPt() const override;
    bool Is(OdUInt16 type) const noexcept override {return type == EoDb::kHatchPrimitive;}
bool IsEqualTo(EoDbPrimitive* primitive) const noexcept override {return false;}
	bool IsInView(AeSysView* view) const override;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	OdGePoint3d	SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const override;
	void TransformBy(const EoGeMatrix3d& transformMatrix) override;
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD) override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, OdUInt8* buffer) const override;

public: // Methods
	int Append(const OdGePoint3d& vertex);
	/// <summary>A Hatch is generated using line patterns.</summary>
	void DisplayHatch(AeSysView* view, CDC* deviceContext) const;
	void DisplaySolid(AeSysView* view, CDC* deviceContext) const;
	CString FormatInteriorStyle() const;
	OdGePoint3d GetPointAt(int pointIndex);
	void ModifyState() noexcept override;
	int NumberOfVertices() const;
	bool PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) noexcept override;
	OdGeVector3d RecomputeReferenceSystem();
	void SetHatchOrigin(const OdGePoint3d& origin) noexcept;
	void SetHatchXAxis(const OdGeVector3d& xAxis) noexcept;
	void SetHatchYAxis(const OdGeVector3d& yAxis) noexcept;
	void SetHatRefVecs(double patternAngle, double patternScaleX, double patternScaleY);
	void SetInteriorStyle(OdInt16 interiorStyle) noexcept;
	void SetInteriorStyleIndex(size_t styleIndex);
	void SetLoopAt(int loopIndex, const OdDbHatchPtr& hatchEntity);
	void SetPatternReferenceSystem(const OdGePoint3d& origin, const OdGeVector3d& normal, double patternAngle, double patternScale);
	void SetVertices(const OdGePoint3dArray& vertices);
	size_t SwingVertex() const;

public:
	static size_t Edge() noexcept;
	static void SetEdgeToEvaluate(size_t edgeToEvaluate) noexcept;

    static EoDbHatch* ConstructFrom(OdUInt8* primitiveBuffer, int versionNumber);

    static EoDbHatch* Create1(const EoDbHatch& other, OdDbDatabasePtr database);

    static EoDbHatch* Create0(OdDbDatabasePtr database);
    static OdDbHatchPtr Create(OdDbBlockTableRecordPtr blockTableRecord);
    static OdDbHatchPtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);

    static EoDbHatch* Create(OdDbHatchPtr& hatch);

    static void ConvertPolylineType(int loopIndex, OdDbHatchPtr &hatchEntity, EoDbHatch* hatchPrimitive);
    static void ConvertCircularArcEdge(OdGeCurve2d* edge) noexcept;
    static void ConvertEllipticalArcEdge(OdGeCurve2d* edge) noexcept;
    static void ConvertNurbCurveEdge(OdGeCurve2d* edge) noexcept;
    static void ConvertEdgesType(int loopIndex, OdDbHatchPtr &hatchEntity, EoDbHatch* hatchPrimitive);
};