#pragma once

class EoDbPolyline : public EoDbPrimitive {
	static size_t sm_EdgeToEvaluate;
	static size_t sm_Edge;
	static size_t sm_PivotVertex;

public:
	static const EoUInt16 sm_Closed = 0x0010;

private:
	EoUInt16 m_Flags;
	double m_ConstantWidth;
	double m_Elevation;
	double m_Thickness;
	OdGePoint2dArray m_Vertices;
	OdGeDoubleArray m_StartWidths;
	OdGeDoubleArray m_EndWidths;
	OdGeDoubleArray m_Bulges;
	OdGeVector3d m_Normal;

public:	// Constructors and destructor
	EoDbPolyline();
	EoDbPolyline(const EoDbPolyline& other);

	~EoDbPolyline();

public: // Operators
	const EoDbPolyline& operator=(const EoDbPolyline& other);

public: // Methods - absolute virtuals
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const;
	void AssociateWith(OdDbBlockTableRecordPtr blockTableRecord);
	EoDbPrimitive* Clone(OdDbDatabasePtr database) const;
	void Display(AeSysView* view, CDC* deviceContext);
	void AddReportToMessageList(const OdGePoint3d& point) const;
	void FormatExtra(CString& extra) const;
	void FormatGeometry(CString& geometry) const;
	void GetAllPoints(OdGePoint3dArray& points) const;
	OdGePoint3d	GetCtrlPt() const;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const;
	OdGePoint3d	GoToNxtCtrlPt() const;
	bool Is(EoUInt16 type) const;
	bool IsEqualTo(EoDbPrimitive* primitive) const {return false;}
	bool IsInView(AeSysView* view) const;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	OdGePoint3d	SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const;
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD);
	bool Write(EoDbFile& file) const;
	void Write(CFile& file, EoByte* buffer) const;

public: // Methods
	void AppendVertex(const OdGePoint2d& vertex, double bulge = 0., double startWidth = - 1., double endWidth = - 1.);
	void GetPointAt(int vertexIndex, OdGePoint3d& point) const;
	bool IsClosed() const;
	void SetClosed(bool closed);
	bool PivotOnGripPoint(AeSysView* view, const EoGePoint4d& point);
	void SetConstantWidth(double constantWidth);
	void SetElevation(double elevation);
	void SetNormal(const OdGeVector3d& normal);
	void SetPoints(const OdGePoint3dArray& points);
	size_t SwingVertex() const;

public: // Methods - static
	static EoDbPolyline* ConstructFrom(EoDbFile& file);
	static EoDbPolyline* Create(OdDbDatabasePtr database);
	static EoDbPolyline* Create(const EoDbPolyline& other, OdDbDatabasePtr database);

	static size_t Edge();
	static void SetEdgeToEvaluate(size_t edgeToEvaluate);
};
