#pragma once

class EoDbPoint : public EoDbPrimitive {
	EoInt16	m_PointDisplayMode;
	OdGePoint3d	m_Position;
	EoUInt16 m_NumberOfDatums;
	double* m_Data;

public: // Constructors and destructor
	EoDbPoint();
	EoDbPoint(const OdGePoint3d& point);

	EoDbPoint(const EoDbPoint& other);

	~EoDbPoint();

public: // Operators
	const EoDbPoint& operator=(const EoDbPoint& other);

public: // Methods - absolute virtuals
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const;
	void AssociateWith(OdDbBlockTableRecordPtr blockTableRecord);
	EoDbPrimitive* Clone(OdDbDatabasePtr database) const;
	void Display(AeSysView* view, CDC* deviceContext);
	void AddReportToMessageList(const OdGePoint3d& point) const;
	void FormatExtra(CString& extra) const;
	void FormatGeometry(CString& geometry) const;
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

public: // Methods
	double DataAt(EoUInt16 dataIndex) const;
	void ModifyState();
	EoInt16 PointDisplayMode() const;
	OdGePoint3d Position() const;
	void SetData(EoUInt16 numberOfDatums, double* data);
	void SetPointDisplayMode(EoInt16 displayMode);
	void SetPosition(const OdGePoint3d& point);

public: // Methods - static
	static EoDbPoint* ConstructFrom(EoDbFile& file);
	static EoDbPoint* ConstructFrom(EoByte* primitiveBuffer, int versionNumber);
	static EoDbPoint* Create(OdDbDatabasePtr database);
	static EoDbPoint* Create(const EoDbPoint& other, OdDbDatabasePtr database);
};
