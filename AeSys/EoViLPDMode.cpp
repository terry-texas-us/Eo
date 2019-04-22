#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoDlgLowPressureDuctOptions.h"

std::pair<EoDbGroup*, EoDbPoint*> AeSysView::SelectPointUsingPoint(const OdGePoint3d& point, double tolerance, OdInt16 pointColor) {
    auto GroupPosition {GetFirstVisibleGroupPosition()};
    while (GroupPosition != nullptr) {
        auto Group = GetNextVisibleGroup(GroupPosition);
        auto PrimitivePosition = Group->GetHeadPosition();
        while (PrimitivePosition != nullptr) {
            auto Primitive = Group->GetNext(PrimitivePosition);
            if (Primitive->Is(kPointPrimitive) && Primitive->ColorIndex() == pointColor) {
                const auto Point = dynamic_cast<EoDbPoint*>(Primitive);
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
///Group data contains whatever primative follows marker (hopefully this is associated end-cap line).
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
    
    auto Selection {SelectPointUsingPoint(CurrentPnt, .01, 15)};
    m_EndCapGroup = std::get<0>(Selection);

    if (m_EndCapGroup != nullptr) {
        m_EndCapPoint = std::get<1>(Selection);

        m_PreviousPnt = m_EndCapPoint->Position();
        m_PreviousSection.SetWidth(m_EndCapPoint->DataAt(0));
        m_PreviousSection.SetDepth(m_EndCapPoint->DataAt(1));
        m_ContinueSection = false;

        m_EndCapLocation = (m_PreviousOp == 0) ? 1 : -1; // 1 (start) and -1 (end)

        OdString Message(L"Cross sectional dimension (Width by Depth) is ");
        Message += theApp.FormatLength(m_PreviousSection.Width(), max(theApp.GetUnits(), AeSysApp::kInches), 12, 2);
        Message += L" by ";
        Message += theApp.FormatLength(m_PreviousSection.Depth(), max(theApp.GetUnits(), AeSysApp::kInches), 12, 2);
        theApp.AddStringToMessageList(Message);
        SetCursorPosition(m_PreviousPnt);
    }
}

void AeSysView::OnLpdModeDuct() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	if (m_PreviousOp != 0) {
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	if (m_PreviousOp == ID_OP2) {
		CurrentPnt = SnapPointToAxis(m_PreviousPnt, CurrentPnt);
		m_CurrentReferenceLine.set(m_PreviousPnt, CurrentPnt);

		if (m_ContinueSection) {
            auto Group {new EoDbGroup};
			GetDocument()->AddWorkLayerGroup(Group);
			GenerateRectangularElbow(m_PreviousReferenceLine, m_PreviousSection, m_CurrentReferenceLine, m_CurrentSection, Group);
			m_OriginalPreviousGroup->DeletePrimitivesAndRemoveAll();
			GenerateRectangularSection(m_PreviousReferenceLine, m_CenterLineEccentricity, m_PreviousSection, m_OriginalPreviousGroup);
			m_OriginalPreviousGroupDisplayed = true;
			m_PreviousSection = m_CurrentSection;
		}
		const double TransitionLength = (m_PreviousSection == m_CurrentSection) ? 0. : LengthOfTransition(m_DuctJustification, m_TransitionSlope, m_PreviousSection, m_CurrentSection);
		EoGeLineSeg3d ReferenceLine(m_CurrentReferenceLine);

		if (m_BeginWithTransition) {
			if (TransitionLength != 0.) {
				ReferenceLine.SetEndPoint(ReferenceLine.ProjToEndPt(TransitionLength));

                auto Group {new EoDbGroup};
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
		}
		else {
			if (ReferenceLine.length() - TransitionLength > FLT_EPSILON) {
				ReferenceLine.SetEndPoint(ReferenceLine.ProjToBegPt(TransitionLength));
				m_OriginalPreviousGroup = new EoDbGroup;
				GetDocument()->AddWorkLayerGroup(m_OriginalPreviousGroup);
				GenerateRectangularSection(ReferenceLine, m_CenterLineEccentricity, m_PreviousSection, m_OriginalPreviousGroup);
				ReferenceLine.SetStartPoint(ReferenceLine.endPoint());
				ReferenceLine.SetEndPoint(m_CurrentReferenceLine.endPoint());
				m_ContinueSection = true;
			}
			if (TransitionLength != 0.) {
				EoDbGroup* Group = new EoDbGroup;
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

	m_BeginWithTransition = (m_PreviousOp == 0) ? true : false;

	DoDuctModeMouseMove();
	OnLpdModeDuct();
}
void AeSysView::OnLpdModeTap() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	if (m_PreviousOp != 0) {
		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	EoDbLine* LinePrimitive;
    auto Group {SelectLineBy(CurrentPnt, LinePrimitive)};
	if (Group != 0) {
		const OdGePoint3d TestPoint(CurrentPnt);
		CurrentPnt = SnapPointToAxis(m_PreviousPnt, CurrentPnt);
		CurrentPnt = LinePrimitive->ProjPt_(CurrentPnt);
		m_CurrentReferenceLine.set(m_PreviousPnt, CurrentPnt);

		EJust Justification;
		const int Relationship = m_CurrentReferenceLine.DirectedRelationshipOf(TestPoint);
		if (Relationship == 1) {
			Justification = Left;
		}
		else if (Relationship == - 1) {
			Justification = Right;
		}
		else {
			theApp.AddStringToMessageList(L"Could not determine orientation of component");
			return;
		}
		if (m_PreviousOp == ID_OP2) {
			if (m_ContinueSection) {
				Group = new EoDbGroup;
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateRectangularElbow(m_PreviousReferenceLine, m_PreviousSection, m_CurrentReferenceLine, m_CurrentSection, Group);
				m_PreviousSection = m_CurrentSection;
			}
			const double SectionLength = m_CurrentReferenceLine.length();
			if (SectionLength >= m_DuctTapSize + m_DuctSeamSize) {
				EoGeLineSeg3d ReferenceLine(m_CurrentReferenceLine);
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
	}
	else
		theApp.AddStringToMessageList(IDS_MSG_LINE_NOT_SELECTED);
}
void AeSysView::OnLpdModeEll() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	if (m_PreviousOp != 0) {
		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
    if (m_PreviousOp == ID_OP2) {
        auto Selection {SelectPointUsingPoint(CurrentPnt, .01, 15)};
        auto ExistingGroup {std::get<0>(Selection)};

		if (ExistingGroup == nullptr) {
			theApp.AddStringToMessageList(IDS_MSG_LPD_NO_END_CAP_LOC);
			return;
		}
        auto EndPointPrimitive {std::get<1>(Selection)};
        CurrentPnt = EndPointPrimitive->Position();
		Section ExistingSection(EndPointPrimitive->DataAt(0), EndPointPrimitive->DataAt(1), Section::Rectangular);

        auto BeginPointPrimitive {ExistingGroup->GetFirstDifferentPoint(EndPointPrimitive)};
		if (BeginPointPrimitive != 0) {
			EoGeLineSeg3d ExistingSectionReferenceLine(BeginPointPrimitive->Position(), CurrentPnt);

			const auto IntersectionPoint(ExistingSectionReferenceLine.ProjPt(m_PreviousPnt));
			double Relationship;
			ExistingSectionReferenceLine.ParametricRelationshipOf(IntersectionPoint, Relationship);
			if (Relationship > FLT_EPSILON) {
				m_CurrentReferenceLine.set(m_PreviousPnt, IntersectionPoint);
				const double SectionLength = m_CurrentReferenceLine.length() - (m_PreviousSection.Width() + m_DuctSeamSize + ExistingSection.Width() * .5);
				if (SectionLength > FLT_EPSILON) {
					m_CurrentReferenceLine.SetEndPoint(m_CurrentReferenceLine.ProjToEndPt(SectionLength));
                    auto Group {new EoDbGroup};
					GetDocument()->AddWorkLayerGroup(Group);
					GenerateRectangularSection(m_CurrentReferenceLine, m_CenterLineEccentricity, m_PreviousSection, Group);
					GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
				}
                auto Group {new EoDbGroup};
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateFullElbowTakeoff(ExistingGroup, ExistingSectionReferenceLine, ExistingSection, Group);
				GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
			}
		}
		// determine where cursor should be moved to.
	}
	m_ContinueSection = false;
	m_PreviousOp = ID_OP2;
}
void AeSysView::OnLpdModeTee() {
	const OdGePoint3d CurrentPnt = GetCursorPosition();

	if (m_PreviousOp != 0) {
		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();
	}
	//m_PreviousPnt = GenerateBullheadTee(this, m_PreviousPnt, CurrentPnt, m_PreviousSection);

	m_ContinueSection = false;
	m_PreviousOp = ID_OP2;
}
void AeSysView::OnLpdModeUpDown() {
	OdGePoint3d CurrentPnt = GetCursorPosition();

	const int iRet = 0; // dialog to "Select direction", 'Up.Down.'
	if (iRet >= 0) {
		if (m_PreviousOp == ID_OP2) {
			CurrentPnt = SnapPointToAxis(m_PreviousPnt, CurrentPnt);
			m_CurrentReferenceLine.set(m_PreviousPnt, CurrentPnt);

			if (m_ContinueSection) {
                auto Group {new EoDbGroup};
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateRectangularElbow(m_PreviousReferenceLine, m_PreviousSection, m_CurrentReferenceLine, m_CurrentSection, Group);
				m_PreviousSection = m_CurrentSection;
			}
			const double SectionLength = m_CurrentReferenceLine.length();
			if (SectionLength > m_PreviousSection.Depth() * .5 + m_DuctSeamSize) {
				EoGeLineSeg3d ReferenceLine(m_CurrentReferenceLine);
				const OdGePoint3d StartPoint = ReferenceLine.startPoint();
				ReferenceLine.SetEndPoint(ProjectToward(StartPoint, ReferenceLine.endPoint(), SectionLength - m_PreviousSection.Depth() * .5 - m_DuctSeamSize));
                auto Group {new EoDbGroup};
				GetDocument()->AddWorkLayerGroup(Group);
				GenerateRectangularSection(ReferenceLine, m_CenterLineEccentricity, m_PreviousSection, Group);
				GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
				m_CurrentReferenceLine.SetStartPoint(ReferenceLine.endPoint());
			}
            auto Group {new EoDbGroup};
			GetDocument()->AddWorkLayerGroup(Group);
			GenerateRiseDrop(1, m_PreviousSection, m_CurrentReferenceLine, Group);
			GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
		}
		m_ContinueSection = false;
		m_PreviousOp = ID_OP2;
		m_PreviousPnt = CurrentPnt;
	}
}

void AeSysView::OnLpdModeSize() {
    const auto CurrentPnt {GetCursorPosition()};

    double Angle = 0.;
    if (m_EndCapPoint != nullptr) {
        if (m_EndCapPoint->ColorIndex() == 15) {
            POSITION Position = m_EndCapGroup->Find(m_EndCapPoint);
            m_EndCapGroup->GetNext(Position);
            auto pLine {dynamic_cast<EoDbLine*>(m_EndCapGroup->GetAt(Position))};
            auto Line = pLine->Line();
            Angle = fmod(Line.AngleFromXAxis_xy(), PI);
            if (Angle <= RADIAN)
                Angle += PI;
            Angle -= HALF_PI;
        }
        m_EndCapPoint = nullptr;
    }
    GenSizeNote(CurrentPnt, Angle, m_PreviousSection);
    if (m_PreviousOp != 0)
        RubberBandingDisable();
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
    GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
    m_PreviewGroup.DeletePrimitivesAndRemoveAll();

    if (!m_OriginalPreviousGroupDisplayed) {
        GetDocument()->UpdateGroupInAllViews(kGroupSafe, m_OriginalPreviousGroup);
        m_OriginalPreviousGroupDisplayed = true;
    }
    ModeLineUnhighlightOp(m_PreviousOp);
    m_PreviousOp = 0;

    m_ContinueSection = false;
    m_EndCapGroup = nullptr;
    m_EndCapPoint = nullptr;
}

void AeSysView::DoDuctModeMouseMove() {
	static OdGePoint3d CurrentPnt = OdGePoint3d();

	if (m_PreviousOp == 0) {
		CurrentPnt = GetCursorPosition();
		m_OriginalPreviousGroupDisplayed = true;
	}
	else if (m_PreviousOp == ID_OP2) {
		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
		m_PreviewGroup.DeletePrimitivesAndRemoveAll();

		CurrentPnt = GetCursorPosition();
		CurrentPnt = SnapPointToAxis(m_PreviousPnt, CurrentPnt);
		m_CurrentReferenceLine.set(m_PreviousPnt, CurrentPnt);

		if (m_ContinueSection && m_CurrentReferenceLine.length() > m_PreviousSection.Width() * m_CenterLineEccentricity + m_DuctSeamSize) {
			EoGeLineSeg3d PreviousReferenceLine = m_PreviousReferenceLine;
			if (m_OriginalPreviousGroupDisplayed) {
				GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, m_OriginalPreviousGroup);
				m_OriginalPreviousGroupDisplayed = false;
			}
			GenerateRectangularElbow(PreviousReferenceLine, m_PreviousSection, m_CurrentReferenceLine, m_CurrentSection, &m_PreviewGroup, false);
			GenerateRectangularSection(PreviousReferenceLine, m_CenterLineEccentricity, m_PreviousSection, &m_PreviewGroup);
		}
        auto Selection {SelectPointUsingPoint(CurrentPnt, .01, 15)};
        auto ExistingGroup {std::get<0>(Selection)};

        if (ExistingGroup != nullptr) {
            auto EndPointPrimitive {std::get<1>(Selection)};
            CurrentPnt = EndPointPrimitive->Position();
			Section ExistingSection(EndPointPrimitive->DataAt(0), EndPointPrimitive->DataAt(1), Section::Rectangular);

            auto BeginPointPrimitive {ExistingGroup->GetFirstDifferentPoint(EndPointPrimitive)};
			if (BeginPointPrimitive != 0) {
				EoGeLineSeg3d ExistingSectionReferenceLine(BeginPointPrimitive->Position(), CurrentPnt);

				const OdGePoint3d IntersectionPoint(ExistingSectionReferenceLine.ProjPt(m_PreviousPnt));
				double Relationship;
				ExistingSectionReferenceLine.ParametricRelationshipOf(IntersectionPoint, Relationship);
				if (Relationship > FLT_EPSILON) {
					m_CurrentReferenceLine.set(m_PreviousPnt, IntersectionPoint);
					const double SectionLength = m_CurrentReferenceLine.length() - (m_PreviousSection.Width() + m_DuctSeamSize + ExistingSection.Width() * .5);
					if (SectionLength > FLT_EPSILON) {
						m_CurrentReferenceLine.SetEndPoint(m_CurrentReferenceLine.ProjToEndPt(SectionLength));
						GenerateRectangularSection(m_CurrentReferenceLine, m_CenterLineEccentricity, m_PreviousSection, &m_PreviewGroup);
					}
					GenerateFullElbowTakeoff(ExistingGroup, ExistingSectionReferenceLine, ExistingSection, &m_PreviewGroup);
				}
			}
		}
		else {
			const double TransitionLength = (m_PreviousSection == m_CurrentSection) ? 0. : LengthOfTransition(m_DuctJustification, m_TransitionSlope, m_PreviousSection, m_CurrentSection);
			EoGeLineSeg3d ReferenceLine(m_CurrentReferenceLine);

			if (m_BeginWithTransition) {
				if (TransitionLength != 0.) {
					ReferenceLine.SetEndPoint(ReferenceLine.ProjToEndPt(TransitionLength));
					GenerateTransition(ReferenceLine, m_CenterLineEccentricity, m_DuctJustification, m_TransitionSlope, m_PreviousSection, m_CurrentSection, &m_PreviewGroup);
					ReferenceLine.SetStartPoint(ReferenceLine.endPoint());
					ReferenceLine.SetEndPoint(m_CurrentReferenceLine.endPoint());
				}
				if (m_CurrentReferenceLine.length() - TransitionLength > FLT_EPSILON) {
					GenerateRectangularSection(ReferenceLine, m_CenterLineEccentricity, m_CurrentSection, &m_PreviewGroup);
				}
			}
			else {
				if (ReferenceLine.length() - TransitionLength > FLT_EPSILON) {
					ReferenceLine.SetEndPoint(ReferenceLine.ProjToBegPt(TransitionLength));
					GenerateRectangularSection(ReferenceLine, m_CenterLineEccentricity, m_PreviousSection, &m_PreviewGroup);
					ReferenceLine.SetStartPoint(ReferenceLine.endPoint());
					ReferenceLine.SetEndPoint(m_CurrentReferenceLine.endPoint());
				}
				if (TransitionLength != 0.) {
					GenerateTransition(ReferenceLine, m_CenterLineEccentricity, m_DuctJustification, m_TransitionSlope, m_PreviousSection, m_CurrentSection, &m_PreviewGroup);
				}
			}
		}
		
        /* <tas="RemoveDuplicatePrimitives no long considers lines to be equal if start point and end point are not equal. May result in xor issues with section end cap and elbow end caps being equal."
		m_PreviewGroup.RemoveDuplicatePrimitives();
        </tas> */

		GetDocument()->UpdateGroupInAllViews(kGroupEraseSafe, &m_PreviewGroup);
	}
}

void AeSysView::GenerateEndCap(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint, Section section, EoDbGroup* group) {
    const auto Midpoint {startPoint + (endPoint - startPoint) * .5};

    auto ResourceBuffer {OdResBuf::newRb(OdResBuf::kDxfRegAppName, L"AeSys")};
    ResourceBuffer->last()->setNext(OdResBuf::newRb(OdResBuf::kDxfXdReal, section.Width()));
    ResourceBuffer->last()->setNext(OdResBuf::newRb(OdResBuf::kDxfXdReal, section.Depth()));

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
    auto Point {EoDbPoint::Create(BlockTableRecord)};
    Point->setPosition(Midpoint);
    Point->setColorIndex(15);
    Point->setXData(ResourceBuffer);
    group->AddTail(EoDbPoint::Create(Point));

    /* <tas="display mode 8 is significant for the endcap join. On save to peg, if xdata defined set display mode to 8."
    PointPrimitive->SetPointDisplayMode(8);
    </tas> */

    auto Line {EoDbLine::Create(BlockTableRecord, startPoint, endPoint)};
    Line->setColorIndex(pstate.ColorIndex());
    Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex()));
	group->AddTail(EoDbLine::Create(Line));
}

void AeSysView::GenerateFullElbowTakeoff(EoDbGroup*, EoGeLineSeg3d& existingSectionReferenceLine, Section existingSection, EoDbGroup* group) {
	const OdGeVector3d NewSectionDirection(existingSectionReferenceLine.endPoint() - existingSectionReferenceLine.startPoint());

	OdGePoint3d IntersectionPoint(existingSectionReferenceLine.ProjPt(m_PreviousPnt));
	EoGeLineSeg3d PreviousReferenceLine(m_PreviousPnt, IntersectionPoint);
	PreviousReferenceLine.SetEndPoint(PreviousReferenceLine.ProjToBegPt((existingSection.Width() + m_PreviousSection.Width()) * .5));
	EoGeLineSeg3d CurrentReferenceLine(PreviousReferenceLine.endPoint(), PreviousReferenceLine.endPoint() + NewSectionDirection);

	GenerateRectangularElbow(PreviousReferenceLine, m_PreviousSection, CurrentReferenceLine, m_CurrentSection, group);
	IntersectionPoint = existingSectionReferenceLine.ProjPt(CurrentReferenceLine.startPoint());
	double Relationship;
	if (existingSectionReferenceLine.ParametricRelationshipOf(IntersectionPoint, Relationship)) {
		if (fabs(Relationship) > FLT_EPSILON && fabs(Relationship - 1.) > FLT_EPSILON) { // need to add a section either from the elbow or the existing section
			const double SectionLength = existingSectionReferenceLine.length();
			double DistanceToBeginPoint = Relationship * SectionLength;
			if (Relationship > FLT_EPSILON && Relationship < 1. - FLT_EPSILON) { // section from the elbow
				const OdGePoint3d StartPoint = CurrentReferenceLine.startPoint();
				CurrentReferenceLine.SetEndPoint(ProjectToward(StartPoint, CurrentReferenceLine.endPoint(), SectionLength - DistanceToBeginPoint));
				GenerateRectangularSection(CurrentReferenceLine, m_CenterLineEccentricity, m_PreviousSection, group);
			}
			else {
				DistanceToBeginPoint = EoMax(DistanceToBeginPoint, SectionLength);
				const OdGePoint3d StartPoint = existingSectionReferenceLine.startPoint();
				existingSectionReferenceLine.SetEndPoint(ProjectToward(StartPoint, existingSectionReferenceLine.endPoint(), DistanceToBeginPoint));
			}
		}
		// generate the transition
		OdGePoint3d Points[2];
		const OdGePoint3d EndPoint = existingSectionReferenceLine.endPoint();
		Points[0] = ProjectToward(EndPoint, CurrentReferenceLine.endPoint(), existingSection.Width() * .5 + m_PreviousSection.Width());
		Points[1] = ProjectToward(Points[0], existingSectionReferenceLine.endPoint(), existingSection.Width() + m_PreviousSection.Width());

		const OdGePoint3d MiddleOfTransition = Points[0] + OdGeVector3d(Points[1] - Points[0]) * .5;
		EoGeLineSeg3d TransitionReferenceLine(MiddleOfTransition, MiddleOfTransition + NewSectionDirection);

		const double Width = m_PreviousSection.Width() + existingSection.Width();
		const double Depth = m_PreviousSection.Depth() + existingSection.Depth();
		Section ContinueGroup(Width, Depth, Section::Rectangular);
		Section CurrentSection(Width * .75, Depth * .75, Section::Rectangular);

		GenerateTransition(TransitionReferenceLine, m_CenterLineEccentricity, m_DuctJustification, m_TransitionSlope, ContinueGroup, CurrentSection, group);
	}
	if (m_GenerateTurningVanes) {
/*
		OdGePoint3dArray Points;
		Points.setLogicalLength(5);

		Points[2] = ProjectToward(rPar[0][1], rPar[1][1], dEcc2 * m_PreviousSection.Width());
		EoGeLineSeg3d(Points[2], rPar[1][1]).ProjPtFrom_xy(0., m_DuctSeamSize, Points[3]);
		dDSiz = dDSiz / m_PreviousSection.Width() * m_PreviousSection.Width();
		EoGeLineSeg3d(Points[2], rPar[1][1]).ProjPtFrom_xy(0., dDSiz + m_DuctSeamSize, Points[4]);
		EoDbGroup* Group = new EoDbGroup;
		GetDocument()->AddWorkLayerGroup(Group);
		Group->AddTail(new EoDbLine(1, pstate.LinetypeIndex(), lnLead[0], Points[2]));
		Group->AddTail(new EoDbEllipse(1, pstate.LinetypeIndex(), Points[3], .01));
		Group->AddTail(new EoDbLine(1, pstate.LinetypeIndex(), Points[3], Points[4]));
		GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
*/
	}
}
void AeSysView::GenerateRiseDrop(OdUInt16 riseDropIndicator, Section section, EoGeLineSeg3d& referenceLine, EoDbGroup* group) {
	const double SectionLength = referenceLine.length();

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    const auto ColorIndex {pstate.ColorIndex()};
    const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex())};

    OdDbLinePtr Line;
	EoGeLineSeg3d LeftLine;
	EoGeLineSeg3d RightLine;
	referenceLine.GetParallels(section.Width(), m_CenterLineEccentricity, LeftLine, RightLine);

	if (SectionLength >= section.Depth() * .5 + m_DuctSeamSize) {
		EoGeLineSeg3d ReferenceLine(referenceLine);
        const auto StartPoint {ReferenceLine.startPoint()};
		ReferenceLine.SetEndPoint(ProjectToward(StartPoint, ReferenceLine.endPoint(), m_DuctSeamSize));
		ReferenceLine.GetParallels(section.Width(), m_CenterLineEccentricity, LeftLine, RightLine);

        Line = EoDbLine::Create(BlockTableRecord, LeftLine.startPoint(), LeftLine.endPoint());
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        group->AddTail(EoDbLine::Create(Line));

        Line = EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), RightLine.endPoint());
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(Linetype);
        group->AddTail(EoDbLine::Create(Line));
		
        referenceLine.SetStartPoint(ReferenceLine.endPoint());
	}
	referenceLine.GetParallels(section.Width(), m_CenterLineEccentricity, LeftLine, RightLine);
	GenerateRectangularSection(referenceLine, m_CenterLineEccentricity, section, group);
	// need to allow continuation perpendicular to vertical section ?
	Line = EoDbLine::Create(BlockTableRecord, LeftLine.startPoint(), RightLine.endPoint());
	Line->setColorIndex(ColorIndex);
	Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(riseDropIndicator));
	group->AddTail(EoDbLine::Create(Line));
	
    Line = EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), LeftLine.endPoint());
	Line->setColorIndex(ColorIndex);
	Line->setLinetype(EoDbPrimitive::LinetypeObjectFromIndex(riseDropIndicator));
	group->AddTail(EoDbLine::Create(Line));
}

