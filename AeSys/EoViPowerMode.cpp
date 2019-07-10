#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoDbPolyline.h"

void AeSysView::OnPowerModeOptions() noexcept {
	// TODO: Add your command handler code here
}

void AeSysView::OnPowerModeCircuit() {
	auto CurrentPnt {GetCursorPosition()};
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	const auto ColorIndex {g_PrimitiveState.ColorIndex()};
	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	m_PowerArrow = false;
	m_PowerConductor = false;
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	auto Selection {SelectCircleUsingPoint(CurrentPnt, .02)};
	auto Group {std::get<tGroup>(Selection)};
	if (Group != nullptr) {
		const auto SymbolCircle {std::get<1>(Selection)};
		CurrentPnt = SymbolCircle->Center();
		const auto CurrentRadius {SymbolCircle->MajorAxis().length()};
		if (m_PowerModePoints.empty()) {
			m_PowerModePoints.append(CurrentPnt);
			m_PreviousOp = ModeLineHighlightOp(ID_OP2);
		} else {
			Group = new EoDbGroup;
			GetDocument()->AddWorkLayerGroup(Group);
			const auto pt1 {ProjectToward(m_PowerModePoints[0], CurrentPnt, m_PreviousRadius)};
			const auto pt2 {ProjectToward(CurrentPnt, m_PowerModePoints[0], CurrentRadius)};
			auto Line {EoDbLine::Create(BlockTableRecord, pt1, pt2)};
			Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
			Line->setLinetype(Linetype);
			Group->AddTail(EoDbLine::Create(Line));
			m_PowerModePoints[0] = CurrentPnt;
		}
		m_PreviousRadius = CurrentRadius;
	} else {
		if (m_PowerModePoints.empty()) {
			m_PowerModePoints.append(CurrentPnt);
		} else {
			CurrentPnt = SnapPointToAxis(m_PowerModePoints[0], CurrentPnt);
			Group = new EoDbGroup;
			GetDocument()->AddWorkLayerGroup(Group);
			const auto pt1 {ProjectToward(m_PowerModePoints[0], CurrentPnt, m_PreviousRadius)};
			const auto pt2 {ProjectToward(CurrentPnt, m_PowerModePoints[0], 0.0)};
			auto Line {EoDbLine::Create(BlockTableRecord, pt1, pt2)};
			Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
			Line->setLinetype(Linetype);
			Group->AddTail(EoDbLine::Create(Line));
			m_PowerModePoints[0] = CurrentPnt;
		}
		m_PreviousOp = ModeLineHighlightOp(ID_OP2);
		m_PreviousRadius = 0.0;
	}
}

void AeSysView::OnPowerModeGround() {
	DoPowerModeConductor(ID_OP4);
}

void AeSysView::OnPowerModeHot() {
	DoPowerModeConductor(ID_OP5);
}

void AeSysView::OnPowerModeSwitch() {
	DoPowerModeConductor(ID_OP6);
}

void AeSysView::OnPowerModeNeutral() {
	DoPowerModeConductor(ID_OP7);
}

void AeSysView::OnPowerModeHome() {
	static OdGePoint3d PointOnCircuit;
	auto CurrentPnt {GetCursorPosition()};
	m_PowerConductor = false;
	m_PreviousOp = 0;
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	if (!m_PowerArrow || PointOnCircuit != CurrentPnt) {
		m_PowerArrow = false;
		auto Selection {SelectLineUsingPoint(CurrentPnt)};
		const auto Group {std::get<tGroup>(Selection)};
		if (Group != nullptr) {
			const auto Circuit {std::get<1>(Selection)};
			CurrentPnt = Circuit->ProjPt_(CurrentPnt);
			if (Circuit->ParametricRelationshipOf(CurrentPnt) <= 0.5) {
				m_CircuitEndPoint = Circuit->EndPoint();
				if (CurrentPnt.distanceTo(Circuit->StartPoint()) <= 0.1) { CurrentPnt = Circuit->StartPoint(); }
			} else {
				m_CircuitEndPoint = Circuit->StartPoint();
				if (CurrentPnt.distanceTo(Circuit->EndPoint()) <= 0.1) { CurrentPnt = Circuit->EndPoint(); }
			}
			m_PowerArrow = CurrentPnt.distanceTo(m_CircuitEndPoint) > m_PowerConductorSpacing;
			GenerateHomeRunArrow(CurrentPnt, m_CircuitEndPoint);
			CurrentPnt = ProjectToward(CurrentPnt, m_CircuitEndPoint, m_PowerConductorSpacing);
			SetCursorPosition(CurrentPnt);
		}
	} else {
		m_PowerArrow = CurrentPnt.distanceTo(m_CircuitEndPoint) > m_PowerConductorSpacing;
		GenerateHomeRunArrow(CurrentPnt, m_CircuitEndPoint);
		CurrentPnt = ProjectToward(CurrentPnt, m_CircuitEndPoint, m_PowerConductorSpacing);
		SetCursorPosition(CurrentPnt);
	}
	PointOnCircuit = CurrentPnt;
}

