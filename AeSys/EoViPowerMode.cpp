#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

void AeSysView::OnPowerModeOptions() noexcept {
	// TODO: Add your command handler code here
}

void AeSysView::OnPowerModeCircuit() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	m_PowerArrow = false;
	m_PowerConductor = false;

	m_PreviewGroup.DeletePrimitivesAndRemoveAll();

	EoDbEllipse* SymbolCircle;
	EoDbGroup* Group = SelectCircleUsingPoint(CurrentPnt, .02, SymbolCircle);
	if (Group != 0) {
		CurrentPnt = SymbolCircle->Center();
		const double CurrentRadius = SymbolCircle->MajorAxis().length();

		if (m_PowerModePoints.empty()) {
			m_PowerModePoints.append(CurrentPnt);
			m_PreviousOp = ModeLineHighlightOp(ID_OP2);
		}
		else {
			Group = new EoDbGroup;
			GetDocument()->AddWorkLayerGroup(Group);
			const OdGePoint3d pt1 = ProjectToward(m_PowerModePoints[0], CurrentPnt, m_PreviousRadius);
			const OdGePoint3d pt2 = ProjectToward(CurrentPnt, m_PowerModePoints[0], CurrentRadius);
			EoDbLine* Line = EoDbLine::Create(Database());
			Line->SetTo(pt1, pt2);
			Group->AddTail(Line);
			m_PowerModePoints[0] = CurrentPnt;
		}
		m_PreviousRadius = CurrentRadius;
	}
	else {
		if (m_PowerModePoints.empty()) {
			m_PowerModePoints.append(CurrentPnt);
		}
		else {
			CurrentPnt = SnapPointToAxis(m_PowerModePoints[0], CurrentPnt);

			Group = new EoDbGroup;
			GetDocument()->AddWorkLayerGroup(Group);
			const OdGePoint3d pt1 = ProjectToward(m_PowerModePoints[0], CurrentPnt, m_PreviousRadius);
			const OdGePoint3d pt2 = ProjectToward(CurrentPnt, m_PowerModePoints[0], 0.);
			EoDbLine* Line = EoDbLine::Create(Database());
			Line->SetTo(pt1, pt2);
			Group->AddTail(Line);

			m_PowerModePoints[0] = CurrentPnt;
		}
		m_PreviousOp = ModeLineHighlightOp(ID_OP2);
		m_PreviousRadius = 0.;
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

	OdGePoint3d CurrentPnt = GetCursorPosition();

	m_PowerConductor = false;
	m_PreviousOp = 0;

	GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();

	if (!m_PowerArrow || (PointOnCircuit != CurrentPnt)) {
		m_PowerArrow = false;
		EoDbLine* Circuit;
		EoDbGroup* Group = SelectLineBy(CurrentPnt, Circuit);
		if (Group != 0) {
			CurrentPnt = Circuit->ProjPt_(CurrentPnt);
			if (Circuit->ParametricRelationshipOf(CurrentPnt) <= .5) {
				m_CircuitEndPoint = Circuit->EndPoint();
				if (CurrentPnt.distanceTo(Circuit->StartPoint()) <= .1)
					CurrentPnt = Circuit->StartPoint();
			}
			else {
				m_CircuitEndPoint = Circuit->StartPoint();
				if (CurrentPnt.distanceTo(Circuit->EndPoint()) <= .1)
					CurrentPnt = Circuit->EndPoint();
			}
			m_PowerArrow = CurrentPnt.distanceTo(m_CircuitEndPoint) > m_PowerConductorSpacing;
			GenerateHomeRunArrow(CurrentPnt, m_CircuitEndPoint);
			CurrentPnt = ProjectToward(CurrentPnt, m_CircuitEndPoint, m_PowerConductorSpacing);
			SetCursorPosition(CurrentPnt);
		}
	}
	else {
		m_PowerArrow = CurrentPnt.distanceTo(m_CircuitEndPoint) > m_PowerConductorSpacing;
		GenerateHomeRunArrow(CurrentPnt, m_CircuitEndPoint);
		CurrentPnt = ProjectToward(CurrentPnt, m_CircuitEndPoint, m_PowerConductorSpacing);
		SetCursorPosition(CurrentPnt);
	}
	PointOnCircuit = CurrentPnt;
}

