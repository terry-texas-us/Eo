#include "stdafx.h"
#include <DbBlockTableRecord.h>
#include "AeSys.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoVaxFloat.h"
#include "EoDbFile.h"
IMPLEMENT_DYNAMIC(EoDbPoint, EoDbPrimitive)

EoDbPoint::EoDbPoint() noexcept
	: m_Position {OdGePoint3d::kOrigin}
	, m_NumberOfDatums {0}
	, m_Data {nullptr} {
	m_ColorIndex = 1;
	m_PointDisplayMode = 1;
}

EoDbPoint::EoDbPoint(const OdGePoint3d& point) noexcept
	: m_Position {point}
	, m_NumberOfDatums {0}
	, m_Data {nullptr} {
	m_ColorIndex = 1;
	m_PointDisplayMode = 1;
}

EoDbPoint::EoDbPoint(const EoDbPoint& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_PointDisplayMode = other.m_PointDisplayMode;
	m_Position = other.m_Position;
	m_NumberOfDatums = other.m_NumberOfDatums;
	m_Data = m_NumberOfDatums == 0 ? nullptr : new double[m_NumberOfDatums];
	for (unsigned DatumIndex = 0; DatumIndex < m_NumberOfDatums; DatumIndex++) {
		m_Data[DatumIndex] = other.m_Data[DatumIndex];
	}
}

EoDbPoint::~EoDbPoint() {
	if (m_NumberOfDatums != 0) { delete[] m_Data; }
}

EoDbPoint& EoDbPoint::operator=(const EoDbPoint& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;
	m_ColorIndex = other.m_ColorIndex;
	m_PointDisplayMode = other.m_PointDisplayMode;
	m_Position = other.m_Position;
	if (m_NumberOfDatums != other.m_NumberOfDatums) {
		if (m_NumberOfDatums != 0) { delete[] m_Data; }
		m_NumberOfDatums = other.m_NumberOfDatums;
		m_Data = m_NumberOfDatums == 0 ? nullptr : new double[m_NumberOfDatums];
	}
	for (unsigned DatumIndex = 0; DatumIndex < m_NumberOfDatums; DatumIndex++) {
		m_Data[DatumIndex] = other.m_Data[DatumIndex];
	}
	return *this;
}

void EoDbPoint::AddReportToMessageList(const OdGePoint3d& /*point*/) const {
	CString Report(L"<Point>");
	Report += L" Color:" + FormatColorIndex();
	CString Mode;
	Mode.Format(L" Point Display Mode:%d", m_PointDisplayMode);
	Report += Mode;
	AeSys::AddStringToMessageList(Report);
}

void EoDbPoint::AddToTreeViewControl(const HWND tree, const HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Point>", this);
}

EoDbPrimitive* EoDbPoint::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbPointPtr Point = m_EntityObjectId.safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(Point);
	return Create(Point);
}

void EoDbPoint::Display(AeSysView* view, CDC* deviceContext) {
	const auto ColorIndex {LogicalColorIndex()};
	const auto HotColor {AeSys::GetHotColor(ColorIndex)};
	EoGePoint4d pt(m_Position, 1.0);
	view->ModelViewTransformPoint(pt);
	if (pt.IsInView()) {
		const auto pnt {view->DoViewportProjection(pt)};
		switch (m_PointDisplayMode) {
			case 0:	// 3 pixel plus
				for (auto i = -1; i <= 1; i++) {
					deviceContext->SetPixel(pnt.x + i, pnt.y, HotColor);
					deviceContext->SetPixel(pnt.x, pnt.y + i, HotColor);
				}
				break;
			case 1:  // 5 pixel plus
				for (auto i = -2; i <= 2; i++) {
					deviceContext->SetPixel(pnt.x + i, pnt.y, HotColor);
					deviceContext->SetPixel(pnt.x, pnt.y + i, HotColor);
				}
				break;
			case 2: // 9 pixel plus
				for (auto i = -4; i <= 4; i++) {
					deviceContext->SetPixel(pnt.x + i, pnt.y, HotColor);
					deviceContext->SetPixel(pnt.x, pnt.y + i, HotColor);
				}
				break;
			case 3: // 9 pixel cross
				for (auto i = -4; i <= 4; i++) {
					deviceContext->SetPixel(pnt.x + i, pnt.y - i, HotColor);
					deviceContext->SetPixel(pnt.x + i, pnt.y + i, HotColor);
				}
				break;
			default: // 5 pixel square
				for (auto Row = -2; Row <= 2; Row++) {
					for (auto Col = -2; Col <= 2; Col++) {
						if (abs(Row) == 2 || abs(Col) == 2) {
							deviceContext->SetPixel(pnt.x + Col, pnt.y + Row, HotColor);
						}
					}
				}
		}
	}
}

