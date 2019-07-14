#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoDb.h"
#include "EoDlgModeLetter.h"
IMPLEMENT_DYNAMIC(EoDlgModeLetter, CDialog)
#pragma warning(push)
#pragma warning(disable : 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
BEGIN_MESSAGE_MAP(EoDlgModeLetter, CDialog)
		ON_WM_SIZE()
END_MESSAGE_MAP()
#pragma warning (pop)
OdGePoint3d EoDlgModeLetter::m_Point = OdGePoint3d::kOrigin;

EoDlgModeLetter::EoDlgModeLetter(CWnd* parent)
	: CDialog(IDD, parent) {}

EoDlgModeLetter::~EoDlgModeLetter() = default;

void EoDlgModeLetter::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_TEXT, textEditControl);
}

BOOL EoDlgModeLetter::OnInitDialog() {
	CDialog::OnInitDialog();
	m_Point = AeSys::GetCursorPosition();
	return TRUE;
}

void EoDlgModeLetter::OnOK() {
	auto Document {AeSysDoc::GetDoc()};
	auto Database {Document->m_DatabasePtr};
	const auto CharacterCellDefinition {g_PrimitiveState.CharacterCellDefinition()};
	EoGeReferenceSystem ReferenceSystem(m_Point, AeSysView::GetActiveView(), CharacterCellDefinition);
	auto FontDefinition {g_PrimitiveState.FontDefinition()};
	if (textEditControl.GetWindowTextLengthW() != 0) {
		CString TextEditControl;
		textEditControl.GetWindowTextW(TextEditControl);
		textEditControl.SetWindowTextW(L"");
		EoDbText* TextPrimitive;
		const auto HardLineBreakPosition {TextEditControl.Find(L"\r\n")};
		if (HardLineBreakPosition == -1) { // single line text
			OdDbBlockTableRecordPtr BlockTableRecord {Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
			auto Text {EoDbText::Create(BlockTableRecord, ReferenceSystem.Origin(), static_cast<const wchar_t*>(TextEditControl))};
			Text->setHeight(ReferenceSystem.YDirection().length());
			Text->setRotation(ReferenceSystem.Rotation());
			Text->setAlignmentPoint(Text->position());
			TextPrimitive = EoDbText::Create(Text);
		} else {
			TextEditControl.Replace(L"\r\n", L"\\P");
			OdDbBlockTableRecordPtr BlockTableRecord {Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
			auto MText {EoDbText::CreateM(BlockTableRecord, static_cast<const wchar_t*>(TextEditControl))};
			MText->setLocation(ReferenceSystem.Origin());
			MText->setTextHeight(ReferenceSystem.YDirection().length());
			MText->setRotation(ReferenceSystem.Rotation());
			TextPrimitive = EoDbText::Create(MText);
		}
		auto Group {new EoDbGroup};
		Group->AddTail(TextPrimitive);
		Document->AddWorkLayerGroup(Group);
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	}
	m_Point = text_GetNewLinePos(FontDefinition, ReferenceSystem, 1.0, 0);
	textEditControl.SetFocus();
	CDialog::OnOK();
}

void EoDlgModeLetter::OnSize(const unsigned type, const int cx, const int cy) {
	CDialog::OnSize(type, cx, cy);
	if (IsWindow(textEditControl.GetSafeHwnd()) != 0) {
		textEditControl.MoveWindow(0, 0, cx, cy, TRUE);
	}
}
