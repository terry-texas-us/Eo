#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoDlgLowPressureDuctOptions.h"

std::pair<EoDbGroup*, EoDbPoint*> AeSysView::SelectPointUsingPoint(const OdGePoint3d& point, const double tolerance, const short pointColor) {
	auto GroupPosition {GetFirstVisibleGroupPosition()};
	while (GroupPosition != nullptr) {
		auto Group = GetNextVisibleGroup(GroupPosition);
		auto PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbPoint)) != 0 && Primitive->ColorIndex() == pointColor) {
				const auto Point {dynamic_cast<EoDbPoint*>(Primitive)};
				if (point.distanceTo(Point->Position()) <= tolerance) {
					return {Group, Point};
				}
			}
		}
	}
	return {nullptr, nullptr};
}

/// <remarks>
///Only check for actual end-cap marker is by attributes. No error processing for invalid width or depth values.
///Group data contains whatever primitive follows marker (hopefully this is associated end-cap line).
///Issues:
/// xor operations on transition not clean
/// ending section with 3 key will generate a shortened section if the point is less than transition length from the begin point.
/// full el only works with center just
/// </remarks>
void AeSysView::OnLpdModeOptions() {
	SetDuctOptions(m_CurrentSection);
}

void AeSysView::OnLpdModeJoin() {
	const auto CurrentPnt {GetCursorPosition()};
	auto Selection {SelectPointUsingPoint(CurrentPnt, 0.01, 15)};
	m_EndCapGroup = std::get<tGroup>(Selection);
	if (m_EndCapGroup != nullptr) {
		m_EndCapPoint = std::get<1>(Selection);
		m_PreviousPnt = m_EndCapPoint->Position();
		m_PreviousSection.SetWidth(m_EndCapPoint->DataAt(0));
		m_PreviousSection.SetDepth(m_EndCapPoint->DataAt(1));
		m_ContinueSection = false;
		m_EndCapLocation = m_PreviousOp == 0 ? 1 : -1; // 1 (start) and -1 (end)
		OdString Message(L"Cross sectional dimension (Width by Depth) is ");
		Message += theApp.FormatLength(m_PreviousSection.Width(), max(theApp.GetUnits(), AeSys::kInches), 12, 2);
		Message += L" by ";
		Message += theApp.FormatLength(m_PreviousSection.Depth(), max(theApp.GetUnits(), AeSys::kInches), 12, 2);
		AeSys::AddStringToMessageList(Message);
		SetCursorPosition(m_PreviousPnt);
	}
}

