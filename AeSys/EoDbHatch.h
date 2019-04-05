#pragma once

#include "DbHatch.h"

class EoDbHatch : public EoDbPrimitive {
	static size_t sm_EdgeToEvaluate;
	static size_t sm_Edge;
	static size_t sm_PivotVertex;

public:
	static double sm_PatternAngle;
	static double sm_PatternScaleX;
	static double sm_PatternScaleY;

	static OdStringArray sm_HatchNames;
	static int sm_HatchPatternOffsets[];
	static double sm_HatchPatternTable[];

public:
	enum InteriorStyle {
		kHollow,
		kSolid,
		kPattern,
		kHatch
	};

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

	~EoDbHatch();

public: // Operators
	const EoDbHatch& operator=(const EoDbHatch& other);

public: // Methods - absolute virtuals
	void AddReportToMessageList(const OdGePoint3d& point) const;
	void AssociateWith(OdDbBlockTableRecordPtr blockTableRecord);
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept;
	EoDbPrimitive* Clone(OdDbDatabasePtr database) const;
	void Display(AeSysView* view, CDC* deviceContext);
	void FormatExtra(CString& extra) const;
	void FormatGeometry(CString& geometry) const;
	void GetAllPoints(OdGePoint3dArray& points) const;
	OdGePoint3d	GetCtrlPt() const;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const;
	OdGePoint3d	GoToNxtCtrlPt() const;
	bool Is(OdUInt16 type) const noexcept;
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept {return false;}
	bool IsInView(AeSysView* view) const;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	OdGePoint3d	SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const;
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD);
	bool Write(EoDbFile& file) const;
	void Write(CFile& file, OdUInt8* buffer) const;

public: // Methods
	int Append(const OdGePoint3d& vertex);
	/// <summary>A Hatch is generated using line patterns.</summary>
	void DisplayHatch(AeSysView* view, CDC* deviceContext) const;
	void DisplaySolid(AeSysView* view, CDC* deviceContext) const;
	CString FormatInteriorStyle() const;
	OdGePoint3d GetPointAt(int pointIndex);
	void ModifyState() noexcept;
	int NumberOfVertices() const;
	bool PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point) noexcept;
	OdGeVector3d RecomputeReferenceSystem();
	void RetrieveHatchPattern(const OdString& hatchPatternName, OdHatchPattern& hatchPattern) const;
	void SetHatchOrigin(const OdGePoint3d& origin) noexcept;
	void SetHatchXAxis(const OdGeVector3d& xAxis) noexcept;
	void SetHatchYAxis(const OdGeVector3d& yAxis) noexcept;
	void SetHatRefVecs(double patternAngle, double patternScaleX, double patternScaleY);
	void SetInteriorStyle(OdInt16 interiorStyle) noexcept;
	void SetInteriorStyleIndex(size_t styleIndex);
	void SetLoopAt(int loopIndex, OdDbHatchPtr& hatchEntity);
	void SetPatternReferenceSystem(const OdGePoint3d& origin, const OdGeVector3d& normal, double patternAngle, double patternScale);
	void SetVertices(const OdGePoint3dArray& vertices);
	size_t SwingVertex() const;

public:
	static size_t Edge() noexcept;
	static size_t LegacyInteriorStyleIndex(const OdString& name);
	static void SetEdgeToEvaluate(size_t edgeToEvaluate) noexcept;

    static EoDbHatch* ConstructFrom(EoDbFile& file);
    static EoDbHatch* ConstructFrom(OdUInt8* primitiveBuffer, int versionNumber);

    static EoDbHatch* Create(const EoDbHatch& other, OdDbDatabasePtr database);

    static EoDbHatch* Create(OdDbDatabasePtr database);
    static OdDbHatchPtr Create(OdDbBlockTableRecordPtr blockTableRecord);
    static OdDbHatchPtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);

    static EoDbHatch* Create(OdDbHatchPtr hatch);

    static void ConvertPolylineType(int loopIndex, OdDbHatchPtr &hatchEntity, EoDbHatch* hatchPrimitive);
    static void ConvertCircularArcEdge(OdGeCurve2d* edge);
    static void ConvertEllipticalArcEdge(OdGeCurve2d* edge);
    static void ConvertNurbCurveEdge(OdGeCurve2d* edge);
    static void ConvertEdgesType(int loopIndex, OdDbHatchPtr &hatchEntity, EoDbHatch* hatchPrimitive);
};