void EoDbPoint::FormatExtra(CString& extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	CString Mode;
	Mode.Format(L"Point Display Mode;%d\t", m_PointDisplayMode);
	extra += Mode;
}

void EoDbPoint::FormatGeometry(CString& geometry) const {
	CString PositionString;
	PositionString.Format(L"Position;%f;%f;%f\t", m_Position.x, m_Position.y, m_Position.z);
	geometry += PositionString;
}

void EoDbPoint::GetAllPoints(OdGePoint3dArray& points) const {
	points.clear();
	points.append(m_Position);
}

OdGePoint3d EoDbPoint::GetCtrlPt() const noexcept {
	return m_Position;
}

void EoDbPoint::GetExtents(AeSysView* /*view*/, OdGeExtents3d& extents) const {
	extents.addPoint(m_Position);
}

OdGePoint3d EoDbPoint::GoToNxtCtrlPt() const noexcept {
	return m_Position;
}

bool EoDbPoint::IsEqualTo(EoDbPrimitive* primitive) const {
	return m_Position == dynamic_cast<EoDbPoint*>(primitive)->m_Position;
}

bool EoDbPoint::IsInView(AeSysView* view) const {
	EoGePoint4d pt(m_Position, 1.0);
	view->ModelViewTransformPoint(pt);
	return pt.IsInView();
}

bool EoDbPoint::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	EoGePoint4d pt(m_Position, 1.0);
	view->ModelViewTransformPoint(pt);
	return point.DistanceToPointXY(pt) < ms_SelectApertureSize;
}

OdGePoint3d EoDbPoint::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	EoGePoint4d pt(m_Position, 1.0);
	view->ModelViewTransformPoint(pt);
	ms_ControlPointIndex = point.DistanceToPointXY(pt) < ms_SelectApertureSize ? 0 : SIZE_T_MAX;
	return ms_ControlPointIndex == 0 ? m_Position : OdGePoint3d::kOrigin;
}

bool EoDbPoint::SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const {
	EoGePoint4d pt(m_Position, 1.0);
	view->ModelViewTransformPoint(pt);
	projectedPoint = pt.Convert3d();
	return point.DistanceToPointXY(pt) <= view->SelectApertureSize();
}

bool EoDbPoint::SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	EoGePoint4d pt(m_Position, 1.0);
	view->ModelViewTransformPoint(pt);
	return pt.x >= lowerLeftCorner.x && pt.x <= upperRightCorner.x && pt.y >= lowerLeftCorner.y && pt.y <= upperRightCorner.y;
}

double EoDbPoint::DataAt(const unsigned short dataIndex) const noexcept {
	return m_Data[dataIndex];
}

OdGePoint3d EoDbPoint::Position() const noexcept {
	return m_Position;
}

short EoDbPoint::PointDisplayMode() const noexcept {
	return m_PointDisplayMode;
}

void EoDbPoint::ModifyState() noexcept {
	EoDbPrimitive::ModifyState();
	m_PointDisplayMode = g_PrimitiveState.PointDisplayMode();
}

bool EoDbPoint::SelectUsingLineSeg(const EoGeLineSeg3d& /*lineSeg*/, AeSysView* /*view*/, OdGePoint3dArray& /*intersections*/) {
	const CRuntimeClass* PrimitiveClass = GetRuntimeClass();
	theApp.AddStringToMessageList(L"Selection by line segment not implemented for <%s>\n", CString(PrimitiveClass->m_lpszClassName));
	return false;
}

void EoDbPoint::SetData(const unsigned short numberOfDatums, double* data) {
	if (m_NumberOfDatums != numberOfDatums) {
		if (m_NumberOfDatums != 0) { delete[] m_Data; }
		m_NumberOfDatums = numberOfDatums;
		m_Data = m_NumberOfDatums == 0 ? nullptr : new double[m_NumberOfDatums];
	}
	for (unsigned DatumIndex = 0; DatumIndex < m_NumberOfDatums; DatumIndex++) {
		m_Data[DatumIndex] = data[DatumIndex];
	}
}

void EoDbPoint::SetPointDisplayMode(const short displayMode) noexcept {
	m_PointDisplayMode = displayMode;
}

void EoDbPoint::TransformBy(const EoGeMatrix3d& transformMatrix) {
	m_Position.transformBy(transformMatrix);
}

void EoDbPoint::TranslateUsingMask(const OdGeVector3d& translate, const unsigned mask) {
	if (mask != 0) { m_Position += translate; }
}

bool EoDbPoint::Write(EoDbFile& file) const {
	file.WriteUInt16(EoDb::kPointPrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_PointDisplayMode);
	file.WritePoint3d(m_Position);
	file.WriteUInt16(m_NumberOfDatums);
	for (unsigned w = 0; w < m_NumberOfDatums; w++) {
		file.WriteDouble(m_Data[w]);
	}
	return true;
}

