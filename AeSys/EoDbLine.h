#pragma once

class EoDbLine : public EoDbPrimitive {
	EoGeLineSeg3d m_Line;

public: // Constructors and destructor
	EoDbLine();
	EoDbLine(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint);
	EoDbLine(const EoDbLine& other);

	~EoDbLine();

public: // Operators
	const EoDbLine& operator=(const EoDbLine& other);

public: // Methods - absolute virtuals
	void AddReportToMessageList(const OdGePoint3d& point) const;
	void AssociateWith(OdDbBlockTableRecordPtr blockTableRecord);
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const;
	EoDbPrimitive* Clone(OdDbDatabasePtr database) const;
	void Display(AeSysView* view, CDC* deviceContext);
	void FormatExtra(CString& extra) const;
	void FormatGeometry(CString& str) const;
	void GetAllPoints(OdGePoint3dArray& points) const;
	OdGePoint3d GetCtrlPt() const;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const;
	OdGePoint3d GoToNxtCtrlPt() const;
	bool Is(EoUInt16 type) const;
	bool IsEqualTo(EoDbPrimitive* primitive) const;
	bool IsInView(AeSysView* view) const;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const;
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD);
	bool Write(EoDbFile& file) const;
	void Write(CFile& file, EoByte* buffer) const;

public: // Methods - virtuals
	/// <summary>Cuts a line a point.</summary>
	void CutAt(const OdGePoint3d& point, EoDbGroup*, OdDbDatabasePtr database);
	void CutAt2Points(OdGePoint3d* points, EoDbGroupList*, EoDbGroupList*, OdDbDatabasePtr database);
	int IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections);
	bool SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections);

public: // Methods
	OdGePoint3d EndPoint() const;
	void GetLine(EoGeLineSeg3d& line) const;
	void GetPoints(OdGePoint3d& startPoint, OdGePoint3d& endPoint);
	EoGeLineSeg3d Line() const;
	double Length() const;
	OdGePoint3d ProjPt_(const OdGePoint3d& point) const;
	double ParametricRelationshipOf(const OdGePoint3d& point) const;
	void SetEndPoint_(const OdGePoint3d& endPoint) {m_Line.SetEndPoint(endPoint);}
	void SetEndPoint(const OdGePoint3d& endPoint);
	void SetStartPoint_(const OdGePoint3d& startPoint) { m_Line.SetStartPoint(startPoint); }
	void SetStartPoint(const OdGePoint3d& startPoint);
	EoDbLine& SetTo(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint);
	void Square(AeSysView* view);
	OdGePoint3d StartPoint() const;

public: // Methods - static
	static EoDbLine* ConstructFrom(EoDbFile& file);
	static EoDbLine* ConstructFrom(EoByte* primitiveBuffer, int versionNumber);

	static EoDbLine* Create(const EoDbLine& line, OdDbDatabasePtr database);
	static EoDbLine* Create(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint);
	static EoDbLine* Create(OdDbDatabasePtr database);
	static OdDbLinePtr Create(OdDbDatabasePtr database, OdDbBlockTableRecordPtr blockTableRecord);
	static EoDbLine* Create(OdDbLinePtr line);
};
