#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

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
    : CDialog(EoDlgModeRevise::IDD, parent) {
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
	if (sm_TextPrimitive != 0) {
		sm_FontDefinition = sm_TextPrimitive->FontDefinition();
		sm_ReferenceSystem = sm_TextPrimitive->ReferenceSystem();
		m_TextEditControl.SetWindowTextW(sm_TextPrimitive->Text());
	}
	else {
		EndDialog(TRUE);
	}
	return TRUE;
}
void EoDlgModeRevise::OnOK() {
	AeSysDoc* Document = AeSysDoc::GetDoc();
	OdDbDatabasePtr Database = Document->m_DatabasePtr;

	CString Text;
	m_TextEditControl.GetWindowTextW(Text);

	if (sm_TextPrimitive != 0) {
		Document->UpdatePrimitiveInAllViews(kPrimitiveEraseSafe, sm_TextPrimitive);
		sm_TextPrimitive->SetText(Text);
		Document->UpdatePrimitiveInAllViews(kPrimitiveSafe, sm_TextPrimitive);
	}
	else {
		EoDbText* TextPrimitive = EoDbText::Create1(Database);
		TextPrimitive->SetTo(sm_FontDefinition, sm_ReferenceSystem, Text);
		EoDbGroup* Group = new EoDbGroup;
		Group->AddTail(TextPrimitive);
		Document->AddWorkLayerGroup(Group);
		Document->UpdateGroupInAllViews(kGroupSafe, Group);
	}
	sm_ReferenceSystem.SetOrigin(text_GetNewLinePos(sm_FontDefinition, sm_ReferenceSystem, 1., 0));

	sm_TextPrimitive = AeSysView::GetActiveView()->SelectTextUsingPoint(sm_ReferenceSystem.Origin());
	if (sm_TextPrimitive != 0) {
		sm_FontDefinition = sm_TextPrimitive->FontDefinition();
		sm_ReferenceSystem = sm_TextPrimitive->ReferenceSystem();
		m_TextEditControl.SetWindowTextW(sm_TextPrimitive->Text());
	}
	else
		m_TextEditControl.SetWindowTextW(L"");

	m_TextEditControl.SetFocus();

	CDialog::OnOK();
}
void EoDlgModeRevise::OnSize(UINT type, int cx, int cy) {
	CDialog::OnSize(type, cx, cy);

	if (::IsWindow(m_TextEditControl.GetSafeHwnd())) {
		m_TextEditControl.MoveWindow(0, 0, cx, cy, TRUE);
	}
}
