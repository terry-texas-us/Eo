#pragma once
#include "DbPoint.h"
#include "EoDbPrimitive.h"

class EoDbPoint final : public EoDbPrimitive {
DECLARE_DYNAMIC(EoDbPoint)

private:
	OdGePoint3d m_Position;
	short m_PointDisplayMode;
	unsigned short m_NumberOfDatums;
	double* m_Data;
public:
	EoDbPoint() noexcept;

	EoDbPoint(const OdGePoint3d& point) noexcept;

	EoDbPoint(const EoDbPoint& other);

	EoDbPoint& operator=(const EoDbPoint& other);

	~EoDbPoint() override;

	void AddReportToMessageList(const OdGePoint3d& point) const override;

	void AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept override;

	[[nodiscard]] EoDbPrimitive* Clone(OdDbBlockTableRecordPtr blockTableRecord) const override;

	void Display(AeSysView* view, CDC* deviceContext) override;

	void FormatExtra(CString& extra) const override;

	void FormatGeometry(CString& geometry) const override;

	void GetAllPoints(OdGePoint3dArray& points) const override;

	[[nodiscard]] OdGePoint3d GetCtrlPt() const noexcept override;

	void GetExtents(AeSysView* view, OdGeExtents3d& extents) const override;

	[[nodiscard]] OdGePoint3d GoToNxtCtrlPt() const noexcept override;

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

	[[nodiscard]] double DataAt(unsigned short dataIndex) const noexcept;

	void ModifyState() noexcept override;

	[[nodiscard]] short PointDisplayMode() const noexcept;

	[[nodiscard]] OdGePoint3d Position() const noexcept;

	void SetData(unsigned short numberOfDatums, double* data);

	void SetPointDisplayMode(short displayMode) noexcept;

	static EoDbPoint* Create(OdDbPointPtr& point);

	static OdDbPointPtr Create(OdDbBlockTableRecordPtr& blockTableRecord);

	static OdDbPointPtr Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file);

	static OdDbPointPtr Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, int versionNumber);
};
