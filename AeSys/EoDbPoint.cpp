#include "stdafx.h"

#include "DbBlockTableRecord.h"

#include "AeSys.h"
#include "AeSysView.h"

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
	m_Data = (m_NumberOfDatums == 0) ? nullptr : new double[m_NumberOfDatums];

	for (unsigned n = 0; n < m_NumberOfDatums; n++) {
		m_Data[n] = other.m_Data[n];
	}
}

EoDbPoint::~EoDbPoint() {
	if (m_NumberOfDatums != 0) { delete[] m_Data; }
}

const EoDbPoint& EoDbPoint::operator=(const EoDbPoint& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;

	m_ColorIndex = other.m_ColorIndex;
	m_PointDisplayMode = other.m_PointDisplayMode;
	m_Position = other.m_Position;
	
	if (m_NumberOfDatums != other.m_NumberOfDatums) {
		
		if (m_NumberOfDatums != 0) { delete[] m_Data; }

		m_NumberOfDatums = other.m_NumberOfDatums;

		m_Data = (m_NumberOfDatums == 0) ? nullptr : new double[m_NumberOfDatums];
	}
	for (unsigned n = 0; n < m_NumberOfDatums; n++) {
		m_Data[n] = other.m_Data[n];
	}
	return (*this);
}

void EoDbPoint::AddReportToMessageList(const OdGePoint3d & point) const {
	CString Report(L"<Point>");
	Report += L" Color:" + FormatColorIndex();
	CString Mode;
	Mode.Format(L" Point Display Mode:%d", m_PointDisplayMode);
	Report += Mode;
	theApp.AddStringToMessageList(Report);
}

void EoDbPoint::AddToTreeViewControl(HWND tree, HTREEITEM parent) const noexcept {
	CMainFrame::InsertTreeViewControlItem(tree, parent, L"<Point>", this);
}

EoDbPrimitive* EoDbPoint::Clone(OdDbBlockTableRecordPtr blockTableRecord) const {
	OdDbPointPtr Point = m_EntityObjectId.safeOpenObject()->clone();
	blockTableRecord->appendOdDbEntity(Point);

	return EoDbPoint::Create(Point);
}

void EoDbPoint::Display(AeSysView* view, CDC* deviceContext) {
	const short ColorIndex = LogicalColorIndex();

	const COLORREF HotColor = theApp.GetHotColor(ColorIndex);

	EoGePoint4d pt(m_Position, 1.0);
	view->ModelViewTransformPoint(pt);

	if (pt.IsInView()) {
		const CPoint pnt = view->DoViewportProjection(pt);

		int i {0};
		switch (m_PointDisplayMode) {
			case 0:	// 3 pixel plus
				for (i = -1; i <= 1; i++) {
					deviceContext->SetPixel(pnt.x + i, pnt.y, HotColor);
					deviceContext->SetPixel(pnt.x, pnt.y + i, HotColor);
				}
				break;

			case 1:  // 5 pixel plus
				for (i = -2; i <= 2; i++) {
					deviceContext->SetPixel(pnt.x + i, pnt.y, HotColor);
					deviceContext->SetPixel(pnt.x, pnt.y + i, HotColor);
				}
				break;

			case 2: // 9 pixel plus
				for (i = -4; i <= 4; i++) {
					deviceContext->SetPixel(pnt.x + i, pnt.y, HotColor);
					deviceContext->SetPixel(pnt.x, pnt.y + i, HotColor);
				}
				break;

			case 3: // 9 pixel cross
				for (i = -4; i <= 4; i++) {
					deviceContext->SetPixel(pnt.x + i, pnt.y - i, HotColor);
					deviceContext->SetPixel(pnt.x + i, pnt.y + i, HotColor);
				}
				break;

			default: // 5 pixel square
				for (int Row = -2; Row <= 2; Row++) {
					for (int Col = -2; Col <= 2; Col++) {
						if (abs(Row) == 2 || abs(Col) == 2) {
							deviceContext->SetPixel(pnt.x + Col, pnt.y + Row, HotColor);
						}
					}
				}

		}
	}
}

