#pragma once

class EoDbPegFile;

class EoDbBlockReference : public EoDbPrimitive {
	CString m_Name;
	OdGePoint3d m_Position;
	OdGeVector3d m_Normal;
	OdGeScale3d m_ScaleFactors;
	double m_Rotation;

// Multiple inserts - not implemented
	OdUInt16 m_Columns;
	double m_ColumnSpacing;
	OdUInt16 m_Rows;
	double m_RowSpacing;

public: // Constructors and destructor
	EoDbBlockReference();
	EoDbBlockReference(const EoDbBlockReference& other);
	virtual ~EoDbBlockReference();

public: // Operators
	const EoDbBlockReference& operator=(const EoDbBlockReference& other);

public: // Methods - absolute virtuals
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const;
	void AssociateWith(OdDbBlockTableRecordPtr& blockTableRecord);
	EoDbPrimitive* Clone(OdDbDatabasePtr& database) const;
	void Display(AeSysView* view, CDC* deviceContext);
	void AddReportToMessageList(const OdGePoint3d& point) const;
	void GetAllPoints(OdGePoint3dArray& points) const;
	void FormatExtra(CString& extra) const;
	void FormatGeometry(CString& geometry) const;
	OdGePoint3d	GetCtrlPt() const noexcept;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const;
	OdGePoint3d	GoToNxtCtrlPt() const noexcept;
	bool Is(OdUInt16 type) const noexcept;
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept;
	bool IsInView(AeSysView* view) const;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const noexcept;
	void Read(CFile&);
	OdGePoint3d	SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const;
	/// <summary>Evaluates whether a point lies within tolerance specified of block.</summary>
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const;
	void TransformBy(const EoGeMatrix3d& transformMatrix);
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD mask);
	bool Write(EoDbFile& file) const;
	void Write(CFile& file, OdUInt8* buffer) const noexcept;
public: // Methods
	EoGeMatrix3d BlockTransformMatrix(const OdGePoint3d& basePoint) const;
	OdUInt16 Columns() const noexcept;
	double ColumnSpacing() const noexcept;
	CString Name() const;
	OdGeVector3d Normal() const noexcept;
	OdGePoint3d Position() const noexcept;
	double Rotation() const noexcept;
	OdUInt16 Rows() const noexcept;
	double RowSpacing() const noexcept;
	OdGeScale3d ScaleFactors() const noexcept;

	void SetName(const CString& name);
	void SetNormal(const OdGeVector3d& normal);
	void SetPosition(const OdGePoint3d& position);
	void SetScaleFactors(const OdGeScale3d& scaleFactors) noexcept;
	void SetRotation(double rotation) noexcept;
	void SetRows(OdUInt16 rows) noexcept;
	void SetRowSpacing(double rowSpacing) noexcept;
	void SetColumns(OdUInt16 columns) noexcept;
	void SetColumnSpacing(double columnSpacing) noexcept;

public: // Methods - static
	static EoDbBlockReference* ConstructFrom(EoDbFile& file);
	static EoDbBlockReference* Create(OdDbDatabasePtr database);
	static EoDbBlockReference* Create(const EoDbBlockReference& other, OdDbDatabasePtr database);
};