void AeSysView::OnLpdModeDuct() {
	auto CurrentPnt {GetCursorPosition()};
	if (m_PreviousOp != 0) {
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	if (m_PreviousOp == ID_OP2) {
		CurrentPnt = SnapPointToAxis(m_PreviousPnt, CurrentPnt);
		m_CurrentReferenceLine.set(m_PreviousPnt, CurrentPnt);
		if (m_ContinueSection) {
			const auto Group {new EoDbGroup};
			GetDocument()->AddWorkLayerGroup(Group);
			GenerateRectangularElbow(m_PreviousReferenceLine, m_PreviousSection, m_CurrentReferenceLine, m_CurrentSection, Group);
			m_OriginalPreviousGroup->DeletePrimitivesAndRemoveAll();
			GenerateRectangularSection(m_PreviousReferenceLine, m_CenterLineEccentricity, m_PreviousSection, m_OriginalPreviousGroup);
			m_OriginalPreviousGroupDisplayed = true;
			m_PreviousSection = m_CurrentSection;
		}
		const auto TransitionLength {m_PreviousSection == m_CurrentSection ? 0.0 : LengthOfTransition(m_DuctJustification, m_TransitionSlope, m_PreviousSection, m_CurrentSection)};
		auto ReferenceLine {m_CurrentReferenceLine};
		if (m_BeginWithTransition) {
			if (TransitionLength != 0.0) {
				ReferenceLine.SetEndPoint(ReferenceLine.ProjToEndPt(TransitionLength));
				const auto Group {new EoDbGroup};
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateTransition(ReferenceLine, m_CenterLineEccentricity, m_DuctJustification, m_TransitionSlope, m_PreviousSection, m_CurrentSection, Group);
				ReferenceLine.SetStartPoint(ReferenceLine.endPoint());
				ReferenceLine.SetEndPoint(m_CurrentReferenceLine.endPoint());
				m_ContinueSection = false;
			}
			if (m_CurrentReferenceLine.length() - TransitionLength > FLT_EPSILON) {
				m_OriginalPreviousGroup = new EoDbGroup;
				GetDocument()->AddWorkLayerGroup(m_OriginalPreviousGroup);
				GenerateRectangularSection(ReferenceLine, m_CenterLineEccentricity, m_CurrentSection, m_OriginalPreviousGroup);
				m_ContinueSection = true;
			}
		} else {
			if (ReferenceLine.length() - TransitionLength > FLT_EPSILON) {
				ReferenceLine.SetEndPoint(ReferenceLine.ProjToBegPt(TransitionLength));
				m_OriginalPreviousGroup = new EoDbGroup;
				GetDocument()->AddWorkLayerGroup(m_OriginalPreviousGroup);
				GenerateRectangularSection(ReferenceLine, m_CenterLineEccentricity, m_PreviousSection, m_OriginalPreviousGroup);
				ReferenceLine.SetStartPoint(ReferenceLine.endPoint());
				ReferenceLine.SetEndPoint(m_CurrentReferenceLine.endPoint());
				m_ContinueSection = true;
			}
			if (TransitionLength != 0.0) {
				const auto Group {new EoDbGroup};
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateTransition(ReferenceLine, m_CenterLineEccentricity, m_DuctJustification, m_TransitionSlope, m_PreviousSection, m_CurrentSection, Group);
				m_ContinueSection = false;
			}
		}
		m_PreviousReferenceLine = m_CurrentReferenceLine;
		m_PreviousSection = m_CurrentSection;
	}
	m_PreviousOp = ID_OP2;
	m_PreviousPnt = CurrentPnt;
}

void AeSysView::OnLpdModeTransition() {
	m_CurrentSection = m_PreviousSection;
	SetDuctOptions(m_CurrentSection);
	m_BeginWithTransition = m_PreviousOp == 0;
	DoDuctModeMouseMove();
	OnLpdModeDuct();
}

void AeSysView::OnLpdModeTap() {
	auto CurrentPnt {GetCursorPosition()};
	if (m_PreviousOp != 0) {
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	auto Selection {SelectLineUsingPoint(CurrentPnt)};
	auto Group {std::get<tGroup>(Selection)};
	if (Group != nullptr) {
		const auto LinePrimitive {std::get<1>(Selection)};
		const auto TestPoint {CurrentPnt};
		CurrentPnt = SnapPointToAxis(m_PreviousPnt, CurrentPnt);
		CurrentPnt = LinePrimitive->ProjPt_(CurrentPnt);
		m_CurrentReferenceLine.set(m_PreviousPnt, CurrentPnt);
		EJust Justification;
		const auto Relationship {m_CurrentReferenceLine.DirectedRelationshipOf(TestPoint)};
		if (Relationship == 1) {
			Justification = kLeft;
		} else if (Relationship == -1) {
			Justification = kRight;
		} else {
			AeSys::AddStringToMessageList(L"Could not determine orientation of component");
			return;
		}
		if (m_PreviousOp == ID_OP2) {
			if (m_ContinueSection) {
				Group = new EoDbGroup;
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateRectangularElbow(m_PreviousReferenceLine, m_PreviousSection, m_CurrentReferenceLine, m_CurrentSection, Group);
				m_PreviousSection = m_CurrentSection;
			}
			const auto SectionLength {m_CurrentReferenceLine.length()};
			if (SectionLength >= m_DuctTapSize + m_DuctSeamSize) {
				auto ReferenceLine {m_CurrentReferenceLine};
				ReferenceLine.SetEndPoint(ReferenceLine.ProjToBegPt(m_DuctTapSize + m_DuctSeamSize));
				Group = new EoDbGroup;
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateRectangularSection(ReferenceLine, m_CenterLineEccentricity, m_PreviousSection, Group);
				m_CurrentReferenceLine.SetStartPoint(ReferenceLine.endPoint());
				m_PreviousReferenceLine = m_CurrentReferenceLine;
				m_PreviousSection = m_CurrentSection;
			}
			GenerateRectangularTap(Justification, m_PreviousSection);
			m_PreviousOp = 0;
			m_ContinueSection = false;
			m_PreviousPnt = CurrentPnt;
		}
	} else {
		theApp.AddStringToMessageList(IDS_MSG_LINE_NOT_SELECTED);
	}
}

void AeSysView::OnLpdModeEll() {
	auto CurrentPnt {GetCursorPosition()};
	if (m_PreviousOp != 0) {
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	if (m_PreviousOp == ID_OP2) {
		auto Selection {SelectPointUsingPoint(CurrentPnt, 0.01, 15)};
		auto ExistingGroup {std::get<tGroup>(Selection)};
		if (ExistingGroup == nullptr) {
			theApp.AddStringToMessageList(IDS_MSG_LPD_NO_END_CAP_LOC);
			return;
		}
		const auto EndPointPrimitive {std::get<1>(Selection)};
		CurrentPnt = EndPointPrimitive->Position();
		const Section ExistingSection {EndPointPrimitive->DataAt(0), EndPointPrimitive->DataAt(1), Section::mc_Rectangular};
		const auto BeginPointPrimitive {ExistingGroup->GetFirstDifferentPoint(EndPointPrimitive)};
		if (BeginPointPrimitive != nullptr) {
			EoGeLineSeg3d ExistingSectionReferenceLine(BeginPointPrimitive->Position(), CurrentPnt);
			const auto IntersectionPoint(ExistingSectionReferenceLine.ProjPt(m_PreviousPnt));
			double Relationship;
			ExistingSectionReferenceLine.ParametricRelationshipOf(IntersectionPoint, Relationship);
			if (Relationship > FLT_EPSILON) {
				m_CurrentReferenceLine.set(m_PreviousPnt, IntersectionPoint);
				const auto SectionLength {m_CurrentReferenceLine.length() - (m_PreviousSection.Width() + m_DuctSeamSize + ExistingSection.Width() * 0.5)};
				if (SectionLength > FLT_EPSILON) {
					m_CurrentReferenceLine.SetEndPoint(m_CurrentReferenceLine.ProjToEndPt(SectionLength));
					const auto Group {new EoDbGroup};
					GetDocument()->AddWorkLayerGroup(Group);
					GenerateRectangularSection(m_CurrentReferenceLine, m_CenterLineEccentricity, m_PreviousSection, Group);
					GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				}
				const auto Group {new EoDbGroup};
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateFullElbowTakeoff(ExistingGroup, ExistingSectionReferenceLine, ExistingSection, Group);
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
			}
		}
		// determine where cursor should be moved to.
	}
	m_ContinueSection = false;
	m_PreviousOp = ID_OP2;
}

void AeSysView::OnLpdModeTee() {
	if (m_PreviousOp != 0) {
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	// <tas="GenerateBullheadTee not implemented"/>
	m_ContinueSection = false;
	m_PreviousOp = ID_OP2;
}

void AeSysView::OnLpdModeUpDown() {
	auto CurrentPnt {GetCursorPosition()};
	const auto iRet {0}; // dialog to "Select direction", 'Up.Down.'
	if (iRet >= 0) {
		if (m_PreviousOp == ID_OP2) {
			CurrentPnt = SnapPointToAxis(m_PreviousPnt, CurrentPnt);
			m_CurrentReferenceLine.set(m_PreviousPnt, CurrentPnt);
			if (m_ContinueSection) {
				const auto Group {new EoDbGroup};
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateRectangularElbow(m_PreviousReferenceLine, m_PreviousSection, m_CurrentReferenceLine, m_CurrentSection, Group);
				m_PreviousSection = m_CurrentSection;
			}
			const auto SectionLength {m_CurrentReferenceLine.length()};
			if (SectionLength > m_PreviousSection.Depth() * 0.5 + m_DuctSeamSize) {
				auto ReferenceLine {m_CurrentReferenceLine};
				const auto StartPoint {ReferenceLine.startPoint()};
				ReferenceLine.SetEndPoint(ProjectToward(StartPoint, ReferenceLine.endPoint(), SectionLength - m_PreviousSection.Depth() * 0.5 - m_DuctSeamSize));
				const auto Group {new EoDbGroup};
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateRectangularSection(ReferenceLine, m_CenterLineEccentricity, m_PreviousSection, Group);
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
				m_CurrentReferenceLine.SetStartPoint(ReferenceLine.endPoint());
			}
			const auto Group {new EoDbGroup};
			GetDocument()->AddWorkLayerGroup(Group);
			GenerateRiseDrop(1, m_PreviousSection, m_CurrentReferenceLine, Group);
			GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
		}
		m_ContinueSection = false;
		m_PreviousOp = ID_OP2;
		m_PreviousPnt = CurrentPnt;
	}
}

void AeSysView::OnLpdModeSize() {
	const auto CurrentPnt {GetCursorPosition()};
	auto Angle {0.0};
	if (m_EndCapPoint != nullptr) {
		if (m_EndCapPoint->ColorIndex() == 15) {
			auto Position {m_EndCapGroup->Find(m_EndCapPoint)};
			m_EndCapGroup->GetNext(Position);
			const auto pLine {dynamic_cast<EoDbLine*>(m_EndCapGroup->GetAt(Position))};
			const auto Line {pLine->LineSeg()};
			Angle = fmod(Line.AngleFromXAxis_xy(), OdaPI);
			if (Angle <= OdaPI / 180.0) {
				Angle += OdaPI;
			}
			Angle -= OdaPI2;
		}
		m_EndCapPoint = nullptr;
	}
	GenSizeNote(CurrentPnt, Angle, m_PreviousSection);
	if (m_PreviousOp != 0) {
		RubberBandingDisable();
	}
	m_PreviousOp = 0;
	m_ContinueSection = false;
}

void AeSysView::OnLpdModeReturn() {
	const auto CurrentPnt {GetCursorPosition()};
	if (m_PreviousOp != 0) {
		OnLpdModeEscape();
	}
	m_PreviousPnt = CurrentPnt;
}

void AeSysView::OnLpdModeEscape() {
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
	m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	if (!m_OriginalPreviousGroupDisplayed) {
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, m_OriginalPreviousGroup);
		m_OriginalPreviousGroupDisplayed = true;
	}
	ModeLineUnhighlightOp(m_PreviousOp);
	m_PreviousOp = 0;
	m_ContinueSection = false;
	m_EndCapGroup = nullptr;
	m_EndCapPoint = nullptr;
}

void AeSysView::DoDuctModeMouseMove() {
	static auto CurrentPnt {OdGePoint3d()};
	if (m_PreviousOp == 0) {
		CurrentPnt = GetCursorPosition();
		m_OriginalPreviousGroupDisplayed = true;
	} else if (m_PreviousOp == ID_OP2) {
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
		CurrentPnt = GetCursorPosition();
		CurrentPnt = SnapPointToAxis(m_PreviousPnt, CurrentPnt);
		m_CurrentReferenceLine.set(m_PreviousPnt, CurrentPnt);
		if (m_ContinueSection && m_CurrentReferenceLine.length() > m_PreviousSection.Width() * m_CenterLineEccentricity + m_DuctSeamSize) {
			auto PreviousReferenceLine {m_PreviousReferenceLine};
			if (m_OriginalPreviousGroupDisplayed) {
				GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, m_OriginalPreviousGroup);
				m_OriginalPreviousGroupDisplayed = false;
			}
			GenerateRectangularElbow(PreviousReferenceLine, m_PreviousSection, m_CurrentReferenceLine, m_CurrentSection, &m_PreviewGroup, false);
			GenerateRectangularSection(PreviousReferenceLine, m_CenterLineEccentricity, m_PreviousSection, &m_PreviewGroup);
		}
		auto Selection {SelectPointUsingPoint(CurrentPnt, 0.01, 15)};
		auto ExistingGroup {std::get<tGroup>(Selection)};
		if (ExistingGroup != nullptr) {
			const auto EndPointPrimitive {std::get<1>(Selection)};
			CurrentPnt = EndPointPrimitive->Position();
			const Section ExistingSection {EndPointPrimitive->DataAt(0), EndPointPrimitive->DataAt(1), Section::mc_Rectangular};
			const auto BeginPointPrimitive {ExistingGroup->GetFirstDifferentPoint(EndPointPrimitive)};
			if (BeginPointPrimitive != nullptr) {
				EoGeLineSeg3d ExistingSectionReferenceLine(BeginPointPrimitive->Position(), CurrentPnt);
				const auto IntersectionPoint {ExistingSectionReferenceLine.ProjPt(m_PreviousPnt)};
				double Relationship;
				ExistingSectionReferenceLine.ParametricRelationshipOf(IntersectionPoint, Relationship);
				if (Relationship > FLT_EPSILON) {
					m_CurrentReferenceLine.set(m_PreviousPnt, IntersectionPoint);
					const auto SectionLength {m_CurrentReferenceLine.length() - (m_PreviousSection.Width() + m_DuctSeamSize + ExistingSection.Width() * 0.5)};
					if (SectionLength > FLT_EPSILON) {
						m_CurrentReferenceLine.SetEndPoint(m_CurrentReferenceLine.ProjToEndPt(SectionLength));
						GenerateRectangularSection(m_CurrentReferenceLine, m_CenterLineEccentricity, m_PreviousSection, &m_PreviewGroup);
					}
					GenerateFullElbowTakeoff(ExistingGroup, ExistingSectionReferenceLine, ExistingSection, &m_PreviewGroup);
				}
			}
		} else {
			const auto TransitionLength {m_PreviousSection == m_CurrentSection ? 0.0 : LengthOfTransition(m_DuctJustification, m_TransitionSlope, m_PreviousSection, m_CurrentSection)};
			auto ReferenceLine {m_CurrentReferenceLine};
			if (m_BeginWithTransition) {
				if (TransitionLength != 0.0) {
					ReferenceLine.SetEndPoint(ReferenceLine.ProjToEndPt(TransitionLength));
					GenerateTransition(ReferenceLine, m_CenterLineEccentricity, m_DuctJustification, m_TransitionSlope, m_PreviousSection, m_CurrentSection, &m_PreviewGroup);
					ReferenceLine.SetStartPoint(ReferenceLine.endPoint());
					ReferenceLine.SetEndPoint(m_CurrentReferenceLine.endPoint());
				}
				if (m_CurrentReferenceLine.length() - TransitionLength > FLT_EPSILON) {
					GenerateRectangularSection(ReferenceLine, m_CenterLineEccentricity, m_CurrentSection, &m_PreviewGroup);
				}
			} else {
				if (ReferenceLine.length() - TransitionLength > FLT_EPSILON) {
					ReferenceLine.SetEndPoint(ReferenceLine.ProjToBegPt(TransitionLength));
					GenerateRectangularSection(ReferenceLine, m_CenterLineEccentricity, m_PreviousSection, &m_PreviewGroup);
					ReferenceLine.SetStartPoint(ReferenceLine.endPoint());
					ReferenceLine.SetEndPoint(m_CurrentReferenceLine.endPoint());
				}
				if (TransitionLength != 0.0) {
					GenerateTransition(ReferenceLine, m_CenterLineEccentricity, m_DuctJustification, m_TransitionSlope, m_PreviousSection, m_CurrentSection, &m_PreviewGroup);
				}
			}
		}

		/* <tas="RemoveDuplicatePrimitives no long considers lines to be equal if start point and end point are not equal. May result in xor issues with section end cap and elbow end caps being equal."
		m_PreviewGroup.RemoveDuplicatePrimitives();
		</tas> */
		GetDocument()->UpdateGroupInAllViews(EoDb::kGroupEraseSafe, &m_PreviewGroup);
	}
}

void AeSysView::GenerateEndCap(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint, const Section section, EoDbGroup* group) const {
	const auto Midpoint {startPoint + (endPoint - startPoint) * 0.5};
	auto ResourceBuffer {OdResBuf::newRb(OdResBuf::kDxfRegAppName, L"AeSys")};
	ResourceBuffer->last()->setNext(OdResBuf::newRb(OdResBuf::kDxfXdReal, section.Width()));
	ResourceBuffer->last()->setNext(OdResBuf::newRb(OdResBuf::kDxfXdReal, section.Depth()));
	OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	auto Point {EoDbPoint::Create(BlockTableRecord)};
	Point->setPosition(Midpoint);
	Point->setColorIndex(15);
	Point->setXData(ResourceBuffer);
	group->AddTail(EoDbPoint::Create(Point));

	/* <tas="display mode 8 is significant for the endcap join. On save to peg, if xdata defined set display mode to 8."
	PointPrimitive->SetPointDisplayMode(8);
	</tas> */
	auto Line {EoDbLine::Create(BlockTableRecord, startPoint, endPoint)};
	Line->setColorIndex(static_cast<unsigned short>(g_PrimitiveState.ColorIndex()));
	Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex()));
	group->AddTail(EoDbLine::Create(Line));
}

