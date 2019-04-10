#pragma once

using namespace EoDb;

/* <remarks>
Hatch(Polygon) primitive
  Type code <0x0400>			    OdUInt16[0-1]
  Pen color				            OdUInt16[2-3]
  Polygon style				        OdUInt16[4-5]
  Polygon Style Index			    OdUInt16[6-7]
  Number of vertices			    OdUInt16[8-9]
  Hatch origin				        OdGePoint3d[10-13][14-17][18-21]
  Hatch/pattern reference x-axis	OdGeVector3d[22-25][26-29][30-33]
  Hatch/pattern reference y-axis  	OdGeVector3d[34-37][38-41][42-45]
  {0 or more points}			    OdGePoint3d[46- ]
  </remarks> */

class EoDbLine : public EoDbPrimitive {
	EoGeLineSeg3d m_Line;

public: // Constructors and destructor
	EoDbLine() noexcept;
	EoDbLine(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint);
	EoDbLine(const EoDbLine& other);
    const EoDbLine& operator=(const EoDbLine& other);

	~EoDbLine();

public: // Methods - absolute virtuals
	void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;
    void AssociateWith(OdDbBlockTableRecordPtr& blockTableRecord) override;
    EoDbPrimitive* Clone(OdDbDatabasePtr& database) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& str) const override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	OdGePoint3d GetCtrlPt() const override;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	OdGePoint3d GoToNxtCtrlPt() const override;
    bool Is(OdUInt16 type) const noexcept override {return type == kLinePrimitive;}
	bool IsEqualTo(EoDbPrimitive* primitive) const override;
	bool IsInView(AeSysView* view) const override;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const override;
	void TransformBy(const EoGeMatrix3d& transformMatrix) override;
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD) override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, OdUInt8* buffer) const override;

public: // Methods - virtuals

	/// <summary>Cuts a line a point.</summary>
	void CutAt(const OdGePoint3d& point, EoDbGroup*, OdDbDatabasePtr& database) override;
	void CutAt2Points(OdGePoint3d* points, EoDbGroupList*, EoDbGroupList*, OdDbDatabasePtr& database) override;
	int IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) noexcept override;
	bool SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections) override;

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

	static EoDbLine* ConstructFrom(OdUInt8* primitiveBuffer, int versionNumber);

	static EoDbLine* Create(const EoDbLine& line, OdDbDatabasePtr database);
	static EoDbLine* Create(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint);
	static EoDbLine* Create(OdDbDatabasePtr database);
    static OdDbLinePtr Create(OdDbBlockTableRecordPtr blockTableRecord);
    static OdDbLinePtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);

	static EoDbLine* Create(OdDbLinePtr line);
};