void AeSysView::GenerateRectangularElbow(EoGeLineSeg3d& previousReferenceLine, Section previousSection, EoGeLineSeg3d& currentReferenceLine, Section currentSection, EoDbGroup* group, bool generateEndCaps) {
    if (previousReferenceLine.isParallelTo(currentReferenceLine))
        return;

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

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
    auto const ColorIndex {pstate.ColorIndex()};
    auto const LinetypeObjectId {EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex())};

    auto Line = EoDbLine::Create(BlockTableRecord, PreviousLeftLine.endPoint(), InsideCorner);
    Line->setColorIndex(ColorIndex);
    Line->setLinetype(LinetypeObjectId);
    group->AddTail(EoDbLine::Create(Line));

    Line = EoDbLine::Create(BlockTableRecord, InsideCorner, CurrentLeftLine.startPoint());
    Line->setColorIndex(ColorIndex);
    Line->setLinetype(LinetypeObjectId);
    group->AddTail(EoDbLine::Create(Line));

    Line = EoDbLine::Create(BlockTableRecord, PreviousRightLine.endPoint(), OutsideCorner);
    Line->setColorIndex(ColorIndex);
    Line->setLinetype(LinetypeObjectId);
    group->AddTail(EoDbLine::Create(Line));

    Line = EoDbLine::Create(BlockTableRecord, OutsideCorner, CurrentRightLine.startPoint());
    Line->setColorIndex(ColorIndex);
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

