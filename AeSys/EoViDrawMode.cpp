#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "Ge/GeCircArc3d.h"

#include "EoDlgBlockInsert.h"

#include "EoDbHatch.h"
#include "EoDbSpline.h"
#include "DbGroup.h"
#include "DbAudit.h"
#include "EoDbEntityToPrimitiveProtocolExtension.h"
#include "EoDbPolyline.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

unsigned short PreviousDrawCommand = 0;

void AeSysView::OnDrawModeOptions() {
	AeSysDoc::GetDoc()->OnSetupOptionsDraw();
}

void AeSysView::OnDrawModePoint() {
	const auto CurrentPnt {GetCursorPosition()};
	OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
	auto Point {EoDbPoint::Create(BlockTableRecord)};

	Point->setPosition(CurrentPnt);

	auto Group {new EoDbGroup};
	Group->AddTail(EoDbPoint::Create(Point));
	GetDocument()->AddWorkLayerGroup(Group);
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
}

void AeSysView::OnDrawModeLine() {
	auto CurrentPnt {GetCursorPosition()};

	if (PreviousDrawCommand != ID_OP2) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP2);
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
	} else {
		CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);
		OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
		auto Line {EoDbLine::Create(BlockTableRecord)};
		Line->setStartPoint(m_DrawModePoints[0]);
		Line->setEndPoint(CurrentPnt);

		auto Group {new EoDbGroup};
		Group->AddTail(EoDbLine::Create(Line));
		GetDocument()->AddWorkLayerGroup(Group);

		m_DrawModePoints[0] = CurrentPnt;
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
}

void AeSysView::OnDrawModePolygon() {
	auto CurrentPnt {GetCursorPosition()};

	if (PreviousDrawCommand != ID_OP3) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP3);
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
	} else {
		const int NumberOfPoints = m_DrawModePoints.size();

		if (m_DrawModePoints[NumberOfPoints - 1] != CurrentPnt) {
			CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
			m_DrawModePoints.append(CurrentPnt);
		}
	}
}

void AeSysView::OnDrawModeQuad() {
	const auto CurrentPnt {GetCursorPosition()};

	if (PreviousDrawCommand != ID_OP4) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP4);
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
	} else {
		OnDrawModeReturn();
	}
}

void AeSysView::OnDrawModeArc() {
	const auto CurrentPnt {GetCursorPosition()};

	if (PreviousDrawCommand != ID_OP5) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP5);
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
	} else {
		OnDrawModeReturn();
	}
}

void AeSysView::OnDrawModeBspline() {
	const auto CurrentPnt {GetCursorPosition()};

	if (PreviousDrawCommand != ID_OP6) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP6);

		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
	} else {
		if (!m_DrawModePoints[m_DrawModePoints.size() - 1].isEqualTo(CurrentPnt)) {
			m_DrawModePoints.append(CurrentPnt);
		}
	}
}
void AeSysView::OnDrawModeCircle() {
	const auto CurrentPnt {GetCursorPosition()};

	if (PreviousDrawCommand != ID_OP7) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP7);
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	} else {
		OnDrawModeReturn();
	}
}

void AeSysView::OnDrawModeEllipse() {
	const auto CurrentPnt {GetCursorPosition()};

	if (PreviousDrawCommand != ID_OP8) {
		PreviousDrawCommand = ModeLineHighlightOp(ID_OP8);
		m_DrawModePoints.clear();
		m_DrawModePoints.append(CurrentPnt);
	} else {
		OnDrawModeReturn();
	}
}

void AeSysView::OnDrawModeInsert() {
	auto Document {GetDocument()};

	if (Document->BlockTableSize() > 0) {

		EoDlgBlockInsert Dialog(Document);
		Dialog.DoModal();
	}
}

