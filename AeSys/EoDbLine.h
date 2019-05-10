#pragma once

#include "EoGeLineSeg3d.h"

#include "EoDbPrimitive.h"

class EoDbLine : public EoDbPrimitive {
	EoGeLineSeg3d m_LineSeg;

public: // Constructors and destructor
	
	EoDbLine() noexcept;
	EoDbLine(const EoDbLine& other);
	const EoDbLine& operator=(const EoDbLine& other);

	~EoDbLine();

public: // Methods - absolute virtuals
	
	void AddReportToMessageList(const OdGePoint3d& point) const override;
	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;
	EoDbPrimitive* Clone(OdDbDatabasePtr& database) const override;
	void Display(AeSysView* view, CDC* deviceContext) override;
	void FormatExtra(CString& extra) const override;
	void FormatGeometry(CString& str) const override;
	void GetAllPoints(OdGePoint3dArray& points) const override;
	OdGePoint3d GetCtrlPt() const override;
	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;
	OdGePoint3d GoToNxtCtrlPt() const override;
	bool Is(OdUInt16 type) const noexcept override { return type == EoDb::kLinePrimitive; }
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
	void CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) noexcept override;
	void CutAt2Points(OdGePoint3d* points, EoDbGroupList* groupsOut, EoDbGroupList* groupsIn, OdDbDatabasePtr& database) noexcept override;
	int IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) noexcept override;
	bool SelectBy(const EoGeLineSeg3d& line, AeSysView* view, OdGePoint3dArray& intersections) override;

public: // Methods

	OdGePoint3d EndPoint() const { return m_LineSeg.endPoint(); }
	EoGeLineSeg3d LineSeg() const { return m_LineSeg; }
	double Length()  const { return (m_LineSeg.length()); }
	OdGePoint3d ProjPt_(const OdGePoint3d& point) const;
	double ParametricRelationshipOf(const OdGePoint3d& point) const;
	void SetEndPoint2(const OdGePoint3d& endPoint);
	void SetStartPoint2(const OdGePoint3d& startPoint);
	void Square(AeSysView* view);
	OdGePoint3d StartPoint() const { return m_LineSeg.startPoint(); }


public: // Methods - static

	static EoDbLine* Create(const OdDbLinePtr& line);

	static OdDbLinePtr Create(OdDbBlockTableRecordPtr blockTableRecord);
	static OdDbLinePtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);
	static OdDbLinePtr Create(OdDbBlockTableRecordPtr blockTableRecord, OdUInt8* primitiveBuffer, int versionNumber);

	static OdDbLinePtr Create(OdDbBlockTableRecordPtr blockTableRecord, const OdGePoint3d& startPoint, const OdGePoint3d& endPoint);
};