void AeSysView::GenerateFullElbowTakeoff(EoDbGroup* /*group*/, EoGeLineSeg3d& existingSectionReferenceLine, const Section existingSection, EoDbGroup* group) {
	const auto NewSectionDirection {existingSectionReferenceLine.endPoint() - existingSectionReferenceLine.startPoint()};
	auto IntersectionPoint {existingSectionReferenceLine.ProjPt(m_PreviousPnt)};
	EoGeLineSeg3d PreviousReferenceLine(m_PreviousPnt, IntersectionPoint);
	PreviousReferenceLine.SetEndPoint(PreviousReferenceLine.ProjToBegPt((existingSection.Width() + m_PreviousSection.Width()) * 0.5));
	EoGeLineSeg3d CurrentReferenceLine(PreviousReferenceLine.endPoint(), PreviousReferenceLine.endPoint() + NewSectionDirection);
	GenerateRectangularElbow(PreviousReferenceLine, m_PreviousSection, CurrentReferenceLine, m_CurrentSection, group);
	IntersectionPoint = existingSectionReferenceLine.ProjPt(CurrentReferenceLine.startPoint());
	double Relationship;
	if (existingSectionReferenceLine.ParametricRelationshipOf(IntersectionPoint, Relationship)) {
		if (fabs(Relationship) > FLT_EPSILON && fabs(Relationship - 1.0) > FLT_EPSILON) { // need to add a section either from the elbow or the existing section
			const auto SectionLength {existingSectionReferenceLine.length()};
			auto DistanceToBeginPoint {Relationship * SectionLength};
			if (Relationship > FLT_EPSILON && Relationship < 1.0 - FLT_EPSILON) { // section from the elbow
				const auto StartPoint {CurrentReferenceLine.startPoint()};
				CurrentReferenceLine.SetEndPoint(ProjectToward(StartPoint, CurrentReferenceLine.endPoint(), SectionLength - DistanceToBeginPoint));
				GenerateRectangularSection(CurrentReferenceLine, m_CenterLineEccentricity, m_PreviousSection, group);
			} else {
				DistanceToBeginPoint = EoMax(DistanceToBeginPoint, SectionLength);
				const auto StartPoint {existingSectionReferenceLine.startPoint()};
				existingSectionReferenceLine.SetEndPoint(ProjectToward(StartPoint, existingSectionReferenceLine.endPoint(), DistanceToBeginPoint));
			}
		}
		// generate the transition
		OdGePoint3d Points[2];
		const auto EndPoint {existingSectionReferenceLine.endPoint()};
		Points[0] = ProjectToward(EndPoint, CurrentReferenceLine.endPoint(), existingSection.Width() * 0.5 + m_PreviousSection.Width());
		Points[1] = ProjectToward(Points[0], existingSectionReferenceLine.endPoint(), existingSection.Width() + m_PreviousSection.Width());
		const auto MiddleOfTransition {Points[0] + OdGeVector3d(Points[1] - Points[0]) * 0.5};
		EoGeLineSeg3d TransitionReferenceLine(MiddleOfTransition, MiddleOfTransition + NewSectionDirection);
		const auto Width {m_PreviousSection.Width() + existingSection.Width()};
		const auto Depth {m_PreviousSection.Depth() + existingSection.Depth()};
		const Section ContinueGroup(Width, Depth, Section::mc_Rectangular);
		const Section CurrentSection(Width * 0.75, Depth * 0.75, Section::mc_Rectangular);
		GenerateTransition(TransitionReferenceLine, m_CenterLineEccentricity, m_DuctJustification, m_TransitionSlope, ContinueGroup, CurrentSection, group);
	}
	/*
		if (m_GenerateTurningVanes) {
			OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	
			OdGePoint3dArray Points;
			Points.setLogicalLength(5);
	
			Points[2] = ProjectToward(rPar[0][1], rPar[1][1], dEcc2 * m_PreviousSection.Width());
			EoGeLineSeg3d(Points[2], rPar[1][1]).ProjPtFrom_xy(0.0, m_DuctSeamSize, Points[3]);
			dDSiz = dDSiz / m_PreviousSection.Width() * m_PreviousSection.Width();
			EoGeLineSeg3d(Points[2], rPar[1][1]).ProjPtFrom_xy(0.0, dDSiz + m_DuctSeamSize, Points[4]);
			EoDbGroup* Group = new EoDbGroup;
			GetDocument()->AddWorkLayerGroup(Group);
	
			auto Line {EoDbLine::Create(BlockTableRecord, lnLead[0], Points[2])};
			Line->setColorIndex(1);
			Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
			Group->AddTail(EoDbLine::Create(Line));
	
			Group->AddTail(new EoDbEllipse(1, pstate.LinetypeIndex(), Points[3], 0.01));
	
			auto Line {EoDbLine::Create(BlockTableRecord, Points[3], Points[4])};
			Line->setColorIndex(1);
			Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
			Group->AddTail(EoDbLine::Create(Line));
	
			GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
	
		}
	*/
}

