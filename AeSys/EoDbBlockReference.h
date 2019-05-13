#pragma once

#include "EoDb.h"
#include "EoDbPrimitive.h"

class EoDbPegFile;

/* <remarks>
GroupReference(SegRef) primitive in Peg files and Tracing files
  Type code <0x0102>	            OdUInt16[0-1]
  Pen color				            OdUInt16[2-3]
  Line type				            OdUInt16[4-5]
  Group name			            string
  Insertion point		            OdGePoint3d
  Local normal vector	            OdGeVector3d
  Scale factors(x, y, z)            OdGeVector3d
  Rotation				            double
  Number of columns		            OdUInt16
  Number of rows		            OdUInt16
  Column spacing		            double
  Row spacing			            double
</remarks> */

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

	EoDbBlockReference() noexcept;
	EoDbBlockReference(const EoDbBlockReference& other);
	const EoDbBlockReference& operator=(const EoDbBlockReference& other);

	~EoDbBlockReference();

public: // Methods - absolute virtuals

	void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const override;
	EoDbPrimitive* Clone(OdDbDatabasePtr& database) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& geometry) const override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	OdGePoint3d	GetCtrlPt() const noexcept override;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	OdGePoint3d	GoToNxtCtrlPt() const noexcept override;
	bool Is(OdUInt16 type) const noexcept override { return type == EoDb::kGroupReferencePrimitive; }
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept override;
	bool IsInView(AeSysView* view) const override;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const noexcept override;
	void Read(CFile&);
	OdGePoint3d	SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	bool SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	/// <summary>Evaluates whether a point lies within tolerance specified of block.</summary>
	bool SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const override;
	void TransformBy(const EoGeMatrix3d& transformMatrix) override;
	void TranslateUsingMask(const OdGeVector3d& translate, const DWORD mask) override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, OdUInt8* buffer) const noexcept override;

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
	void SetNormal(const OdGeVector3d& normal) noexcept { m_Normal = normal; }
	void SetPosition(const OdGePoint3d& position) noexcept { m_Position = position; }
	void SetPosition2(const OdGePoint3d& position);
	void SetScaleFactors(const OdGeScale3d& scaleFactors) noexcept;
	void SetRotation(double rotation) noexcept;
	void SetRows(OdUInt16 rows) noexcept;
	void SetRowSpacing(double rowSpacing) noexcept;
	void SetColumns(OdUInt16 columns) noexcept;
	void SetColumnSpacing(double columnSpacing) noexcept;

public: // Methods - static

	static EoDbBlockReference* Create(const EoDbBlockReference& other, OdDbDatabasePtr database);
	static EoDbBlockReference* Create(OdDbDatabasePtr& database);
	static OdDbBlockReferencePtr Create(OdDbBlockTableRecordPtr blockTableRecord);
	static OdDbBlockReferencePtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);

	static EoDbBlockReference* Create(OdDbBlockReferencePtr line);
};
