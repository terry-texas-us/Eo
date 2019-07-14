#pragma once
#include <DbLine.h>
#include "EoGeLineSeg3d.h"
#include "EoDbPrimitive.h"

class EoDbLine final : public EoDbPrimitive {
DECLARE_DYNAMIC(EoDbLine)

private:
	EoGeLineSeg3d m_LineSeg;
public:
	EoDbLine() = default;

	EoDbLine(const EoDbLine& other);

	EoDbLine& operator=(const EoDbLine& other); // hides non-virtual function of parent
	~EoDbLine() override = default;

	void AddReportToMessageList(const OdGePoint3d& point) const override;

	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;

	[[nodiscard]] EoDbPrimitive* Clone(OdDbBlockTableRecordPtr blockTableRecord) const override;

	void Display(AeSysView* view, CDC* deviceContext) override;

	void FormatExtra(CString& extra) const override;

	void FormatGeometry(CString& geometry) const override;

	void GetAllPoints(OdGePoint3dArray& points) const override;

	[[nodiscard]] OdGePoint3d GetCtrlPt() const override;

	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;

	[[nodiscard]] OdGePoint3d GoToNxtCtrlPt() const override;

	bool IsEqualTo(EoDbPrimitive* primitive) const override;

	bool IsInView(AeSysView* view) const override;

	bool IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const override;

	OdGePoint3d SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const override;

	bool SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) override;

	bool SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const override;

	bool SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const override;

	void TransformBy(const EoGeMatrix3d& transformMatrix) override;

	void TranslateUsingMask(const OdGeVector3d& translate, unsigned mask) override;

	bool Write(EoDbFile& file) const override;

	void Write(CFile& file, unsigned char* buffer) const override;

	/// <summary>Cuts a line a point.</summary>
	void CutAt(const OdGePoint3d& point, EoDbGroup* newGroup) override;

	void CutAt2Points(OdGePoint3d* points, EoDbGroupList* groupsOut, EoDbGroupList* groupsIn, OdDbDatabasePtr database) override;

	int IsWithinArea(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, OdGePoint3d* intersections) override;

	[[nodiscard]] OdGePoint3d EndPoint() const {
		return m_LineSeg.endPoint();
	}

	[[nodiscard]] EoGeLineSeg3d LineSeg() const {
		return m_LineSeg;
	}

	[[nodiscard]] double Length() const {
		return m_LineSeg.length();
	}

	[[nodiscard]] OdGePoint3d ProjPt_(const OdGePoint3d& point) const;

	[[nodiscard]] double ParametricRelationshipOf(const OdGePoint3d& point) const;

	void SetEndPoint(const OdGePoint3d& endPoint);

	void SetStartPoint(const OdGePoint3d& startPoint);

	void Square(AeSysView* view);

	[[nodiscard]] OdGePoint3d StartPoint() const {
		return m_LineSeg.startPoint();
	}

	static EoDbLine* Create(const OdDbLinePtr& line);

	static OdDbLinePtr Create(OdDbBlockTableRecordPtr blockTableRecord);

	static OdDbLinePtr Create(OdDbBlockTableRecordPtr blockTableRecord, EoDbFile& file);

	static OdDbLinePtr Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, int versionNumber);

	static OdDbLinePtr Create(OdDbBlockTableRecordPtr blockTableRecord, const OdGePoint3d& startPoint, const OdGePoint3d& endPoint);
};