void AeSysView::GenerateRiseDrop(const unsigned short riseDropIndicator, const Section section, EoGeLineSeg3d& referenceLine, EoDbGroup* group) {
	const auto SectionLength {referenceLine.length()};
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	const auto ColorIndex {g_PrimitiveState.ColorIndex()};
	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	OdDbLinePtr Line;
	EoGeLineSeg3d LeftLine;
	EoGeLineSeg3d RightLine;
	referenceLine.GetParallels(section.Width(), m_CenterLineEccentricity, LeftLine, RightLine);
	if (SectionLength >= section.Depth() * 0.5 + m_DuctSeamSize) {
		auto ReferenceLine {referenceLine};
		const auto StartPoint {ReferenceLine.startPoint()};
		ReferenceLine.SetEndPoint(ProjectToward(StartPoint, ReferenceLine.endPoint(), m_DuctSeamSize));
		ReferenceLine.GetParallels(section.Width(), m_CenterLineEccentricity, LeftLine, RightLine);
		Line = EoDbLine::Create(BlockTableRecord, LeftLine.startPoint(), LeftLine.endPoint());
		Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
		Line->setLinetype(Linetype);
		group->AddTail(EoDbLine::Create(Line));
		Line = EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), RightLine.endPoint());
		Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
		Line->setLinetype(Linetype);
		group->AddTail(EoDbLine::Create(Line));
		referenceLine.SetStartPoint(ReferenceLine.endPoint());
	}
	referenceLine.GetParallels(section.Width(), m_CenterLineEccentricity, LeftLine, RightLine);
	GenerateRectangularSection(referenceLine, m_CenterLineEccentricity, section, group);
	// need to allow continuation perpendicular to vertical section ?
	Line = EoDbLine::Create(BlockTableRecord, LeftLine.startPoint(), RightLine.endPoint());
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(static_cast<short>(riseDropIndicator)));
	group->AddTail(EoDbLine::Create(Line));
	Line = EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), LeftLine.endPoint());
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(static_cast<short>(riseDropIndicator)));
	group->AddTail(EoDbLine::Create(Line));
}