void AeSysView::GenerateRectangularSection(EoGeLineSeg3d& referenceLine, double eccentricity, Section section, EoDbGroup* group) {

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    EoGeLineSeg3d LeftLine;
    EoGeLineSeg3d RightLine;

    if (referenceLine.GetParallels(section.Width(), eccentricity, LeftLine, RightLine)) {
        GenerateEndCap(LeftLine.startPoint(), RightLine.startPoint(), section, group);

        auto const ColorIndex {pstate.ColorIndex()};
        auto const LinetypeObjectId {EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex())};

        auto Line {EoDbLine::Create(BlockTableRecord, LeftLine.startPoint(), LeftLine.endPoint())};
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(LinetypeObjectId);
        group->AddTail(EoDbLine::Create(Line));

        GenerateEndCap(LeftLine.endPoint(), RightLine.endPoint(), section, group);

        Line = EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), RightLine.endPoint());
        Line->setColorIndex(ColorIndex);
        Line->setLinetype(LinetypeObjectId);
        group->AddTail(EoDbLine::Create(Line));
    }
}

void AeSysView::GenSizeNote(const OdGePoint3d& position, double angle, Section section) {
    const auto XDirection {OdGeVector3d(0.06, 0., 0.).rotateBy(angle, OdGeVector3d::kZAxis)};
    const auto YDirection {OdGeVector3d(0., 0.1, 0.).rotateBy(angle, OdGeVector3d::kZAxis)};
    EoGeReferenceSystem ReferenceSystem(position, XDirection, YDirection);
    OdGeVector3d PlaneNormal;
    ReferenceSystem.GetUnitNormal(PlaneNormal);

    OdString Note;
    Note += theApp.FormatLength(section.Width(), max(theApp.GetUnits(), AeSysApp::kInches), 8, 3);
    Note += L"/";
    Note += theApp.FormatLength(section.Depth(), max(theApp.GetUnits(), AeSysApp::kInches), 8, 3);

    auto DeviceContext {GetDC()};
    const int PrimitiveState = pstate.Save();
    pstate.SetColorIndex(DeviceContext, 2);

    auto FontDefinition {pstate.FontDefinition()};
    FontDefinition.SetHorizontalAlignment(kAlignCenter);
    FontDefinition.SetVerticalAlignment(kAlignMiddle);

    auto CharacterCellDefinition {pstate.CharacterCellDefinition()};
    CharacterCellDefinition.SetRotationAngle(0.);
    pstate.SetCharacterCellDefinition(CharacterCellDefinition);

    auto Group {new EoDbGroup};

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
    auto Text {EoDbText::Create(BlockTableRecord, ReferenceSystem.Origin(), Note)};

    Text->setNormal(PlaneNormal);
    Text->setRotation(ReferenceSystem.Rotation());
    Text->setHeight(ReferenceSystem.YDirection().length());
    Text->setAlignmentPoint(ReferenceSystem.Origin());
    Text->setHorizontalMode(OdDb::kTextCenter);
    Text->setVerticalMode(OdDb::kTextVertMid);

    Group->AddTail(EoDbText::Create(Text));

    GetDocument()->AddWorkLayerGroup(Group);
    GetDocument()->UpdateGroupInAllViews(kGroupSafe, Group);
    pstate.Restore(DeviceContext, PrimitiveState);
    ReleaseDC(DeviceContext);
}