void EoDbPoint::FormatExtra(CString & extra) const {
	extra.Empty();
	extra += L"Color;" + FormatColorIndex() + L"\t";
	CString Mode;
	Mode.Format(L"Point Display Mode;%d", m_PointDisplayMode);
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
	return (m_Position);
}

void EoDbPoint::GetExtents(AeSysView* view, OdGeExtents3d & extents) const {
	extents.addPoint(m_Position);
}

OdGePoint3d EoDbPoint::GoToNxtCtrlPt() const noexcept {
	return (m_Position);
}

bool EoDbPoint::IsEqualTo(EoDbPrimitive* primitive) const {
	return m_Position == dynamic_cast<EoDbPoint*>(primitive)->m_Position;
}

bool EoDbPoint::IsInView(AeSysView* view) const {
	EoGePoint4d pt(m_Position, 1.0);

	view->ModelViewTransformPoint(pt);

	return (pt.IsInView());
}

bool EoDbPoint::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	EoGePoint4d pt(m_Position, 1.0);
	view->ModelViewTransformPoint(pt);

	return ((point.DistanceToPointXY(pt) < sm_SelectApertureSize) ? true : false);
}

OdGePoint3d EoDbPoint::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	EoGePoint4d pt(m_Position, 1.0);
	view->ModelViewTransformPoint(pt);

	sm_ControlPointIndex = (point.DistanceToPointXY(pt) < sm_SelectApertureSize) ? 0 : SIZE_T_MAX;
	return (sm_ControlPointIndex == 0) ? m_Position : OdGePoint3d::kOrigin;
}

bool EoDbPoint::SelectUsingPoint(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& projectedPoint) const {
	EoGePoint4d pt(m_Position, 1.0);

	view->ModelViewTransformPoint(pt);

	projectedPoint = pt.Convert3d();

	return (point.DistanceToPointXY(pt) <= view->SelectApertureSize()) ? true : false;
}

bool EoDbPoint::SelectUsingRectangle(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	EoGePoint4d pt(m_Position, 1.0);
	view->ModelViewTransformPoint(pt);

	return ((pt.x >= lowerLeftCorner.x && pt.x <= upperRightCorner.x && pt.y >= lowerLeftCorner.y && pt.y <= upperRightCorner.y) ? true : false);
}

double EoDbPoint::DataAt(unsigned short dataIndex) const noexcept {
	return (m_Data[dataIndex]);
}

OdGePoint3d EoDbPoint::Position() const noexcept {
	return (m_Position);
}

short EoDbPoint::PointDisplayMode() const noexcept {
	return (m_PointDisplayMode);
}

void EoDbPoint::ModifyState() noexcept {
	EoDbPrimitive::ModifyState();
	m_PointDisplayMode = pstate.PointDisplayMode();
}

bool EoDbPoint::SelectUsingLineSeg(const EoGeLineSeg3d& lineSeg, AeSysView* view, OdGePoint3dArray& intersections) {
	const CRuntimeClass* PrimitiveClass = GetRuntimeClass();
	theApp.AddStringToMessageList(L"Selection by line segment not implemented for <%s>\n", CString(PrimitiveClass->m_lpszClassName));
	return false;
}

void EoDbPoint::SetData(unsigned short numberOfDatums, double* data) {
	if (m_NumberOfDatums != numberOfDatums) {

		if (m_NumberOfDatums != 0) { delete[] m_Data; }

		m_NumberOfDatums = numberOfDatums;
		m_Data = (m_NumberOfDatums == 0) ? nullptr : new double[m_NumberOfDatums];
	}
	for (unsigned w = 0; w < m_NumberOfDatums; w++) {
		m_Data[w] = data[w];
	}
}

void EoDbPoint::SetPointDisplayMode(short displayMode) noexcept {
	m_PointDisplayMode = displayMode;
}

void EoDbPoint::TransformBy(const EoGeMatrix3d & transformMatrix) {
	m_Position.transformBy(transformMatrix);
}

void EoDbPoint::TranslateUsingMask(const OdGeVector3d & translate, const unsigned long mask) {

	if (mask != 0) { m_Position += translate; }
}