void AeSysView::GenerateRectangularElbow(EoGeLineSeg3d& previousReferenceLine, const Section previousSection, EoGeLineSeg3d& currentReferenceLine, const Section currentSection, EoDbGroup* group, const bool generateEndCaps) const {
	if (previousReferenceLine.isParallelTo(currentReferenceLine)) {
		return;
	}
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	const auto EndPoint {previousReferenceLine.endPoint()};
	previousReferenceLine.SetEndPoint(ProjectToward(EndPoint, previousReferenceLine.startPoint(), m_DuctSeamSize + previousSection.Width() * m_CenterLineEccentricity));
	EoGeLineSeg3d PreviousLeftLine;
	EoGeLineSeg3d PreviousRightLine;
	previousReferenceLine.GetParallels(previousSection.Width(), m_CenterLineEccentricity, PreviousLeftLine, PreviousRightLine);
	const auto StartPoint {currentReferenceLine.startPoint()};
	currentReferenceLine.SetStartPoint(ProjectToward(StartPoint, currentReferenceLine.endPoint(), m_DuctSeamSize + previousSection.Width() * m_CenterLineEccentricity));
	EoGeLineSeg3d CurrentLeftLine;
	EoGeLineSeg3d CurrentRightLine;
	currentReferenceLine.GetParallels(currentSection.Width(), m_CenterLineEccentricity, CurrentLeftLine, CurrentRightLine);
	OdGePoint3d InsideCorner;
	OdGePoint3d OutsideCorner;
	PreviousLeftLine.IntersectWith_xy(CurrentLeftLine, InsideCorner);
	PreviousRightLine.IntersectWith_xy(CurrentRightLine, OutsideCorner);
	if (generateEndCaps) {
		GenerateEndCap(PreviousLeftLine.endPoint(), PreviousRightLine.endPoint(), previousSection, group);
	}
	const auto ColorIndex {g_PrimitiveState.ColorIndex()};
	const auto LinetypeObjectId {EoDbPrimitive::LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	auto Line = EoDbLine::Create(BlockTableRecord, PreviousLeftLine.endPoint(), InsideCorner);
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(LinetypeObjectId);
	group->AddTail(EoDbLine::Create(Line));
	Line = EoDbLine::Create(BlockTableRecord, InsideCorner, CurrentLeftLine.startPoint());
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(LinetypeObjectId);
	group->AddTail(EoDbLine::Create(Line));
	Line = EoDbLine::Create(BlockTableRecord, PreviousRightLine.endPoint(), OutsideCorner);
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(LinetypeObjectId);
	group->AddTail(EoDbLine::Create(Line));
	Line = EoDbLine::Create(BlockTableRecord, OutsideCorner, CurrentRightLine.startPoint());
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(LinetypeObjectId);
	group->AddTail(EoDbLine::Create(Line));
	if (m_GenerateTurningVanes) {
		Line = EoDbLine::Create(BlockTableRecord, InsideCorner, OutsideCorner);
		Line->setColorIndex(2);
		Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(2));
		group->AddTail(EoDbLine::Create(Line));
	}
	if (generateEndCaps) {
		GenerateEndCap(CurrentLeftLine.startPoint(), CurrentRightLine.startPoint(), currentSection, group);
	}
}