void EoDbPoint::Write(CFile& file, unsigned char* buffer) const {
	buffer[3] = 1;
	*reinterpret_cast<unsigned short*>(& buffer[4]) = static_cast<unsigned short>(EoDb::kPointPrimitive);
	buffer[6] = static_cast<unsigned char>(m_ColorIndex == mc_ColorindexBylayer ? ms_LayerColorIndex : m_ColorIndex);
	buffer[7] = static_cast<unsigned char>(m_PointDisplayMode);
	reinterpret_cast<EoVaxPoint3d*>(& buffer[8])->Convert(m_Position);
	::ZeroMemory(&buffer[20], 12);
	auto i {20};
	for (unsigned w = 0; w < m_NumberOfDatums; w++) {
		reinterpret_cast<EoVaxFloat*>(& buffer[i])->Convert(m_Data[w]);
		i += sizeof(EoVaxFloat);
	}
	file.Write(buffer, 32);
}

OdDbPointPtr EoDbPoint::Create(OdDbBlockTableRecordPtr& blockTableRecord) {
	auto Point = OdDbPoint::createObject();
	Point->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Point);
	Point->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));

	// The point object does not store the appearance and size of a point.
	// The database stores the appearance and size of all points in the PDMODE and PDSIZE system variables.
	return Point;
}

EoDbPoint* EoDbPoint::Create(OdDbPointPtr& point) {
	auto Point {new EoDbPoint()};
	Point->m_EntityObjectId = point->objectId();
	Point->m_ColorIndex = static_cast<short>(point->colorIndex());
	Point->m_Position = point->position();
	Point->SetPointDisplayMode(g_PrimitiveState.PointDisplayMode());
	unsigned short NumberOfDatums {0};
	double Data[] {0.0, 0.0, 0.};
	auto ResourceBuffer = point->xData(L"AeSys");
	if (!ResourceBuffer.isNull()) {
		auto Name {ResourceBuffer->getString()};
		ResourceBuffer = ResourceBuffer->next();
		while (!ResourceBuffer.isNull()) {
			Data[NumberOfDatums++] = ResourceBuffer->getDouble(); // not testing for more than 3 datums. boom
			ResourceBuffer = ResourceBuffer->next();
		}
	}
	Point->SetData(NumberOfDatums, Data);
	return Point;
}

OdDbPointPtr EoDbPoint::Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file) {
	auto Point {OdDbPoint::createObject()};
	Point->setDatabaseDefaults(blockTableRecord->database());
	blockTableRecord->appendOdDbEntity(Point);
	Point->setColorIndex(static_cast<unsigned short>(file.ReadInt16()));
	const auto PointDisplayMode {file.ReadInt16()};
	Point->setPosition(file.ReadPoint3d());
	const auto NumberOfDatums {file.ReadUInt16()};
	if (NumberOfDatums > 0) {
		auto ResourceBuffer {OdResBuf::newRb(OdResBuf::kDxfRegAppName, L"AeSys")};
		for (unsigned n = 0; n < NumberOfDatums; n++) {
			const auto Datum {file.ReadDouble()};
			ResourceBuffer->last()->setNext(OdResBuf::newRb(OdResBuf::kDxfXdReal, Datum));
		}
		Point->setXData(ResourceBuffer);
	}
	return Point;
}

OdDbPointPtr EoDbPoint::Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, const int versionNumber) {
	short ColorIndex;
	short PointDisplayMode;
	OdGePoint3d Position;
	if (versionNumber == 1) {
		ColorIndex = static_cast<short>(primitiveBuffer[4] & 0x000fU);
		PointDisplayMode = static_cast<short>((primitiveBuffer[4] & 0x00ffU) >> 4);
		Position = reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[8])->Convert() * 1.e-3;
	} else {
		ColorIndex = static_cast<short>(primitiveBuffer[6]);
		PointDisplayMode = static_cast<short>(primitiveBuffer[7]);
		Position = reinterpret_cast<EoVaxPoint3d*>(& primitiveBuffer[8])->Convert();
	}
	double Data[3] {0.0, 0.0, 0.0};
	Data[0] = reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[20])->Convert();
	Data[1] = reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[24])->Convert();
	Data[2] = reinterpret_cast<EoVaxFloat*>(& primitiveBuffer[28])->Convert();
	const auto Database {blockTableRecord->database()};
	auto Point {OdDbPoint::createObject()};
	Point->setDatabaseDefaults(Database);
	blockTableRecord->appendOdDbEntity(Point);
	Point->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Point->setPosition(Position);
	auto ResourceBuffer {OdResBuf::newRb(OdResBuf::kDxfRegAppName, L"AeSys")};
	for (auto Datum : Data) {
		ResourceBuffer->last()->setNext(OdResBuf::newRb(OdResBuf::kDxfXdReal, Datum));
	}
	Point->setXData(ResourceBuffer);
	return Point;
}
