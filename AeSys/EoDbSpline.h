#pragma once

class EoDbSpline : public EoDbPrimitive {
	EoGeNurbCurve3d m_Spline;

public:	// Constructors and destructor
	EoDbSpline();
	EoDbSpline(const EoDbSpline& other);

	~EoDbSpline();
public: // Operators
	const EoDbSpline& operator=(const EoDbSpline& other);

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
	bool IsEqualTo(EoDbPrimitive* primitive) const;
	bool IsInView(AeSysView* view) const;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const noexcept;
	OdGePoint3d	SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const;
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const;
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD);
	bool Write(EoDbFile& file) const;
	void Write(CFile& file, OdUInt8* buffer) const;

public: // Methods
	void Set(int degree, const OdGeKnotVector& knots, const OdGePoint3dArray& controlPoints, const OdGeDoubleArray& weights, bool isPeriodic = false);
	void SetControlPoints(const OdGePoint3dArray& controlPoints);

public: // Methods - static
	static EoDbSpline* ConstructFrom(EoDbFile& file);
	static EoDbSpline* ConstructFrom(OdUInt8* primitiveBuffer, int versionNumber);
	static EoDbSpline* Create(OdDbDatabasePtr database);
	static EoDbSpline* Create(const EoDbSpline& other, OdDbDatabasePtr database);
};
