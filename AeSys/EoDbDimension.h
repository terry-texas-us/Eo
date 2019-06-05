#pragma once

#include "DbAlignedDimension.h"
#include "DbRotatedDimension.h"

#include "EoGeLineSeg3d.h"

#include "EoDbFontDefinition.h"
#include "EoDbPrimitive.h"

class EoDbDimension : public EoDbPrimitive {
	DECLARE_DYNAMIC(EoDbDimension)

	EoGeLineSeg3d m_Line;

	short	m_TextColorIndex;
	EoDbFontDefinition m_FontDefinition;
	EoGeReferenceSystem m_ReferenceSystem;
	CString m_strText;

public:	// Constructors and destructor

	EoDbDimension();
	EoDbDimension(const EoDbDimension& other);
	const EoDbDimension& operator=(const EoDbDimension& other);

	~EoDbDimension();

public: // Methods - absolute virtuals

	void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;
	EoDbPrimitive* Clone(OdDbBlockTableRecordPtr blockTableRecord) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& geometry) const override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	OdGePoint3d GetCtrlPt() const override;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	OdGePoint3d GoToNxtCtrlPt() const override;
	bool IsEqualTo(EoDbPrimitive* primitive) const noexcept override;
	bool IsInView(AeSysView* view) const override;
	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;
	/// <summary>Evaluates whether a line intersects a dimension line.</summary>
	bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) override;
	bool SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;
	bool SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d&) const override;
	void TransformBy(const EoGeMatrix3d& transformMatrix) override;
	void TranslateUsingMask(const OdGeVector3d& translate, const unsigned long) override;
	bool Write(EoDbFile& file) const override;
	void Write(CFile& file, unsigned char* buffer) const override;

public:	// Methods - virtuals

	void CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) override;
	void CutAt2Points(OdGePoint3d* points, EoDbGroupList*, EoDbGroupList*, OdDbDatabasePtr database) override;
	void ModifyState() noexcept override;

public:	// Methods

	void GetBoundingBox(OdGePoint3dArray& boundingBox, double spaceFactor) const;
	const EoDbFontDefinition& FontDef() noexcept;
	const EoGeLineSeg3d& Line() noexcept;
	void GetPts(OdGePoint3d& ptBeg, OdGePoint3d& ptEnd);
	EoGeReferenceSystem ReferenceSystem() const;
	double Length() const;
	double ParametricRelationshipOf(const OdGePoint3d& point) const;
	void SetDefaultNote();
	void SetFontDefinition(const EoDbFontDefinition& fontDefinition) noexcept;
	void SetStartPoint(const OdGePoint3d& startPoint);
	void SetEndPoint(const OdGePoint3d& endPoint);
	void SetReferenceSystem(const EoGeReferenceSystem& referenceSystem) noexcept;
	void SetText(const CString& text);
	void SetTextHorizontalAlignment(EoDb::HorizontalAlignment horizontalAlignment) noexcept;
	void SetTextColorIndex(short colorIndex) noexcept;
	void SetTextVerticalAlignment(EoDb::VerticalAlignment verticalAlignment) noexcept;
	const CString& Text() noexcept;
	const short& TextColorIndex() noexcept;

private:
	static unsigned short sm_wFlags;	// bit 1 clear if dimension selected at note, set if dimension selected at line

public: // Methods - static
	static EoDbDimension* Create(OdDbAlignedDimensionPtr& alignedDimension);

	static OdDbAlignedDimensionPtr Create(OdDbBlockTableRecordPtr blockTableRecord);
	static OdDbAlignedDimensionPtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);
	static OdDbAlignedDimensionPtr Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, int versionNumber);

};