bool AeSysView::GenerateRectangularTap(EJust justification, Section section) {
	EoGeLineSeg3d LeftLine;
	EoGeLineSeg3d RightLine;

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    const auto ColorIndex {pstate.ColorIndex()};
    const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex())};

    double SectionLength = m_CurrentReferenceLine.length();

	if (SectionLength < m_DuctTapSize + m_DuctSeamSize) {
		m_CurrentReferenceLine.SetStartPoint(m_CurrentReferenceLine.ProjToBegPt(m_DuctTapSize + m_DuctSeamSize));
		SectionLength = m_DuctTapSize + m_DuctSeamSize;
	}
	EoGeLineSeg3d ReferenceLine(m_CurrentReferenceLine);
	ReferenceLine.SetEndPoint(ReferenceLine.ProjToEndPt(m_DuctSeamSize));
	ReferenceLine.GetParallels(section.Width(), m_CenterLineEccentricity, LeftLine, RightLine);

    auto Section {new EoDbGroup};
	GetDocument()->AddWorkLayerGroup(Section);

	GenerateEndCap(LeftLine.startPoint(), RightLine.startPoint(), section, Section);

	OdDbLinePtr Line;
	
    Line = EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), RightLine.endPoint());
    Line->setColorIndex(ColorIndex);
    Line->setLinetype(Linetype);
    Section->AddTail(EoDbLine::Create(Line));

    Line = EoDbLine::Create(BlockTableRecord, RightLine.endPoint(), LeftLine.endPoint());
    Line->setColorIndex(ColorIndex);
    Line->setLinetype(Linetype);
    Section->AddTail(EoDbLine::Create(Line));
	
    Line = EoDbLine::Create(BlockTableRecord, LeftLine.startPoint(), LeftLine.endPoint());
    Line->setColorIndex(ColorIndex);
    Line->setLinetype(Linetype);
    Section->AddTail(EoDbLine::Create(Line));

	m_CurrentReferenceLine.SetStartPoint(ReferenceLine.endPoint());
	m_CurrentReferenceLine.GetParallels(section.Width(), m_CenterLineEccentricity, LeftLine, RightLine);

	OdGePoint3d EndPoint;
	if (justification == Right) {
		RightLine.ProjPtFrom_xy(m_DuctTapSize, - m_DuctTapSize, EndPoint);
		RightLine.SetEndPoint(EndPoint);
	}
	else {
		LeftLine.ProjPtFrom_xy(m_DuctTapSize, m_DuctTapSize, EndPoint);
		LeftLine.SetEndPoint(EndPoint);

	}
	Line = EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), RightLine.endPoint());
    Line->setColorIndex(ColorIndex);
    Line->setLinetype(Linetype);
    Section->AddTail(EoDbLine::Create(Line));

    Line = EoDbLine::Create(BlockTableRecord, LeftLine.endPoint(), LeftLine.startPoint());
    Line->setColorIndex(ColorIndex);
    Line->setLinetype(Linetype);
    Section->AddTail(EoDbLine::Create(Line));

	if (m_GenerateTurningVanes) {
        const auto BeginPoint {((justification == Left) ? RightLine : LeftLine).ProjToBegPt(-m_DuctTapSize / 3.)};
        const auto EndPoint {m_CurrentReferenceLine.ProjToBegPt(-m_DuctTapSize / 2.)};

        const auto ActiveViewPlaneNormal {GetActiveView()->CameraDirection()};

        auto Circle {EoDbEllipse::CreateCircle(BlockTableRecord, BeginPoint, ActiveViewPlaneNormal, .01)};
        Circle->setColorIndex(1);
        Circle->setLinetype(L"Continuous");
        Section->AddTail(EoDbEllipse::Create(Circle));

        Line = EoDbLine::Create(BlockTableRecord, BeginPoint, EndPoint);
		Line->setColorIndex(1);
		Line->setLinetype(Linetype);
		Section->AddTail(EoDbLine::Create(Line));
	}
	GetDocument()->UpdateGroupInAllViews(kGroupSafe, Section);
	return true;
}

