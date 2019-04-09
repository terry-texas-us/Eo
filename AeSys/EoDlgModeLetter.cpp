#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"
#include "AeSysView.h"

#include "EoDlgModeLetter.h"

// EoDlgModeLetter dialog

IMPLEMENT_DYNAMIC(EoDlgModeLetter, CDialog)

BEGIN_MESSAGE_MAP(EoDlgModeLetter, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

OdGePoint3d EoDlgModeLetter::m_Point = OdGePoint3d::kOrigin;

EoDlgModeLetter::EoDlgModeLetter(CWnd* parent) 
    : CDialog(EoDlgModeLetter::IDD, parent) {
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

	const EoDbCharacterCellDefinition CharacterCellDefinition = pstate.CharacterCellDefinition();
	EoGeReferenceSystem ReferenceSystem(m_Point, AeSysView::GetActiveView(), CharacterCellDefinition);

	EoDbFontDefinition FontDefinition = pstate.FontDefinition();

	if (m_TextEditControl.GetWindowTextLengthW() != 0) {
        CString TextEditControl;
        m_TextEditControl.GetWindowTextW(TextEditControl);
        m_TextEditControl.SetWindowTextW(L"");

        EoDbText* TextPrimitive;

        const int HardLineBreakPosition = TextEditControl.Find(L"\r\n");
        if (HardLineBreakPosition == -1) { // single line text
            OdDbTextPtr Text = EoDbText::Create0(Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite));
            Text->setPosition(ReferenceSystem.Origin());
            Text->setTextString((LPCWSTR) TextEditControl);
            Text->setHeight(ReferenceSystem.YDirection().length());
            Text->setRotation(ReferenceSystem.Rotation());
            Text->setAlignmentPoint(Text->position());

            TextPrimitive = EoDbText::Create(Text);
        } else {
            TextEditControl.Replace(L"\r\n", L"\\P");
            OdDbMTextPtr MText = EoDbText::Create(Database, Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite), TextEditControl.GetBuffer());
            MText->setLocation(ReferenceSystem.Origin());
            MText->setContents((LPCWSTR) TextEditControl);
            MText->setTextHeight(ReferenceSystem.YDirection().length());
            MText->setRotation(ReferenceSystem.Rotation());

            TextPrimitive = EoDbText::Create(MText);
        }
      	EoDbGroup* Group = new EoDbGroup;
		Group->AddTail(TextPrimitive);
		Document->AddWorkLayerGroup(Group);
		Document->UpdateGroupInAllViews(kGroupSafe, Group);
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