void AeSysView::OnDrawModeReturn() {
	auto CurrentPnt {GetCursorPosition()};

	const int NumberOfPoints = m_DrawModePoints.size();
	EoDbGroup* Group {nullptr};

	OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	switch (PreviousDrawCommand) {
		case ID_OP2:
		{
			CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);
			auto Line {EoDbLine::Create(BlockTableRecord)};
			Line->setStartPoint(m_DrawModePoints[0]);
			Line->setEndPoint(CurrentPnt);

			Group = new EoDbGroup;
			Group->AddTail(EoDbLine::Create(Line));
			break;
		}
		case ID_OP3:
		{
			if (NumberOfPoints == 1) { return; }

			if (m_DrawModePoints[NumberOfPoints - 1] == CurrentPnt) {
				theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
				return;
			}
			CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
			m_DrawModePoints.append(CurrentPnt);

			auto Hatch {EoDbHatch::Create(BlockTableRecord)};

			const auto PlaneNormal {ComputeNormal(m_DrawModePoints[1], m_DrawModePoints[0], m_DrawModePoints[2])};

			Hatch->setNormal(PlaneNormal);
			Hatch->setElevation(ComputeElevation(m_DrawModePoints[0], PlaneNormal));

			EoDbHatch::AppendLoop(m_DrawModePoints, Hatch);

			Group = new EoDbGroup;
			Group->AddTail(EoDbHatch::Create(Hatch));
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
			break;
		}
		case ID_OP4:
		{
			if (m_DrawModePoints[NumberOfPoints - 1] == CurrentPnt) {
				theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
				return;
			}
			CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
			m_DrawModePoints.append(CurrentPnt);

			if (NumberOfPoints == 1) { return; }

			m_DrawModePoints.append(m_DrawModePoints[0] + OdGeVector3d(m_DrawModePoints[2] - m_DrawModePoints[1]));

			auto GroupPair {EoDbGroup::Create(Database())};
			Group = get<tGroup>(GroupPair);

			for (int i = 0; i < 4; i++) {
				auto Line {EoDbLine::Create(BlockTableRecord)};
				Line->setStartPoint(m_DrawModePoints[i]);
				Line->setEndPoint(m_DrawModePoints[(i + 1) % 4]);

				get<1>(GroupPair)->append(Line->objectId());

				Group->AddTail(EoDbLine::Create(Line));
			}
			break;
		}
		case ID_OP5:
		{
			if (m_DrawModePoints[NumberOfPoints - 1] == CurrentPnt) {
				theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
				return;
			}
			m_DrawModePoints.append(CurrentPnt);

			if (NumberOfPoints == 1) { return; }

			auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
			OdGeCircArc3d CircularArc(m_DrawModePoints[0], m_DrawModePoints[1], m_DrawModePoints[2]);
			Ellipse->set(CircularArc.center(), CircularArc.normal(), CircularArc.refVec() * CircularArc.radius(), 1.0, 0.0, CircularArc.endAng());

			Group = new EoDbGroup;
			Group->AddTail({EoDbEllipse::Create(Ellipse)});
			break;
		}
		case ID_OP6:
		{
			if (!m_DrawModePoints[m_DrawModePoints.size() - 1].isEqualTo(CurrentPnt)) {
				m_DrawModePoints.append(CurrentPnt);
			}
			const int NumberOfControlPoints = m_DrawModePoints.size();

			auto Spline {EoDbSpline::Create(BlockTableRecord)};

			const int Degree = EoMin(3, NumberOfControlPoints - 1);

			OdGeKnotVector Knots;
			EoGeNurbCurve3d::SetDefaultKnotVector(Degree, m_DrawModePoints, Knots);
			OdGeDoubleArray Weights;
			Weights.setLogicalLength(NumberOfControlPoints);

			Spline->setNurbsData(Degree, false, false, false, m_DrawModePoints, Knots, Weights, OdGeContext::gTol.equalPoint());

			Group = new EoDbGroup;
			Group->AddTail(EoDbSpline::Create(Spline));
			break;
		}
		case ID_OP7:
		{
			if (m_DrawModePoints[NumberOfPoints - 1] == CurrentPnt) {
				theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
				return;
			}
			Group = new EoDbGroup;

			const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};

			auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
			auto MajorAxis {ComputeArbitraryAxis(ActiveViewPlaneNormal)};
			MajorAxis.normalize();
			MajorAxis *= OdGeVector3d(CurrentPnt - m_DrawModePoints[0]).length();

			Ellipse->set(m_DrawModePoints[0], ActiveViewPlaneNormal, MajorAxis, 1.0);
			Group->AddTail(EoDbEllipse::Create(Ellipse));

			break;
		}
		case ID_OP8:
		{
			if (CurrentPnt.isEqualTo(m_DrawModePoints[0])) {
				theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
				return;
			}
			if (NumberOfPoints == 1) {
				m_DrawModePoints.append(CurrentPnt);
				SetCursorPosition(m_DrawModePoints[0]);
				return;
			}
			if (CurrentPnt.isEqualTo(m_DrawModePoints[1])) {
				theApp.AddStringToMessageList(IDS_MSG_PTS_COINCIDE);
				return;
			}
			const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};

			auto MajorAxis {m_DrawModePoints[1] - m_DrawModePoints[0]};
			const auto MinorAxis {CurrentPnt - m_DrawModePoints[0]};
			auto RadiusRatio {MinorAxis.length() / MajorAxis.length()};

			if (OdGreater(RadiusRatio, 1.0)) { // Minor axis is longer than major axis - switch
				MajorAxis = MinorAxis;
				RadiusRatio = 1. / RadiusRatio;
			}
			Group = new EoDbGroup;

			auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
			Ellipse->set(m_DrawModePoints[0], ActiveViewPlaneNormal, MajorAxis, RadiusRatio);

			Group->AddTail(EoDbEllipse::Create(Ellipse));
			break;
		}
		default:
			return;
	}
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	GetDocument()->AddWorkLayerGroup(Group);
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);

	m_DrawModePoints.clear();
	ModeLineUnhighlightOp(PreviousDrawCommand);
}

