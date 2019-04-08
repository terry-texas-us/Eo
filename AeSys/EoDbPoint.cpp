#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysView.h"

EoDbPoint::EoDbPoint() noexcept
    : m_Position {OdGePoint3d::kOrigin}
    , m_NumberOfDatums {0}
    , m_Data {nullptr} {
	m_ColorIndex = 1;
	m_PointDisplayMode = 1;
}

EoDbPoint::EoDbPoint(const OdGePoint3d& point)
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

	for (OdUInt16 n = 0; n < m_NumberOfDatums; n++) {
		m_Data[n] = other.m_Data[n];
	}
}

EoDbPoint::~EoDbPoint() {
	if (m_NumberOfDatums != 0)
		delete [] m_Data;
}

const EoDbPoint& EoDbPoint::operator=(const EoDbPoint& other) {
	m_LayerId = other.m_LayerId;
	m_EntityObjectId = other.m_EntityObjectId;

	m_ColorIndex = other.m_ColorIndex;
	m_PointDisplayMode = other.m_PointDisplayMode;
	m_Position = other.m_Position;
	if (m_NumberOfDatums != other.m_NumberOfDatums) {
		if (m_NumberOfDatums != 0)
			delete [] m_Data;

		m_NumberOfDatums = other.m_NumberOfDatums;

		m_Data = (m_NumberOfDatums == 0) ? nullptr : new double[m_NumberOfDatums];
	}
	for (OdUInt16 n = 0; n < m_NumberOfDatums; n++) {
		m_Data[n] = other.m_Data[n];
	}
	return (*this);
}