bool EoDbPoint::Write(EoDbFile & file) const {
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
	*((unsigned short*) & buffer[4]) = static_cast<unsigned short>(EoDb::kPointPrimitive);
	buffer[6] = static_cast<unsigned char>(m_ColorIndex == COLORINDEX_BYLAYER ? sm_LayerColorIndex : m_ColorIndex);
	buffer[7] = static_cast<unsigned char>(m_PointDisplayMode);

	((EoVaxPoint3d*) & buffer[8])->Convert(m_Position);

	::ZeroMemory(&buffer[20], 12);

	int i = 20;

	for (unsigned w = 0; w < m_NumberOfDatums; w++) {
		((EoVaxFloat*) & buffer[i])->Convert(m_Data[w]);
		i += sizeof(EoVaxFloat);
	}
	file.Write(buffer, 32);
}

OdDbPointPtr EoDbPoint::Create(OdDbBlockTableRecordPtr & blockTableRecord) {
	OdDbPointPtr Point = OdDbPoint::createObject();
	Point->setDatabaseDefaults(blockTableRecord->database());

	blockTableRecord->appendOdDbEntity(Point);
	Point->setColorIndex(static_cast<unsigned short>(pstate.ColorIndex()));

	// The point object does not store the appearance and size of a point.
	// The database stores the appearance and size of all points in the PDMODE and PDSIZE system variables.

	return Point;
}

EoDbPoint* EoDbPoint::Create(OdDbPointPtr & point) {
	auto Point {new EoDbPoint()};
	Point->m_EntityObjectId = point->objectId();
	Point->m_ColorIndex = static_cast<short>(point->colorIndex());
	Point->m_Position = point->position();

	Point->SetPointDisplayMode(pstate.PointDisplayMode());

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

	const short PointDisplayMode = file.ReadInt16();

	Point->setPosition(file.ReadPoint3d());

	const auto NumberOfDatums {file.ReadUInt16()};
	
	if (NumberOfDatums > 0) {
		auto ResourceBuffer {OdResBuf::newRb(OdResBuf::kDxfRegAppName, L"AeSys")};

		double Data;
		for (unsigned n = 0; n < NumberOfDatums; n++) {
			Data = file.ReadDouble();
			ResourceBuffer->last()->setNext(OdResBuf::newRb(OdResBuf::kDxfXdReal, Data));
		}
		Point->setXData(ResourceBuffer);
	}
	return Point;
}

OdDbPointPtr EoDbPoint::Create(OdDbBlockTableRecordPtr blockTableRecord, unsigned char* primitiveBuffer, int versionNumber) {
	short ColorIndex {0};
	short PointDisplayMode {0};
	OdGePoint3d Position;

	if (versionNumber == 1) {
		ColorIndex = static_cast<short>(primitiveBuffer[4] & 0x000f);
		PointDisplayMode = static_cast<short>((primitiveBuffer[4] & 0x00ff) >> 4);
		Position = ((EoVaxPoint3d*) & primitiveBuffer[8])->Convert() * 1.e-3;
	} else {
		ColorIndex = static_cast<short>(primitiveBuffer[6]);
		PointDisplayMode = static_cast<short>(primitiveBuffer[7]);
		Position = ((EoVaxPoint3d*) & primitiveBuffer[8])->Convert();
	}
	double Data[3] {0.0, 0.0, 0.};
	Data[0] = ((EoVaxFloat*) & primitiveBuffer[20])->Convert();
	Data[1] = ((EoVaxFloat*) & primitiveBuffer[24])->Convert();
	Data[2] = ((EoVaxFloat*) & primitiveBuffer[28])->Convert();

	auto Database {blockTableRecord->database()};

	auto Point {OdDbPoint::createObject()};
	Point->setDatabaseDefaults(Database);

	blockTableRecord->appendOdDbEntity(Point);

	Point->setColorIndex(static_cast<unsigned short>(ColorIndex));

	Point->setPosition(Position);

	auto ResourceBuffer {OdResBuf::newRb(OdResBuf::kDxfRegAppName, L"AeSys")};

	for (unsigned n = 0; n < 3; n++) {
		ResourceBuffer->last()->setNext(OdResBuf::newRb(OdResBuf::kDxfXdReal, Data[n]));
	}
	Point->setXData(ResourceBuffer);
	return (Point);
}
