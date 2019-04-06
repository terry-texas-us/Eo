#pragma once

using namespace EoDb;

/* <tas="">
Spline primitive
  Type code <0x2000>	        EoUInt16[0-1]
  Pen color				        EoUInt16[2-3]
  Line type				        EoUInt16[4-5]
  Number of control points	    EoUInt16[6-7]
  {0 or more control points}	EoGePoint3d[8- ]
</tas> */

class EoDbSpline : public EoDbPrimitive {
	EoGeNurbCurve3d m_Spline;

public:	// Constructors and destructor
	EoDbSpline() noexcept;
	EoDbSpline(const EoDbSpline& other);

	~EoDbSpline();
public: // Operators
	const EoDbSpline& operator=(const EoDbSpline& other);

public: // Methods - absolute virtuals
	void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;
    void AssociateWith(OdDbBlockTableRecordPtr& blockTableRecord) override;
    EoDbPrimitive* Clone(OdDbDatabasePtr& database) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& geometry) const override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	OdGePoint3d	GetCtrlPt() const override;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	OdGePoint3d	GoToNxtCtrlPt() const override;
    bool Is(OdUInt16 type) const noexcept override {return type == kSplinePrimitive;}
    bool IsEqualTo(EoDbPrimitive* primitive) const override;
	bool IsInView(AeSysView* view) const override;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const noexcept override;
	OdGePoint3d	SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const override;
	void TransformBy(const EoGeMatrix3d& transformMatrix) override;
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD)override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, OdUInt8* buffer) const override;

public: // Methods
	void Set(int degree, const OdGeKnotVector& knots, const OdGePoint3dArray& controlPoints, const OdGeDoubleArray& weights, bool isPeriodic = false);
	void SetControlPoints(const OdGePoint3dArray& controlPoints);

public: // Methods - static
	static EoDbSpline* ConstructFrom(EoDbFile& file);
	static EoDbSpline* ConstructFrom(OdUInt8* primitiveBuffer, int versionNumber);
    static EoDbSpline* Create(const EoDbSpline& spline, OdDbDatabasePtr& database);
    static EoDbSpline* Create(OdDbDatabasePtr& database);
    static OdDbSplinePtr Create(OdDbBlockTableRecordPtr& blockTableRecord);
    static OdDbSplinePtr Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file);

    static EoDbSpline* Create(OdDbSplinePtr& spline);
};
