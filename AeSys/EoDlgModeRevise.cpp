#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoDb.h"
#include "EoDlgModeRevise.h"

// EoDlgModeRevise dialog
/// <remarks>
///Text related attributes for all notes generated will be same as those of the text last picked.
///Upon exit attributes restored to their entry values.
/// </remarks>

IMPLEMENT_DYNAMIC(EoDlgModeRevise, CDialog)

BEGIN_MESSAGE_MAP(EoDlgModeRevise, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

EoDbFontDefinition EoDlgModeRevise::sm_FontDefinition;
EoGeReferenceSystem EoDlgModeRevise::sm_ReferenceSystem;
EoDbText* EoDlgModeRevise::sm_TextPrimitive;

EoDlgModeRevise::EoDlgModeRevise(CWnd* parent) 
    : CDialog(IDD, parent) {
}

EoDlgModeRevise::~EoDlgModeRevise() {
}

void EoDlgModeRevise::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TEXT, m_TextEditControl);
}
BOOL EoDlgModeRevise::OnInitDialog() {
	CDialog::OnInitDialog();

	sm_TextPrimitive = AeSysView::GetActiveView()->SelectTextUsingPoint(theApp.GetCursorPosition());
	if (sm_TextPrimitive != nullptr) {
		sm_FontDefinition = sm_TextPrimitive->FontDefinition();
		sm_ReferenceSystem = sm_TextPrimitive->ReferenceSystem();
		m_TextEditControl.SetWindowTextW(sm_TextPrimitive->Text());
	} else {
		EndDialog(TRUE);
	}
	return TRUE;
}
void EoDlgModeRevise::OnOK() {
	auto Document {AeSysDoc::GetDoc()};
	auto Database {Document->m_DatabasePtr};

	CString TextString;
	m_TextEditControl.GetWindowTextW(TextString);

	if (sm_TextPrimitive != nullptr) {
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, sm_TextPrimitive);
		sm_TextPrimitive->SetText(TextString);
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, sm_TextPrimitive);
	} else {
		OdGeVector3d PlaneNormal;
		sm_ReferenceSystem.GetUnitNormal(PlaneNormal);
		OdDbBlockTableRecordPtr BlockTableRecord {Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
		auto Text {EoDbText::Create(BlockTableRecord, sm_ReferenceSystem.Origin(), static_cast<const wchar_t*>(TextString))};

		Text->setNormal(PlaneNormal);
		Text->setRotation(sm_ReferenceSystem.Rotation());
		Text->setHeight(sm_ReferenceSystem.YDirection().length());
		Text->setAlignmentPoint(sm_ReferenceSystem.Origin());
		Text->setHorizontalMode(EoDbText::ConvertHorizontalMode(sm_FontDefinition.HorizontalAlignment()));
		Text->setVerticalMode(EoDbText::ConvertVerticalMode(sm_FontDefinition.VerticalAlignment()));

		auto Group {new EoDbGroup};
		Group->AddTail(EoDbText::Create(Text));

		Document->AddWorkLayerGroup(Group);
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	}
	sm_ReferenceSystem.SetOrigin(text_GetNewLinePos(sm_FontDefinition, sm_ReferenceSystem, 1.0, 0));

	sm_TextPrimitive = AeSysView::GetActiveView()->SelectTextUsingPoint(sm_ReferenceSystem.Origin());

	if (sm_TextPrimitive != nullptr) {
		sm_FontDefinition = sm_TextPrimitive->FontDefinition();
		sm_ReferenceSystem = sm_TextPrimitive->ReferenceSystem();
		m_TextEditControl.SetWindowTextW(sm_TextPrimitive->Text());
	}
	else {
		m_TextEditControl.SetWindowTextW(L"");
	}
	m_TextEditControl.SetFocus();

	CDialog::OnOK();
}

void EoDlgModeRevise::OnSize(unsigned type, int cx, int cy) {
	CDialog::OnSize(type, cx, cy);

	if (IsWindow(m_TextEditControl.GetSafeHwnd())) {
		m_TextEditControl.MoveWindow(0, 0, cx, cy, TRUE);
	}
}