void AeSysView::DoPowerModeMouseMove() {
	OdGePoint3d CurrentPnt = GetCursorPosition();
	const int NumberOfPoints = m_PowerModePoints.size();

	switch (m_PreviousOp) {
	case ID_OP2:
		if (m_PowerModePoints[0] != CurrentPnt) {
			GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
			m_PreviewGroup.DeletePrimitivesAndRemoveAll();

			EoDbEllipse* SymbolCircle;
			EoDbGroup* Group = SelectCircleUsingPoint(CurrentPnt, .02, SymbolCircle);
			if (Group != 0) {
				const double CurrentRadius = SymbolCircle->MajorAxis().length();
				CurrentPnt = SymbolCircle->Center();
				CurrentPnt = ProjectToward(CurrentPnt, m_PowerModePoints[0], CurrentRadius);
			}
			else {
				CurrentPnt = SnapPointToAxis(m_PowerModePoints[0], CurrentPnt);
			}
			const OdGePoint3d pt1 = ProjectToward(m_PowerModePoints[0], CurrentPnt, m_PreviousRadius);
			m_PreviewGroup.AddTail(new EoDbLine(pt1, CurrentPnt));
			GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		}
		break;
	}
	m_PowerModePoints.setLogicalLength(NumberOfPoints);
}
void AeSysView::DoPowerModeConductor(OdUInt16 conductorType) {
	static OdGePoint3d PointOnCircuit;

	OdGePoint3d CurrentPnt = GetCursorPosition();

	m_PowerArrow = false;
	m_PreviousOp = 0;

	GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();

	if (!m_PowerConductor || PointOnCircuit != CurrentPnt) {
		m_PowerConductor = false;
		EoDbLine* Circuit;
		EoDbGroup* Group = SelectLineBy(CurrentPnt, Circuit);
		if (Group != 0) {
			CurrentPnt = Circuit->ProjPt_(CurrentPnt);

			const OdGePoint3d BeginPoint = Circuit->StartPoint();
			m_CircuitEndPoint = Circuit->EndPoint();

			if (fabs(m_CircuitEndPoint.x - BeginPoint.x) > .025) {
				if (BeginPoint.x > m_CircuitEndPoint.x)
					m_CircuitEndPoint = BeginPoint;
			}
			else if (BeginPoint.y > m_CircuitEndPoint.y)
				m_CircuitEndPoint = BeginPoint;

			GeneratePowerConductorSymbol(conductorType, CurrentPnt, m_CircuitEndPoint);
			CurrentPnt = ProjectToward(CurrentPnt, m_CircuitEndPoint, m_PowerConductorSpacing);
			SetCursorPosition(CurrentPnt);
			m_PowerConductor = CurrentPnt.distanceTo(m_CircuitEndPoint) >  m_PowerConductorSpacing;
		}
	}
	else {
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

	ModeLineUnhighlightOp(m_PreviousOp);	m_PreviousOp = 0;

	GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
}

void AeSysView::GenerateHomeRunArrow(const OdGePoint3d& pointOnCircuit, const OdGePoint3d& endPoint) {
    const auto PlaneNormal {CameraDirection()};

    OdGePoint3dArray Points;
	Points.setLogicalLength(3);

	Points[0] = ProjectToward(pointOnCircuit, endPoint, .05);

	EoGeLineSeg3d Circuit(Points[0], endPoint);

	Circuit.ProjPtFrom_xy(0., - .075, Points[0]);
	Points[1] = pointOnCircuit;
	Circuit.ProjPtFrom_xy(0., .075, Points[2]);

    auto Group {new EoDbGroup};
	GetDocument()->AddWorkLayerGroup(Group);

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    auto Polyline {EoDbPolyline::Create(BlockTableRecord)};

    Polyline->setColorIndex(2);
    Polyline->setLinetype(L"Continuous");

    OdGeMatrix3d WorldToPlaneTransform;
    WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, PlaneNormal));

    for (size_t VertexIndex = 0; VertexIndex < Points.size(); VertexIndex++) {
        auto Vertex = Points[VertexIndex];
        Vertex.transformBy(WorldToPlaneTransform);
        Polyline->addVertexAt(VertexIndex, Vertex.convert2d());
    }
    Polyline->setNormal(PlaneNormal);
    Polyline->setElevation(ComputeElevation(endPoint, PlaneNormal));
    Group->AddTail(EoDbPolyline::Create(Polyline));

	GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
}
void AeSysView::GeneratePowerConductorSymbol(OdUInt16 conductorType, const OdGePoint3d& pointOnCircuit, const OdGePoint3d& endPoint) {
	const OdGeVector3d ActiveViewPlaneNormal = GetActiveView()->CameraDirection();

	OdGePoint3d Points[5];
	EoGeLineSeg3d Circuit(pointOnCircuit, endPoint);
	EoDbEllipse* Circle;
	EoDbLine* Line;
	EoDbGroup* Group = new EoDbGroup;

	switch (conductorType) {
	case ID_OP4:
		Circuit.ProjPtFrom_xy(0., - .1, Points[0]);
		Circuit.ProjPtFrom_xy(0., .075, Points[1]);
		Circuit.ProjPtFrom_xy(0., .0875, Points[2]);
		Line = EoDbLine::Create(Database());
		Line->SetTo(Points[0], Points[1]);
		Line->SetColorIndex(1);
		Line->SetLinetypeIndex(1);
		Group->AddTail(Line);
		Circle = EoDbEllipse::Create(Database());
		Circle->SetToCircle(Points[2], ActiveViewPlaneNormal, .0125);
		Circle->SetColorIndex(1);
		Circle->SetLinetypeIndex(1);
		Group->AddTail(Circle);
		break;

	case ID_OP5:
		Circuit.ProjPtFrom_xy(0., - .1, Points[0]);
		Circuit.ProjPtFrom_xy(0., .1, Points[1]);
		Line = EoDbLine::Create(Database());
		Line->SetTo(Points[0], Points[1]);
		Line->SetColorIndex(1);
		Line->SetLinetypeIndex(1);
		Group->AddTail(Line);
		break;

	case ID_OP6:
		Circuit.ProjPtFrom_xy(0., - .1, Points[0]);
		Circuit.ProjPtFrom_xy(0., .05, Points[1]);

		Points[2] = ProjectToward(pointOnCircuit, endPoint, .025);

		EoGeLineSeg3d(Points[2], endPoint).ProjPtFrom_xy(0., .075, Points[3]);
		EoGeLineSeg3d(pointOnCircuit, endPoint).ProjPtFrom_xy(0., .1, Points[4]);
		
		Line = EoDbLine::Create(Database());
		Line->SetTo(Points[0], Points[1]);
		Line->SetColorIndex(1);
		Line->SetLinetypeIndex(1);
		Group->AddTail(Line);
		Line = EoDbLine::Create(Database());
		Line->SetTo(Points[1], Points[3]);
		Line->SetColorIndex(1);
		Line->SetLinetypeIndex(1);
		Group->AddTail(Line);
		Line = EoDbLine::Create(Database());
		Line->SetTo(Points[3], Points[4]);
		Line->SetColorIndex(1);
		Line->SetLinetypeIndex(1);
		Group->AddTail(Line);
		break;

	case ID_OP7:
		Circuit.ProjPtFrom_xy(0., - .05, Points[0]);
		Circuit.ProjPtFrom_xy(0., .05, Points[1]);
		Line = EoDbLine::Create(Database());
		Line->SetTo(Points[0], Points[1]);
		Line->SetColorIndex(1);
		Line->SetLinetypeIndex(1);
		Group->AddTail(Line);
		break;

	default:
		delete Group;
		return;
	}
	GetDocument()->AddWorkLayerGroup(Group);
	GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
}