void AeSysView::GenerateRectangularSection(EoGeLineSeg3d& referenceLine, const double eccentricity, const Section section, EoDbGroup* group) const {
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	EoGeLineSeg3d LeftLine;
	EoGeLineSeg3d RightLine;
	if (referenceLine.GetParallels(section.Width(), eccentricity, LeftLine, RightLine)) {
		GenerateEndCap(LeftLine.startPoint(), RightLine.startPoint(), section, group);
		const auto ColorIndex {g_PrimitiveState.ColorIndex()};
		const auto LinetypeObjectId {EoDbPrimitive::LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
		auto Line {EoDbLine::Create(BlockTableRecord, LeftLine.startPoint(), LeftLine.endPoint())};
		Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
		Line->setLinetype(LinetypeObjectId);
		group->AddTail(EoDbLine::Create(Line));
		GenerateEndCap(LeftLine.endPoint(), RightLine.endPoint(), section, group);
		Line = EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), RightLine.endPoint());
		Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
		Line->setLinetype(LinetypeObjectId);
		group->AddTail(EoDbLine::Create(Line));
	}
}

void AeSysView::GenSizeNote(const OdGePoint3d& position, const double angle, const Section section) {
	const auto XDirection {OdGeVector3d(0.06, 0.0, 0.0).rotateBy(angle, OdGeVector3d::kZAxis)};
	const auto YDirection {OdGeVector3d(0.0, 0.1, 0.0).rotateBy(angle, OdGeVector3d::kZAxis)};
	const EoGeReferenceSystem ReferenceSystem(position, XDirection, YDirection);
	OdGeVector3d PlaneNormal;
	ReferenceSystem.GetUnitNormal(PlaneNormal);
	OdString Note;
	Note += theApp.FormatLength(section.Width(), max(theApp.GetUnits(), AeSys::kInches), 8, 3);
	Note += L"/";
	Note += theApp.FormatLength(section.Depth(), max(theApp.GetUnits(), AeSys::kInches), 8, 3);
	const auto DeviceContext {GetDC()};
	const auto PrimitiveState {g_PrimitiveState.Save()};
	g_PrimitiveState.SetColorIndex(DeviceContext, 2);
	auto FontDefinition {g_PrimitiveState.FontDefinition()};
	FontDefinition.SetHorizontalAlignment(EoDb::kAlignCenter);
	FontDefinition.SetVerticalAlignment(EoDb::kAlignMiddle);
	auto CharacterCellDefinition {g_PrimitiveState.CharacterCellDefinition()};
	CharacterCellDefinition.SetRotationAngle(0.0);
	g_PrimitiveState.SetCharacterCellDefinition(CharacterCellDefinition);
	auto Group {new EoDbGroup};
	OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	auto Text {EoDbText::Create(BlockTableRecord, ReferenceSystem.Origin(), Note)};
	Text->setNormal(PlaneNormal);
	Text->setRotation(ReferenceSystem.Rotation());
	Text->setHeight(ReferenceSystem.YDirection().length());
	Text->setAlignmentPoint(ReferenceSystem.Origin());
	Text->setHorizontalMode(OdDb::kTextCenter);
	Text->setVerticalMode(OdDb::kTextVertMid);
	Group->AddTail(EoDbText::Create(Text));
	GetDocument()->AddWorkLayerGroup(Group);
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	g_PrimitiveState.Restore(*DeviceContext, PrimitiveState);
	ReleaseDC(DeviceContext);
}

