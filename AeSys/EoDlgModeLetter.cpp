#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"

#include "EoDlgModeLetter.h"

// EoDlgModeLetter dialog

IMPLEMENT_DYNAMIC(EoDlgModeLetter, CDialog)

BEGIN_MESSAGE_MAP(EoDlgModeLetter, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

OdGePoint3d EoDlgModeLetter::m_Point = OdGePoint3d::kOrigin;

EoDlgModeLetter::EoDlgModeLetter(CWnd* pParent /*=NULL*/) :
	CDialog(EoDlgModeLetter::IDD, pParent) {
}
EoDlgModeLetter::~EoDlgModeLetter() {
}
void EoDlgModeLetter::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TEXT, m_TextEditControl);
}
BOOL EoDlgModeLetter::OnInitDialog() {
	CDialog::OnInitDialog();

	m_Point = theApp.GetCursorPosition();

	return TRUE;
}
void EoDlgModeLetter::OnOK() {
	AeSysDoc* Document = AeSysDoc::GetDoc();
	OdDbDatabasePtr Database = Document->m_DatabasePtr;

	EoDbCharacterCellDefinition CharacterCellDefinition = pstate.CharacterCellDefinition();
	EoGeReferenceSystem ReferenceSystem(m_Point, CharacterCellDefinition);

	EoDbFontDefinition FontDefinition = pstate.FontDefinition();

	if (m_TextEditControl.GetWindowTextLengthW() != 0) {
		CString Text;
		m_TextEditControl.GetWindowTextW(Text);
		m_TextEditControl.SetWindowTextW(L"");

		EoDbText* TextPrimitive = EoDbText::Create(Database);
		TextPrimitive->SetTo(FontDefinition, ReferenceSystem, Text);
		EoDbGroup* Group = new EoDbGroup;
		Group->AddTail(TextPrimitive);
		Document->AddWorkLayerGroup(Group);
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	}
	m_Point = text_GetNewLinePos(FontDefinition, ReferenceSystem, 1., 0);
	m_TextEditControl.SetFocus();

	CDialog::OnOK();
}
void EoDlgModeLetter::OnSize(UINT type, int cx, int cy) {
	CDialog::OnSize(type, cx, cy);

	if (::IsWindow(m_TextEditControl.GetSafeHwnd())) {
		m_TextEditControl.MoveWindow(0, 0, cx, cy, TRUE);
	}
}
