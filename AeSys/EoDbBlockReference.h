#pragma once
#include <Ge/GeScale3d.h>
#include <DbBlockReference.h>
#include "EoDbPrimitive.h"
class EoDbPegFile;

/* <remarks>
GroupReference(SegRef) primitive in Peg files and Tracing files
  Type code <0x0102>	            unsigned short[0-1]
  Pen color				            unsigned short[2-3]
  Line type				            unsigned short[4-5]
  Group name			            string
  Insertion point		            OdGePoint3d
  Local normal vector	            OdGeVector3d
  Scale factors(x, y, z)            OdGeVector3d
  Rotation				            double
  Number of columns		            unsigned short
  Number of rows		            unsigned short
  Column spacing		            double
  Row spacing			            double
</remarks> */
class EoDbBlockReference final : public EoDbPrimitive {
DECLARE_DYNAMIC(EoDbBlockReference)

	CString m_Name;
	OdGePoint3d m_Position {OdGePoint3d::kOrigin};
	OdGeVector3d m_Normal {OdGeVector3d::kZAxis};
	OdGeScale3d m_ScaleFactors {OdGeScale3d::kIdentity};
	double m_Rotation {0.0};

	// Multiple inserts - not implemented
	unsigned short m_Columns {1};
	double m_ColumnSpacing {0.0};
	unsigned short m_Rows {1};
	double m_RowSpacing {0.0};

	EoDbBlockReference() = default;

	EoDbBlockReference(const EoDbBlockReference& other);

	EoDbBlockReference& operator=(const EoDbBlockReference& other); // hides non-virtual function of parent
	void AddReportToMessageList(const OdGePoint3d& point) const override;

	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const override;

	[[nodiscard]] EoDbPrimitive* Clone(OdDbBlockTableRecordPtr blockTableRecord) const override;

	void Display(AeSysView* view, CDC* deviceContext) override;

	void FormatExtra(CString& extra) const override;

	void FormatGeometry(CString& geometry) const override;

	void GetAllPoints(OdGePoint3dArray& points) const override;

	[[nodiscard]] OdGePoint3d GetCtrlPt() const noexcept override;

	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;

	[[nodiscard]] OdGePoint3d GoToNxtCtrlPt() const noexcept override;

	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept override;

	bool IsInView(AeSysView* view) const override;

	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const noexcept override;

	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;

	bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) override;

	bool SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	/// <summary>Evaluates whether a point lies within tolerance specified of block.</summary>
	bool SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const override;

	void TransformBy(const EoGeMatrix3d& transformMatrix) override;

	void TranslateUsingMask(const OdGeVector3d& translate, unsigned long mask) override;

	bool Write(EoDbFile& file) const override;

	void Write(CFile& file, unsigned char* buffer) const noexcept override;

	[[nodiscard]] EoGeMatrix3d BlockTransformMatrix(const OdGePoint3d& basePoint) const;

	[[nodiscard]] unsigned short Columns() const noexcept;

	[[nodiscard]] double ColumnSpacing() const noexcept;

	[[nodiscard]] CString Name() const;

	[[nodiscard]] OdGeVector3d Normal() const noexcept;

	[[nodiscard]] OdGePoint3d Position() const noexcept;

	[[nodiscard]] double Rotation() const noexcept;

	[[nodiscard]] unsigned short Rows() const noexcept;

	[[nodiscard]] double RowSpacing() const noexcept;

	[[nodiscard]] OdGeScale3d ScaleFactors() const noexcept;

	void SetName(const wchar_t* name);

	void SetNormal(const OdGeVector3d& normal) noexcept { m_Normal = normal; }

	void SetPosition(const OdGePoint3d& position) noexcept { m_Position = position; }

	void SetPosition2(const OdGePoint3d& position);

	void SetScaleFactors(const OdGeScale3d& scaleFactors) noexcept;

	void SetRotation(double rotation) noexcept;

	void SetRows(unsigned short rows) noexcept;

	void SetRowSpacing(double rowSpacing) noexcept;

	void SetColumns(unsigned short columns) noexcept;

	void SetColumnSpacing(double columnSpacing) noexcept;

	static EoDbBlockReference* Create(const EoDbBlockReference& other, OdDbDatabasePtr database);

	static EoDbBlockReference* Create(OdDbDatabasePtr& database);

	static OdDbBlockReferencePtr Create(OdDbBlockTableRecordPtr blockTableRecord);

	static OdDbBlockReferencePtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);

	static EoDbBlockReference* Create(OdDbBlockReferencePtr blockReference);
};