void AeSysView::DoPowerModeMouseMove() {
	auto CurrentPnt {GetCursorPosition()};
	const auto NumberOfPoints {m_PowerModePoints.size()};
	switch (m_PreviousOp) {
		case ID_OP2:
			if (m_PowerModePoints[0] != CurrentPnt) {
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
				m_PreviewGroup.DeletePrimitivesAndRemoveAll();
				auto Selection {SelectCircleUsingPoint(CurrentPnt, .02)};
				const auto Group {std::get<tGroup>(Selection)};
				if (Group != nullptr) {
					const auto SymbolCircle {std::get<1>(Selection)};
					const auto CurrentRadius {SymbolCircle->MajorAxis().length()};
					CurrentPnt = SymbolCircle->Center();
					CurrentPnt = ProjectToward(CurrentPnt, m_PowerModePoints[0], CurrentRadius);
				} else {
					CurrentPnt = SnapPointToAxis(m_PowerModePoints[0], CurrentPnt);
				}
				const auto pt1 {ProjectToward(m_PowerModePoints[0], CurrentPnt, m_PreviousRadius)};
				const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
				auto Line {EoDbLine::Create(BlockTableRecord, pt1, CurrentPnt)};
				Line->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex()));
				m_PreviewGroup.AddTail(EoDbLine::Create(Line));
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			}
			break;
		default: ;
	}
	m_PowerModePoints.setLogicalLength(NumberOfPoints);
}

void AeSysView::DoPowerModeConductor(const unsigned short conductorType) {
	static OdGePoint3d PointOnCircuit;
	auto CurrentPnt {GetCursorPosition()};
	m_PowerArrow = false;
	m_PreviousOp = 0;
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	if (!m_PowerConductor || PointOnCircuit != CurrentPnt) {
		m_PowerConductor = false;
		auto Selection {SelectLineUsingPoint(CurrentPnt)};
		const auto Group {std::get<tGroup>(Selection)};
		if (Group != nullptr) {
			const auto Circuit {std::get<1>(Selection)};
			CurrentPnt = Circuit->ProjPt_(CurrentPnt);
			const auto BeginPoint {Circuit->StartPoint()};
			m_CircuitEndPoint = Circuit->EndPoint();
			if (fabs(m_CircuitEndPoint.x - BeginPoint.x) > 0.025) {
				if (BeginPoint.x > m_CircuitEndPoint.x) { m_CircuitEndPoint = BeginPoint; }
			} else if (BeginPoint.y > m_CircuitEndPoint.y) { m_CircuitEndPoint = BeginPoint; }
			GeneratePowerConductorSymbol(conductorType, CurrentPnt, m_CircuitEndPoint);
			CurrentPnt = ProjectToward(CurrentPnt, m_CircuitEndPoint, m_PowerConductorSpacing);
			SetCursorPosition(CurrentPnt);
			m_PowerConductor = CurrentPnt.distanceTo(m_CircuitEndPoint) > m_PowerConductorSpacing;
		}
	} else {
		GeneratePowerConductorSymbol(conductorType, CurrentPnt, m_CircuitEndPoint);
		CurrentPnt = ProjectToward(CurrentPnt, m_CircuitEndPoint, m_PowerConductorSpacing);
		SetCursorPosition(CurrentPnt);
		m_PowerConductor = CurrentPnt.distanceTo(m_CircuitEndPoint) > m_PowerConductorSpacing;
	}
	PointOnCircuit = CurrentPnt;
}

void AeSysView::OnPowerModeReturn() {
	OnPowerModeEscape();
}

void AeSysView::OnPowerModeEscape() {
	m_PowerArrow = false;
	m_PowerConductor = false;
	m_PowerModePoints.clear();
	ModeLineUnhighlightOp(m_PreviousOp);
	m_PreviousOp = 0;
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
}