void AeSysView::OnDrawModeEscape() {
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);

	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	m_DrawModePoints.clear();
	ModeLineUnhighlightOp(PreviousDrawCommand);
}

void AeSysView::DoDrawModeMouseMove() {
	auto CurrentPnt {GetCursorPosition()};
	OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

	const int NumberOfPoints = m_DrawModePoints.size();

	switch (PreviousDrawCommand) {
		case ID_OP2:
			if (m_DrawModePoints[0] != CurrentPnt) {
				CurrentPnt = SnapPointToAxis(m_DrawModePoints[0], CurrentPnt);
				m_DrawModePoints.append(CurrentPnt);

				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
				m_PreviewGroup.DeletePrimitivesAndRemoveAll();

				auto Line {EoDbLine::Create(BlockTableRecord, m_DrawModePoints[0], CurrentPnt)};
				Line->setColorIndex(pstate.ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
				m_PreviewGroup.AddTail(EoDbLine::Create(Line));

				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			}
			break;

		case ID_OP3:
		{
			CurrentPnt = SnapPointToAxis(m_DrawModePoints[NumberOfPoints - 1], CurrentPnt);
			m_DrawModePoints.append(CurrentPnt);

			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			m_PreviewGroup.DeletePrimitivesAndRemoveAll();

			if (NumberOfPoints == 1) {
				auto Line {EoDbLine::Create(BlockTableRecord)};
				Line->setStartPoint(m_DrawModePoints[0]);
				Line->setEndPoint(CurrentPnt);
				m_PreviewGroup.AddTail(EoDbLine::Create(Line));
			} else {
				// <tas="This works for plane normal as long as input does not take current point off the view plane. Once input is off the view plane need to use three points for normal and ensure the inputs are planar."/>
				const auto PlaneNormal {CameraDirection()};

				auto Polyline {EoDbPolyline::Create(BlockTableRecord)};

				OdGeMatrix3d WorldToPlaneTransform;
				WorldToPlaneTransform.setToWorldToPlane(OdGePlane(OdGePoint3d::kOrigin, PlaneNormal));

				for (unsigned VertexIndex = 0; VertexIndex < m_DrawModePoints.size(); VertexIndex++) {
					auto Vertex {m_DrawModePoints[VertexIndex]};
					Vertex.transformBy(WorldToPlaneTransform);
					Polyline->addVertexAt(VertexIndex, Vertex.convert2d());
				}
				Polyline->setClosed(true);

				Polyline->setNormal(PlaneNormal);
				Polyline->setElevation(ComputeElevation(m_DrawModePoints[0], PlaneNormal));

				m_PreviewGroup.AddTail(EoDbPolyline::Create(Polyline));
			}
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			break;
		}
		case ID_OP4:
		{
			if (m_DrawModePoints.last() != CurrentPnt) {
				CurrentPnt = SnapPointToAxis(m_DrawModePoints.last(), CurrentPnt);
				m_DrawModePoints.append(CurrentPnt);

				if (NumberOfPoints == 2) {
					m_DrawModePoints.append(m_DrawModePoints[0] + OdGeVector3d(CurrentPnt - m_DrawModePoints[1]));
					m_DrawModePoints.append(m_DrawModePoints[0]);
				}
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
				m_PreviewGroup.DeletePrimitivesAndRemoveAll();

				for (unsigned PointsIndex = 0; PointsIndex < m_DrawModePoints.size() - 1; PointsIndex++) {
					const auto StartPoint {m_DrawModePoints[PointsIndex]};
					const auto EndPoint {m_DrawModePoints[(PointsIndex + 1) % 4]};
					auto Line {EoDbLine::Create(BlockTableRecord, StartPoint, EndPoint)};
					Line->setColorIndex(pstate.ColorIndex());
					Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
					m_PreviewGroup.AddTail(EoDbLine::Create(Line));
				}
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			}
			break;
		}
		case ID_OP5:
			m_DrawModePoints.append(CurrentPnt);

			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			m_PreviewGroup.DeletePrimitivesAndRemoveAll();

			if (NumberOfPoints == 1) {
				auto Line {EoDbLine::Create(BlockTableRecord, m_DrawModePoints[0], CurrentPnt)};
				Line->setColorIndex(pstate.ColorIndex());
				Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
				m_PreviewGroup.AddTail(EoDbLine::Create(Line));
			}
			if (NumberOfPoints == 2) {
				auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
				OdGeCircArc3d CircularArc(m_DrawModePoints[0], m_DrawModePoints[1], m_DrawModePoints[2]);
				Ellipse->set(CircularArc.center(), CircularArc.normal(), CircularArc.refVec() * CircularArc.radius(), 1.0, 0.0, CircularArc.endAng());
				m_PreviewGroup.AddTail(EoDbEllipse::Create(Ellipse));
			}
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			break;

		case ID_OP6:
			if (!m_DrawModePoints[m_DrawModePoints.size() - 1].isEqualTo(CurrentPnt)) {
				m_DrawModePoints.append(CurrentPnt);

				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);

				m_PreviewGroup.DeletePrimitivesAndRemoveAll();
				const int NumberOfControlPoints = m_DrawModePoints.size();
				const int Degree = EoMin(3, NumberOfControlPoints - 1);
				OdGePoint3dArray Points;
				for (int ControlPointIndex = 0; ControlPointIndex < NumberOfControlPoints; ControlPointIndex++) {
					Points.append(m_DrawModePoints[ControlPointIndex]);
				}
				OdGeKnotVector Knots;
				EoGeNurbCurve3d::SetDefaultKnotVector(Degree, Points, Knots);
				OdGeDoubleArray Weights;
				Weights.setLogicalLength(NumberOfControlPoints);
				auto Spline {new EoDbSpline()};
				Spline->Set(Degree, Knots, Points, Weights);
				m_PreviewGroup.AddTail(Spline);
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			}
			break;
		case ID_OP7:
			if (m_DrawModePoints[0] != CurrentPnt) {
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);

				m_PreviewGroup.DeletePrimitivesAndRemoveAll();

				const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};
				auto MajorAxis {ComputeArbitraryAxis(ActiveViewPlaneNormal)};
				const auto Radius {(CurrentPnt - m_DrawModePoints[0]).length()};
				MajorAxis = MajorAxis.normalize() * Radius;

				auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};

				Ellipse->set(m_DrawModePoints[0], ActiveViewPlaneNormal, MajorAxis, 1.0);

				m_PreviewGroup.AddTail(EoDbEllipse::Create(Ellipse));
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			}
			break;

		case ID_OP8:
			if (!CurrentPnt.isEqualTo(m_DrawModePoints[0])) {
				m_DrawModePoints.append(CurrentPnt);

				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
				m_PreviewGroup.DeletePrimitivesAndRemoveAll();
				if (NumberOfPoints == 1) {
					auto Line {EoDbLine::Create(BlockTableRecord)};
					Line->setStartPoint(m_DrawModePoints[0]);
					Line->setEndPoint(CurrentPnt);

					m_PreviewGroup.AddTail(EoDbLine::Create(Line));
				} else {
					const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};
					auto MajorAxis {m_DrawModePoints[1] - m_DrawModePoints[0]};
					const auto MinorAxis {CurrentPnt - m_DrawModePoints[0]};
					auto RadiusRatio {MinorAxis.length() / MajorAxis.length()};

					if (OdGreater(RadiusRatio, 1.0)) {
						MajorAxis = MinorAxis;
						RadiusRatio = 1. / RadiusRatio;
					}
					auto Ellipse {EoDbEllipse::Create(BlockTableRecord)};
					Ellipse->set(m_DrawModePoints[0], ActiveViewPlaneNormal, MajorAxis, RadiusRatio);

					m_PreviewGroup.AddTail(EoDbEllipse::Create(Ellipse));
				}
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
			}
			break;

	}
	m_DrawModePoints.setLogicalLength(NumberOfPoints);
}