void EoDbPoint::AddReportToMessageList(const OdGePoint3d& point) const {
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

void EoDbPoint::AssociateWith(OdDbBlockTableRecordPtr& blockTableRecord) {
	OdDbPointPtr PointEntity = OdDbPoint::createObject();
	blockTableRecord->appendOdDbEntity(PointEntity);
	PointEntity->setDatabaseDefaults();
	
	SetEntityObjectId(PointEntity->objectId());
	
	PointEntity->setColorIndex(m_ColorIndex);
	PointEntity->setPosition(m_Position);
	// <tas="Data values for points?"</tas>
}

EoDbPrimitive* EoDbPoint::Clone(OdDbDatabasePtr& database) const {
	return (EoDbPoint::Create(*this, database));
}

void EoDbPoint::Display(AeSysView* view, CDC* deviceContext) {
	const OdInt16 ColorIndex = LogicalColorIndex();

	const COLORREF HotColor = theApp.GetHotColor(ColorIndex);

	EoGePoint4d pt(m_Position, 1.);
	view->ModelViewTransformPoint(pt);

	if (pt.IsInView()) {
		const CPoint pnt = view->DoViewportProjection(pt);

        int i {0};
		switch (m_PointDisplayMode) {
		case 0:	// 3 pixel plus
			for (i = - 1; i <= 1; i++) {
				deviceContext->SetPixel(pnt.x + i , pnt.y, HotColor);
				deviceContext->SetPixel(pnt.x , pnt.y + i, HotColor);
			}
			break;

		case 1:  // 5 pixel plus
			for (i = - 2; i <= 2; i++) {
				deviceContext->SetPixel(pnt.x + i , pnt.y, HotColor);
				deviceContext->SetPixel(pnt.x , pnt.y + i, HotColor);
			}
			break;

		case 2: // 9 pixel plus
			for (i = - 4; i <= 4; i++) {
				deviceContext->SetPixel(pnt.x + i , pnt.y, HotColor);
				deviceContext->SetPixel(pnt.x , pnt.y + i, HotColor);
			}
			break;

		case 3: // 9 pixel cross
			for (i = - 4; i <= 4; i++) {
				deviceContext->SetPixel(pnt.x + i , pnt.y - i, HotColor);
				deviceContext->SetPixel(pnt.x + i, pnt.y + i, HotColor);
			}
			break;

		default: // 5 pixel square
			for (int Row = - 2; Row <= 2; Row++) {
				for (int Col = - 2; Col <= 2; Col++) {
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
	Mode.Format(L"Point Display Mode;%d",  m_PointDisplayMode);
	extra += Mode;
}

void EoDbPoint::FormatGeometry(CString& geometry) const {
	CString PositionString;
	PositionString.Format(L"Position;%f;%f;%f\t", m_Position.x, m_Position.y, m_Position.z);
	geometry +=  PositionString;
}

void EoDbPoint::GetAllPoints(OdGePoint3dArray& points) const {
	points.clear(); 
	points.append(m_Position);
}

OdGePoint3d EoDbPoint::GetCtrlPt() const noexcept {
	return (m_Position);
}

void EoDbPoint::GetExtents(AeSysView* view, OdGeExtents3d& extents) const {
	extents.addPoint(m_Position);
}

OdGePoint3d EoDbPoint::GoToNxtCtrlPt() const noexcept {
	return (m_Position);
}

bool EoDbPoint::IsEqualTo(EoDbPrimitive* primitive) const {
	return m_Position == static_cast<EoDbPoint*>(primitive)->m_Position;
}

bool EoDbPoint::IsInView(AeSysView* view) const {
	EoGePoint4d pt(m_Position, 1.);

	view->ModelViewTransformPoint(pt);

	return (pt.IsInView());
}

bool EoDbPoint::IsPointOnControlPoint(AeSysView* view, const EoGePoint4d& point) const {
    EoGePoint4d pt(m_Position, 1.);
    view->ModelViewTransformPoint(pt);

    return ((point.DistanceToPointXY(pt) < sm_SelectApertureSize) ? true : false);
}

OdGePoint3d EoDbPoint::SelectAtControlPoint(AeSysView* view, const EoGePoint4d& point) const {
	EoGePoint4d pt(m_Position, 1.);
	view->ModelViewTransformPoint(pt);

	sm_ControlPointIndex = (point.DistanceToPointXY(pt) < sm_SelectApertureSize) ? 0 : SIZE_T_MAX;
	return (sm_ControlPointIndex == 0) ? m_Position : OdGePoint3d::kOrigin;
}

bool EoDbPoint::SelectBy(const EoGePoint4d& point, AeSysView* view, OdGePoint3d& ptProj) const {
	EoGePoint4d pt(m_Position, 1.);

	view->ModelViewTransformPoint(pt);

	ptProj = pt.Convert3d();

	return (point.DistanceToPointXY(pt) <= view->SelectApertureSize()) ? true : false;
}

bool EoDbPoint::SelectBy(const OdGePoint3d& lowerLeftCorner, const OdGePoint3d& upperRightCorner, AeSysView* view) const {
	EoGePoint4d pt(m_Position, 1.);
	view->ModelViewTransformPoint(pt);

	return ((pt.x >= lowerLeftCorner.x && pt.x <= upperRightCorner.x && pt.y >= lowerLeftCorner.y && pt.y <= upperRightCorner.y) ? true : false);
}

double EoDbPoint::DataAt(OdUInt16 dataIndex) const noexcept {
	return (m_Data[dataIndex]);
}

OdGePoint3d EoDbPoint::Position() const noexcept {
	return (m_Position);
}

OdInt16 EoDbPoint::PointDisplayMode() const noexcept {
	return (m_PointDisplayMode);
}

void EoDbPoint::ModifyState() noexcept {
	EoDbPrimitive::ModifyState();
	m_PointDisplayMode = pstate.PointDisplayMode();
}

void EoDbPoint::SetData(OdUInt16 numberOfDatums, double* data) {
	if (m_NumberOfDatums != numberOfDatums) {
		if (m_NumberOfDatums != 0) {
			delete [] m_Data;
		}
		m_NumberOfDatums = numberOfDatums;
		m_Data = (m_NumberOfDatums == 0) ? nullptr : new double[m_NumberOfDatums];
	}
	for (OdUInt16 w = 0; w < m_NumberOfDatums; w++) {
		m_Data[w] = data[w];
	}
}

void EoDbPoint::SetPosition(const OdGePoint3d& position) {
	if (!m_EntityObjectId.isNull()) {
		OdDbPointPtr Point = m_EntityObjectId.safeOpenObject(OdDb::kForWrite);
		Point->setPosition(position);
	}
	m_Position = position;
}

void EoDbPoint::SetPointDisplayMode(OdInt16 displayMode) noexcept {
	m_PointDisplayMode = displayMode;
}

void EoDbPoint::TransformBy(const EoGeMatrix3d& transformMatrix) {
	m_Position.transformBy(transformMatrix);
}

void EoDbPoint::TranslateUsingMask(const OdGeVector3d& translate, const DWORD mask) {
	if (mask != 0)
		m_Position += translate;
}

bool EoDbPoint::Write(EoDbFile& file) const {
	file.WriteUInt16(kPointPrimitive);
	file.WriteInt16(m_ColorIndex);
	file.WriteInt16(m_PointDisplayMode);
	
	file.WritePoint3d(m_Position);

	file.WriteUInt16(m_NumberOfDatums);
	for (OdUInt16 w = 0; w < m_NumberOfDatums; w++)
		file.WriteDouble(m_Data[w]);

	return true;
}

void EoDbPoint::Write(CFile& file, OdUInt8* buffer) const {
	buffer[3] = 1;
	*((OdUInt16*) &buffer[4]) = OdUInt16(kPointPrimitive);
	buffer[6] = OdInt8(m_ColorIndex == COLORINDEX_BYLAYER ? sm_LayerColorIndex : m_ColorIndex);
	buffer[7] = OdInt8(m_PointDisplayMode);

	((EoVaxPoint3d*) &buffer[8])->Convert(m_Position);

	::ZeroMemory(&buffer[20], 12);

	int i = 20;

	for (OdUInt16 w = 0; w < m_NumberOfDatums; w++) {
		((EoVaxFloat*) &buffer[i])->Convert(m_Data[w]);
		i += sizeof(EoVaxFloat);
	}

	file.Write(buffer, 32);
}

EoDbPoint* EoDbPoint::ConstructFrom(OdUInt8* primitiveBuffer, int versionNumber) {
	EoDbPoint* PointPrimitive = 0;
	OdInt16 ColorIndex;
	OdInt16 PointDisplayMode;
	OdGePoint3d Position;

	if (versionNumber == 1) {
		ColorIndex = OdInt16(primitiveBuffer[4] & 0x000f);
		PointDisplayMode = OdInt16((primitiveBuffer[4] & 0x00ff) >> 4);
		Position = ((EoVaxPoint3d*) &primitiveBuffer[8])->Convert() * 1.e-3;
	}
	else {
		ColorIndex = OdInt16(primitiveBuffer[6]);
		PointDisplayMode = OdInt16(primitiveBuffer[7]);
		Position = ((EoVaxPoint3d*) &primitiveBuffer[8])->Convert();
	}
    double Data[3] {0., 0., 0.};
	Data[0] = ((EoVaxFloat*) &primitiveBuffer[20])->Convert();
	Data[1] = ((EoVaxFloat*) &primitiveBuffer[24])->Convert();
	Data[2] = ((EoVaxFloat*) &primitiveBuffer[28])->Convert();

	PointPrimitive = new EoDbPoint(Position);
	PointPrimitive->SetColorIndex(ColorIndex);
	PointPrimitive->SetPointDisplayMode(PointDisplayMode);
	PointPrimitive->SetData(3, Data);
	return (PointPrimitive);
}

EoDbPoint* EoDbPoint::Create(const EoDbPoint& other, OdDbDatabasePtr& database) {
    OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
    OdDbPointPtr PointEntity = other.EntityObjectId().safeOpenObject()->clone();
    BlockTableRecord->appendOdDbEntity(PointEntity);

    EoDbPoint* Point = new EoDbPoint(other);
    Point->SetEntityObjectId(PointEntity->objectId());

    return Point;
}

EoDbPoint* EoDbPoint::Create(OdDbDatabasePtr& database) {
    OdDbBlockTableRecordPtr BlockTableRecord = database->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    OdDbPointPtr PointEntity = OdDbPoint::createObject();
    PointEntity->setDatabaseDefaults(database);
    BlockTableRecord->appendOdDbEntity(PointEntity);

    EoDbPoint* PointPrimitive = new EoDbPoint();
    PointPrimitive->SetEntityObjectId(PointEntity->objectId());

    PointPrimitive->SetColorIndex(pstate.ColorIndex());
    PointPrimitive->SetPointDisplayMode(pstate.PointDisplayMode());

    return PointPrimitive;
}

OdDbPointPtr EoDbPoint::Create(OdDbBlockTableRecordPtr& blockTableRecord) {
	OdDbPointPtr Point = OdDbPoint::createObject();
	Point->setDatabaseDefaults(blockTableRecord->database());

	blockTableRecord->appendOdDbEntity(Point);
	Point->setColorIndex(pstate.ColorIndex());

	// The point object does not store the appearance and size of a point.
	// The database stores the appearance and size of all points in the PDMODE and PDSIZE system variables.

	return Point;
}

OdDbPointPtr EoDbPoint::Create(OdDbBlockTableRecordPtr& blockTableRecord, EoDbFile& file) {
    OdDbPointPtr Point = OdDbPoint::createObject();
    Point->setDatabaseDefaults(blockTableRecord->database());

    blockTableRecord->appendOdDbEntity(Point);

    Point->setColorIndex(file.ReadInt16());

    const OdInt16 PointDisplayMode = file.ReadInt16();

    Point->setPosition(file.ReadPoint3d());

    const OdUInt16 NumberOfDatums = file.ReadUInt16();
    if (NumberOfDatums > 0) {
        double Data[3] {0., 0., 0.};
        for (OdUInt16 n = 0; n < NumberOfDatums; n++) {
            Data[n] = file.ReadDouble();
        }
    }
    return Point;
}

EoDbPoint* EoDbPoint::Create(OdDbPointPtr& point) {
    EoDbPoint* Point = new EoDbPoint();
    Point->SetEntityObjectId(point->objectId());
    Point->SetColorIndex(point->colorIndex());
    Point->SetPosition(point->position());

    Point->SetPointDisplayMode(pstate.PointDisplayMode());

    return Point;
}