void AeSysView::GenerateHomeRunArrow(const OdGePoint3d& pointOnCircuit, const OdGePoint3d& endPoint) const {
	const auto PlaneNormal {CameraDirection()};
	OdGePoint3dArray Points;
	Points.setLogicalLength(3);
	Points[0] = ProjectToward(pointOnCircuit, endPoint, .05);
	EoGeLineSeg3d Circuit(Points[0], endPoint);
	Circuit.ProjPtFrom_xy(0.0, -.075, Points[0]);
	Points[1] = pointOnCircuit;
	Circuit.ProjPtFrom_xy(0.0, .075, Points[2]);
	auto Group {new EoDbGroup};
	GetDocument()->AddWorkLayerGroup(Group);
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	auto Polyline {EoDbPolyline::Create(BlockTableRecord)};
	Polyline->setColorIndex(2);
	Polyline->setLinetype(L"Continuous");
	OdGeMatrix3d WorldToPlaneTransform;
	WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, PlaneNormal));
	for (unsigned VertexIndex = 0; VertexIndex < Points.size(); VertexIndex++) {
		auto Vertex = Points[VertexIndex];
		Vertex.transformBy(WorldToPlaneTransform);
		Polyline->addVertexAt(VertexIndex, Vertex.convert2d());
	}
	Polyline->setNormal(PlaneNormal);
	Polyline->setElevation(ComputeElevation(endPoint, PlaneNormal));
	Group->AddTail(EoDbPolyline::Create(Polyline));
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
}

void AeSysView::GeneratePowerConductorSymbol(const unsigned short conductorType, const OdGePoint3d& pointOnCircuit, const OdGePoint3d& endPoint) const {
	const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};
	OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
	OdGePoint3d Points[5];
	EoGeLineSeg3d Circuit(pointOnCircuit, endPoint);
	OdDbLinePtr Line;
	auto Group {new EoDbGroup};
	switch (conductorType) {
		case ID_OP4: {
			Circuit.ProjPtFrom_xy(0.0, -.1, Points[0]);
			Circuit.ProjPtFrom_xy(0.0, .075, Points[1]);
			Circuit.ProjPtFrom_xy(0.0, .0875, Points[2]);
			Line = EoDbLine::Create(BlockTableRecord, Points[0], Points[1]);
			Line->setColorIndex(1);
			Line->setLinetype(L"Continuous");
			Group->AddTail(EoDbLine::Create(Line));
			auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, Points[2], ActiveViewPlaneNormal, .0125)};
			Circle->setColorIndex(1);
			Circle->setLinetype(L"Continuous");
			Group->AddTail(EoDbEllipse::Create(Circle));
			break;
		}
		case ID_OP5:
			Circuit.ProjPtFrom_xy(0.0, -.1, Points[0]);
			Circuit.ProjPtFrom_xy(0.0, .1, Points[1]);
			Line = EoDbLine::Create(BlockTableRecord, Points[0], Points[1]);
			Line->setColorIndex(1);
			Line->setLinetype(L"Continuous");
			Group->AddTail(EoDbLine::Create(Line));
			break;
		case ID_OP6:
			Circuit.ProjPtFrom_xy(0.0, -.1, Points[0]);
			Circuit.ProjPtFrom_xy(0.0, .05, Points[1]);
			Points[2] = ProjectToward(pointOnCircuit, endPoint, .025);
			EoGeLineSeg3d(Points[2], endPoint).ProjPtFrom_xy(0.0, .075, Points[3]);
			EoGeLineSeg3d(pointOnCircuit, endPoint).ProjPtFrom_xy(0.0, .1, Points[4]);
			Line = EoDbLine::Create(BlockTableRecord, Points[0], Points[1]);
			Line->setColorIndex(1);
			Line->setLinetype(L"Continuous");
			Group->AddTail(EoDbLine::Create(Line));
			Line = EoDbLine::Create(BlockTableRecord, Points[1], Points[3]);
			Line->setColorIndex(1);
			Line->setLinetype(L"Continuous");
			Group->AddTail(EoDbLine::Create(Line));
			Line = EoDbLine::Create(BlockTableRecord, Points[3], Points[4]);
			Line->setColorIndex(1);
			Line->setLinetype(L"Continuous");
			Group->AddTail(EoDbLine::Create(Line));
			break;
		case ID_OP7:
			Circuit.ProjPtFrom_xy(0.0, -.05, Points[0]);
			Circuit.ProjPtFrom_xy(0.0, .05, Points[1]);
			Line = EoDbLine::Create(BlockTableRecord, Points[0], Points[1]);
			Line->setColorIndex(1);
			Line->setLinetype(L"Continuous");
			Group->AddTail(EoDbLine::Create(Line));
			break;
		default:
			delete Group;
			return;
	}
	GetDocument()->AddWorkLayerGroup(Group);
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
}