bool AeSysView::GenerateRectangularTap(const EJust justification, const Section section) {
	EoGeLineSeg3d LeftLine;
	EoGeLineSeg3d RightLine;
	OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	const auto ColorIndex {g_PrimitiveState.ColorIndex()};
	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	auto SectionLength {m_CurrentReferenceLine.length()};
	if (SectionLength < m_DuctTapSize + m_DuctSeamSize) {
		m_CurrentReferenceLine.SetStartPoint(m_CurrentReferenceLine.ProjToBegPt(m_DuctTapSize + m_DuctSeamSize));
		SectionLength = m_DuctTapSize + m_DuctSeamSize;
	}
	auto ReferenceLine {m_CurrentReferenceLine};
	ReferenceLine.SetEndPoint(ReferenceLine.ProjToEndPt(m_DuctSeamSize));
	ReferenceLine.GetParallels(section.Width(), m_CenterLineEccentricity, LeftLine, RightLine);
	auto Section {new EoDbGroup};
	GetDocument()->AddWorkLayerGroup(Section);
	GenerateEndCap(LeftLine.startPoint(), RightLine.startPoint(), section, Section);
	auto Line {EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), RightLine.endPoint())};
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(Linetype);
	Section->AddTail(EoDbLine::Create(Line));
	Line = EoDbLine::Create(BlockTableRecord, RightLine.endPoint(), LeftLine.endPoint());
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(Linetype);
	Section->AddTail(EoDbLine::Create(Line));
	Line = EoDbLine::Create(BlockTableRecord, LeftLine.startPoint(), LeftLine.endPoint());
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(Linetype);
	Section->AddTail(EoDbLine::Create(Line));
	m_CurrentReferenceLine.SetStartPoint(ReferenceLine.endPoint());
	m_CurrentReferenceLine.GetParallels(section.Width(), m_CenterLineEccentricity, LeftLine, RightLine);
	OdGePoint3d EndPoint;
	if (justification == kRight) {
		RightLine.ProjPtFrom_xy(m_DuctTapSize, -m_DuctTapSize, EndPoint);
		RightLine.SetEndPoint(EndPoint);
	} else {
		LeftLine.ProjPtFrom_xy(m_DuctTapSize, m_DuctTapSize, EndPoint);
		LeftLine.SetEndPoint(EndPoint);
	}
	Line = EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), RightLine.endPoint());
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(Linetype);
	Section->AddTail(EoDbLine::Create(Line));
	Line = EoDbLine::Create(BlockTableRecord, LeftLine.endPoint(), LeftLine.startPoint());
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(Linetype);
	Section->AddTail(EoDbLine::Create(Line));
	if (m_GenerateTurningVanes) {
		const auto BeginPoint {(justification == kLeft ? RightLine : LeftLine).ProjToBegPt(-m_DuctTapSize / 3.0)};
		EndPoint = m_CurrentReferenceLine.ProjToBegPt(-m_DuctTapSize / 2.0);
		const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};
		auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, BeginPoint, ActiveViewPlaneNormal, 0.01)};
		Circle->setColorIndex(1);
		Circle->setLinetype(L"Continuous");
		Section->AddTail(EoDbEllipse::Create(Circle));
		Line = EoDbLine::Create(BlockTableRecord, BeginPoint, EndPoint);
		Line->setColorIndex(1);
		Line->setLinetype(Linetype);
		Section->AddTail(EoDbLine::Create(Line));
	}
	GetDocument()->UpdateGroupInAllViews(EoDb::kGroupSafe, Section);
	return true;
}

