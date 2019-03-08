#include "stdafx.h"
#include "AeSysDoc.h"

#include "EoDlgTrapModify.h"

// EoDlgTrapModify dialog

IMPLEMENT_DYNAMIC(EoDlgTrapModify, CDialog)

BEGIN_MESSAGE_MAP(EoDlgTrapModify, CDialog)
END_MESSAGE_MAP()

EoDlgTrapModify::EoDlgTrapModify(CWnd* pParent /*=NULL*/) :
	CDialog(EoDlgTrapModify::IDD, pParent) {
}
EoDlgTrapModify::EoDlgTrapModify(AeSysDoc* document, CWnd* pParent /*=NULL*/) :
	CDialog(EoDlgTrapModify::IDD, pParent), m_Document(document) {
}
EoDlgTrapModify::~EoDlgTrapModify() {
}
void EoDlgTrapModify::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}
void EoDlgTrapModify::OnOK() {
	if (IsDlgButtonChecked(IDC_MOD_PEN)) {
		m_Document->ModifyTrappedGroupsColorIndex(pstate.ColorIndex());
	}
	if (IsDlgButtonChecked(IDC_MOD_LINE)) {
		m_Document->ModifyTrappedGroupsLinetypeIndex(pstate.LinetypeIndex());
	}
	if (IsDlgButtonChecked(IDC_MOD_FILL)) {
		ModifyPolygons();
	}
	EoDbCharacterCellDefinition CharacterCellDefinition = pstate.CharacterCellDefinition();

	EoDbFontDefinition FontDefinition = pstate.FontDefinition();

	if (IsDlgButtonChecked(IDC_MOD_NOTE)) {
		m_Document->ModifyTrappedGroupsNoteAttributes(FontDefinition, CharacterCellDefinition, TM_TEXT_ALL);
	}
	else if (IsDlgButtonChecked(IDC_FONT)) {
		m_Document->ModifyTrappedGroupsNoteAttributes(FontDefinition, CharacterCellDefinition, TM_TEXT_FONT);
	}
	else if (IsDlgButtonChecked(IDC_HEIGHT)) {
		m_Document->ModifyTrappedGroupsNoteAttributes(FontDefinition, CharacterCellDefinition, TM_TEXT_HEIGHT);
	}

	CDialog::OnOK();
}
void EoDlgTrapModify::ModifyPolygons(void) {
	POSITION Position = m_Document->GetFirstTrappedGroupPosition();
	while (Position != 0) {
		EoDbGroup* Group = m_Document->GetNextTrappedGroup(Position);

		POSITION PrimitivePosition = Group->GetHeadPosition();
		while (PrimitivePosition != 0) {
			EoDbPrimitive* Primitive = Group->GetNext(PrimitivePosition);

			if (Primitive->Is(EoDb::kHatchPrimitive)) {
				EoDbHatch* pPolygon = static_cast<EoDbHatch*>(Primitive);
				pPolygon->SetInteriorStyle(pstate.HatchInteriorStyle());
				pPolygon->SetInteriorStyleIndex(pstate.HatchInteriorStyleIndex());
				pPolygon->SetHatRefVecs(EoDbHatch::sm_PatternAngle, EoDbHatch::sm_PatternScaleX, EoDbHatch::sm_PatternScaleY);
			}
		}
	}
}