void AeSysView::GenerateTransition(EoGeLineSeg3d& referenceLine, double eccentricity, EJust justification, double slope, Section previousSection, Section currentSection, EoDbGroup* group) {
	const double ReferenceLength = referenceLine.length();
	if (ReferenceLength <= FLT_EPSILON) return;

    OdDbBlockTableRecordPtr BlockTableRecord = Database()->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    const auto ColorIndex {pstate.ColorIndex()};
    const auto Linetype {EoDbPrimitive::LinetypeObjectFromIndex(pstate.LinetypeIndex())};

    const double WidthChange = currentSection.Width() - previousSection.Width();
	double TransitionLength = LengthOfTransition(justification, slope, previousSection, currentSection);
	TransitionLength = EoMin(TransitionLength, ReferenceLength);

	EoGeLineSeg3d LeftLine;
	EoGeLineSeg3d RightLine;
	referenceLine.GetParallels(previousSection.Width(), eccentricity, LeftLine, RightLine);

	OdGePoint3d EndPoint;
	if (justification == Center) {
		LeftLine.ProjPtFrom_xy(TransitionLength, WidthChange * .5, EndPoint);
		LeftLine.SetEndPoint(EndPoint);
		RightLine.ProjPtFrom_xy(TransitionLength, - WidthChange * .5, EndPoint);
		RightLine.SetEndPoint(EndPoint);
	}
	else if (justification == Right) {
		RightLine.ProjPtFrom_xy(TransitionLength, - WidthChange, EndPoint);
		RightLine.SetEndPoint(EndPoint);
	}
	else {
		LeftLine.ProjPtFrom_xy(TransitionLength, WidthChange, EndPoint);
		LeftLine.SetEndPoint(EndPoint);
	}
	GenerateEndCap(LeftLine.startPoint(), RightLine.startPoint(), previousSection, group);

    auto Line {EoDbLine::Create(BlockTableRecord, RightLine.startPoint(), RightLine.endPoint())};
    Line->setColorIndex(ColorIndex);
    Line->setLinetype(Linetype);
    group->AddTail(EoDbLine::Create(Line));

    GenerateEndCap(RightLine.endPoint(), LeftLine.endPoint(), currentSection, group);

    Line = EoDbLine::Create(BlockTableRecord, LeftLine.endPoint(), LeftLine.startPoint());
    Line->setColorIndex(ColorIndex);
    Line->setLinetype(Linetype);
    group->AddTail(EoDbLine::Create(Line));
}
void AeSysView::SetDuctOptions(Section& section) {
	const AeSysApp::Units Units = theApp.GetUnits();
	theApp.SetUnits(max(Units, AeSysApp::kInches));

	EoDlgLowPressureDuctOptions dlg(this);

	dlg.m_Width = section.Width();
	dlg.m_Depth = section.Depth();
	dlg.m_RadiusFactor = m_InsideRadiusFactor;
	dlg.m_Justification = m_DuctJustification;
	dlg.m_GenerateVanes = m_GenerateTurningVanes;
	dlg.m_BeginWithTransition = m_BeginWithTransition;
	if (dlg.DoModal() == IDOK) {
		section.SetWidth(dlg.m_Width);
		section.SetDepth(dlg.m_Depth);
		m_InsideRadiusFactor = dlg.m_RadiusFactor;
		m_DuctJustification = EJust(dlg.m_Justification);
		m_GenerateTurningVanes = dlg.m_GenerateVanes;
		m_BeginWithTransition = dlg.m_BeginWithTransition;
	}
	theApp.SetUnits(Units);
}
double AeSysView::LengthOfTransition(EJust justification, double slope, Section previousSection, Section currentSection) noexcept {
	const double WidthChange = currentSection.Width() - previousSection.Width();
	const double DepthChange = currentSection.Depth() - previousSection.Depth();

	double Length = EoMax(fabs(WidthChange), fabs(DepthChange)) * slope;
	if (justification == Center) {
		Length *= .5;
	}
	return (Length);
}
bool AeSysView::Find2LinesUsingLineEndpoints(EoDbLine* testLinePrimitive, double angularTolerance, EoGeLineSeg3d& leftLine, EoGeLineSeg3d& rightLine) {
	EoGeLineSeg3d Line;

	EoDbLine* LeftLinePrimitive = 0;
	EoDbLine* RightLinePrimitive = 0;
	int DirectedRelationship = 0;

	EoGeLineSeg3d TestLine;
	testLinePrimitive->GetLine(TestLine);

	const double TestLineAngle = fmod(TestLine.AngleFromXAxis_xy(), PI);

	POSITION GroupPosition = GetLastGroupPosition();
	while (GroupPosition != 0) {
		EoDbGroup* Group = GetPreviousGroup(GroupPosition);

		POSITION PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != 0) {
			EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);
			if (Primitive == testLinePrimitive || !Primitive->Is(kLinePrimitive))
				continue;

			EoDbLine* LinePrimitive = static_cast<EoDbLine*>(Primitive);
			LinePrimitive->GetLine(Line);
			if (Line.startPoint() == TestLine.startPoint() || Line.startPoint() == TestLine.endPoint()) { // Exchange points
				const OdGePoint3d Point = Line.startPoint();
				Line.SetStartPoint(Line.endPoint());
				Line.SetEndPoint(Point);
			}
			else if (Line.endPoint() != TestLine.startPoint() && Line.endPoint() != TestLine.endPoint()) { //	No endpoint coincides with one of the test line endpoints
				continue;
			}
			const double LineAngle = fmod(Line.AngleFromXAxis_xy(), PI);
			if (fabs(fabs(TestLineAngle - LineAngle) - HALF_PI) <= angularTolerance) {
				if (LeftLinePrimitive == 0) { // No qualifiers yet
					DirectedRelationship = TestLine.DirectedRelationshipOf(Line.startPoint());
					LeftLinePrimitive = LinePrimitive;
					leftLine = Line;
				}
				else {
					if (DirectedRelationship == TestLine.DirectedRelationshipOf(Line.startPoint())) { // Both lines are on the same side of test line
						RightLinePrimitive = LinePrimitive;
						rightLine = Line;
						if (rightLine.DirectedRelationshipOf(leftLine.startPoint()) != 1) {
							RightLinePrimitive = LeftLinePrimitive;
							rightLine = leftLine;
							LeftLinePrimitive = LinePrimitive;
							leftLine = Line;
						}
						return true;
					}
				}
			}
		}
	}
	return false;
}
