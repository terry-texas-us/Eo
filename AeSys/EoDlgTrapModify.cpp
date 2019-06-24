#include "stdafx.h"
#include "AeSysDoc.h"
#include "PrimState.h"
#include "EoDbHatch.h"
#include "EoDlgTrapModify.h"

// EoDlgTrapModify dialog
IMPLEMENT_DYNAMIC(EoDlgTrapModify, CDialog)

BEGIN_MESSAGE_MAP(EoDlgTrapModify, CDialog)
END_MESSAGE_MAP()

EoDlgTrapModify::EoDlgTrapModify(CWnd* parent) noexcept
	: CDialog(IDD, parent)
	, m_Document(nullptr) {
}

EoDlgTrapModify::EoDlgTrapModify(AeSysDoc* document, CWnd* parent)
	: CDialog(IDD, parent)
	, m_Document(document) {
}

EoDlgTrapModify::~EoDlgTrapModify() {
}

void EoDlgTrapModify::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

void EoDlgTrapModify::OnOK() {
	if (IsDlgButtonChecked(IDC_MOD_PEN)) {
		m_Document->ModifyTrappedGroupsColorIndex(g_PrimitiveState.ColorIndex());
	}
	if (IsDlgButtonChecked(IDC_MOD_LINE)) {
		m_Document->ModifyTrappedGroupsLinetypeIndex(g_PrimitiveState.LinetypeIndex());
	}
	if (IsDlgButtonChecked(IDC_MOD_FILL)) {
		ModifyPolygons();
	}
	auto CharacterCellDefinition {g_PrimitiveState.CharacterCellDefinition()};
	auto FontDefinition {g_PrimitiveState.FontDefinition()};
	if (IsDlgButtonChecked(IDC_MOD_NOTE)) {
		m_Document->ModifyTrappedGroupsNoteAttributes(FontDefinition, CharacterCellDefinition, TM_TEXT_ALL);
	} else if (IsDlgButtonChecked(IDC_FONT)) {
		m_Document->ModifyTrappedGroupsNoteAttributes(FontDefinition, CharacterCellDefinition, TM_TEXT_FONT);
	} else if (IsDlgButtonChecked(IDC_HEIGHT)) {
		m_Document->ModifyTrappedGroupsNoteAttributes(FontDefinition, CharacterCellDefinition, TM_TEXT_HEIGHT);
	}
	CDialog::OnOK();
}

void EoDlgTrapModify::ModifyPolygons() {
	auto Position {m_Document->GetFirstTrappedGroupPosition()};
	while (Position != nullptr) {
		const auto Group {m_Document->GetNextTrappedGroup(Position)};
		auto PrimitivePosition {Group->GetHeadPosition()};
		while (PrimitivePosition != nullptr) {
			const auto Primitive {Group->GetNext(PrimitivePosition)};
			if (Primitive->IsKindOf(RUNTIME_CLASS(EoDbHatch))) {
				auto pPolygon {dynamic_cast<EoDbHatch*>(Primitive)};
				pPolygon->SetInteriorStyle(g_PrimitiveState.HatchInteriorStyle());
				pPolygon->SetInteriorStyleIndex2(g_PrimitiveState.HatchInteriorStyleIndex());
				pPolygon->SetHatchReferenceAxes(EoDbHatch::sm_PatternAngle, EoDbHatch::sm_PatternScaleX, EoDbHatch::sm_PatternScaleY);
			}
		}
	}
}
