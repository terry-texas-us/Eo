#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "PrimState.h"
#include "EoDb.h"
#include "EoDlgModeLetter.h"

// EoDlgModeLetter dialog
IMPLEMENT_DYNAMIC(EoDlgModeLetter, CDialog)

BEGIN_MESSAGE_MAP(EoDlgModeLetter, CDialog)
		ON_WM_SIZE()
END_MESSAGE_MAP()
OdGePoint3d EoDlgModeLetter::m_Point = OdGePoint3d::kOrigin;

EoDlgModeLetter::EoDlgModeLetter(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgModeLetter::~EoDlgModeLetter() {
}

void EoDlgModeLetter::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TEXT, m_TextEditControl);
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
	if (m_TextEditControl.GetWindowTextLengthW() != 0) {
		CString TextEditControl;
		m_TextEditControl.GetWindowTextW(TextEditControl);
		m_TextEditControl.SetWindowTextW(L"");
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
	m_TextEditControl.SetFocus();
	CDialog::OnOK();
}

void EoDlgModeLetter::OnSize(const unsigned type, const int cx, const int cy) {
	CDialog::OnSize(type, cx, cy);
	if (IsWindow(m_TextEditControl.GetSafeHwnd())) {
		m_TextEditControl.MoveWindow(0, 0, cx, cy, TRUE);
	}
}