void AeSysView::GenerateTransition(EoGeLineSeg3d& referenceLine, const double eccentricity, const EJust justification, const double slope, const Section previousSection, const Section currentSection, EoDbGroup* group) const {
	const auto ReferenceLength {referenceLine.length()};
	if (ReferenceLength <= FLT_EPSILON) {
		return;
	}
	const OdDbBlockTableRecordPtr BlockTableRecord {Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
	const auto ColorIndex {g_PrimitiveState.ColorIndex()};
	const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(g_PrimitiveState.LinetypeIndex())};
	const auto WidthChange {currentSection.Width() - previousSection.Width()};
	auto TransitionLength {LengthOfTransition(justification, slope, previousSection, currentSection)};
	TransitionLength = EoMin(TransitionLength, ReferenceLength);
	EoGeLineSeg3d LeftLine;
	EoGeLineSeg3d RightLine;
	referenceLine.GetParallels(previousSection.Width(), eccentricity, LeftLine, RightLine);
	OdGePoint3d EndPoint;
	if (justification == kCenter) {
		LeftLine.ProjPtFrom_xy(TransitionLength, WidthChange * 0.5, EndPoint);
		LeftLine.SetEndPoint(EndPoint);
		RightLine.ProjPtFrom_xy(TransitionLength, -WidthChange * 0.5, EndPoint);
		RightLine.SetEndPoint(EndPoint);
	} else if (justification == kRight) {
		RightLine.ProjPtFrom_xy(TransitionLength, -WidthChange, EndPoint);
		RightLine.SetEndPoint(EndPoint);
	} else {
		LeftLine.ProjPtFrom_xy(TransitionLength, WidthChange, EndPoint);
		LeftLine.SetEndPoint(EndPoint);
	}
	GenerateEndCap(LeftLine.startPoint(), RightLine.startPoint(), previousSection, group);
	auto Line {EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), RightLine.endPoint())};
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(Linetype);
	group->AddTail(EoDbLine::Create(Line));
	GenerateEndCap(RightLine.endPoint(), LeftLine.endPoint(), currentSection, group);
	Line = EoDbLine::Create(BlockTableRecord, LeftLine.endPoint(), LeftLine.startPoint());
	Line->setColorIndex(static_cast<unsigned short>(ColorIndex));
	Line->setLinetype(Linetype);
	group->AddTail(EoDbLine::Create(Line));
}

void AeSysView::SetDuctOptions(Section& section) {
	const auto Units {theApp.GetUnits()};
	theApp.SetUnits(max(Units, AeSys::kInches));
	EoDlgLowPressureDuctOptions dlg(this);
	dlg.width = section.Width();
	dlg.depth = section.Depth();
	dlg.radiusFactor = m_InsideRadiusFactor;
	dlg.justification = m_DuctJustification;
	dlg.generateVanes = m_GenerateTurningVanes;
	dlg.beginWithTransition = m_BeginWithTransition;
	if (dlg.DoModal() == IDOK) {
		section.SetWidth(dlg.width);
		section.SetDepth(dlg.depth);
		m_InsideRadiusFactor = dlg.radiusFactor;
		m_DuctJustification = EJust(dlg.justification);
		m_GenerateTurningVanes = dlg.generateVanes;
		m_BeginWithTransition = dlg.beginWithTransition;
	}
	theApp.SetUnits(Units);
}

double AeSysView::LengthOfTransition(const EJust justification, const double slope, const Section previousSection, const Section currentSection) const noexcept {
	const auto WidthChange {currentSection.Width() - previousSection.Width()};
	const auto DepthChange {currentSection.Depth() - previousSection.Depth()};
	auto Length {EoMax(fabs(WidthChange), fabs(DepthChange)) * slope};
	if (justification == kCenter) {
		Length *= 0.5;
	}
	return Length;
}

bool AeSysView::Find2LinesUsingLineEndpoints(EoDbLine* testLinePrimitive, const double angularTolerance, EoGeLineSeg3d& leftLine, EoGeLineSeg3d& rightLine) {
	EoDbLine* LeftLinePrimitive {nullptr};
	auto DirectedRelationship {0};
	const auto TestLine {testLinePrimitive->LineSeg()};
	const auto TestLineAngle {fmod(TestLine.AngleFromXAxis_xy(), OdaPI)};
	auto GroupPosition {GetLastGroupPosition()};
	while (GroupPosition != nullptr) {
		const auto Group {GetPreviousGroup(GroupPosition)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive == testLinePrimitive || Primitive->IsKindOf(RUNTIME_CLASS(EoDbLine)) == 0) {
				continue;
			}
			const auto LinePrimitive {dynamic_cast<EoDbLine*>(Primitive)};
			auto LineSeg {LinePrimitive->LineSeg()};
			if (LineSeg.startPoint() == TestLine.startPoint() || LineSeg.startPoint() == TestLine.endPoint()) { // Exchange points
				const auto Point {LineSeg.startPoint()};
				LineSeg.SetStartPoint(LineSeg.endPoint());
				LineSeg.SetEndPoint(Point);
			} else if (LineSeg.endPoint() != TestLine.startPoint() && LineSeg.endPoint() != TestLine.endPoint()) { //	No endpoint coincides with one of the test line endpoints
				continue;
			}
			const auto LineAngle {fmod(LineSeg.AngleFromXAxis_xy(), OdaPI)};
			if (fabs(fabs(TestLineAngle - LineAngle) - OdaPI2) <= angularTolerance) {
				if (LeftLinePrimitive == nullptr) { // No qualifiers yet
					DirectedRelationship = TestLine.DirectedRelationshipOf(LineSeg.startPoint());
					LeftLinePrimitive = LinePrimitive;
					leftLine = LineSeg;
				} else {
					if (DirectedRelationship == TestLine.DirectedRelationshipOf(LineSeg.startPoint())) { // Both lines are on the same side of test line
						rightLine = LineSeg;
						if (rightLine.DirectedRelationshipOf(leftLine.startPoint()) != 1) {
							auto RightLinePrimitive {LeftLinePrimitive};
							rightLine = leftLine;
							LeftLinePrimitive = LinePrimitive;
							leftLine = LineSeg;
						}
						return true;
					}
				}
			}
		}
	}
	return false;
}
