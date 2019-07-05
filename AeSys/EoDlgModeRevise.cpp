#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "AeSysView.h"
#include "EoDb.h"
#include "EoDlgModeRevise.h"

/// <remarks>
///Text related attributes for all notes generated will be same as those of the text last picked.
///Upon exit attributes restored to their entry values.
/// </remarks>
IMPLEMENT_DYNAMIC(EoDlgModeRevise, CDialog)

BEGIN_MESSAGE_MAP(EoDlgModeRevise, CDialog)
		ON_WM_SIZE()
END_MESSAGE_MAP()
EoDbFontDefinition EoDlgModeRevise::m_FontDefinition;
EoGeReferenceSystem EoDlgModeRevise::m_ReferenceSystem;
EoDbText* EoDlgModeRevise::m_TextPrimitive;

EoDlgModeRevise::EoDlgModeRevise(CWnd* parent)
	: CDialog(IDD, parent) {
}

void EoDlgModeRevise::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_TEXT, textEditControl);
}

BOOL EoDlgModeRevise::OnInitDialog() {
	CDialog::OnInitDialog();
	m_TextPrimitive = AeSysView::GetActiveView()->SelectTextUsingPoint(AeSys::GetCursorPosition());
	if (m_TextPrimitive != nullptr) {
		m_FontDefinition = m_TextPrimitive->FontDefinition();
		m_ReferenceSystem = m_TextPrimitive->ReferenceSystem();
		textEditControl.SetWindowTextW(m_TextPrimitive->Text());
	} else {
		EndDialog(TRUE);
	}
	return TRUE;
}

void EoDlgModeRevise::OnOK() {
	auto Document {AeSysDoc::GetDoc()};
	auto Database {Document->m_DatabasePtr};
	CString TextString;
	textEditControl.GetWindowTextW(TextString);
	if (m_TextPrimitive != nullptr) {
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveEraseSafe, m_TextPrimitive);
		m_TextPrimitive->SetText(TextString);
		Document->UpdatePrimitiveInAllViews(EoDb::kPrimitiveSafe, m_TextPrimitive);
	} else {
		OdGeVector3d PlaneNormal;
		m_ReferenceSystem.GetUnitNormal(PlaneNormal);
		OdDbBlockTableRecordPtr BlockTableRecord {Database->getModelSpaceId().safeOpenObject(OdDb::kForWrite)};
		auto Text {EoDbText::Create(BlockTableRecord, m_ReferenceSystem.Origin(), static_cast<const wchar_t*>(TextString))};
		Text->setNormal(PlaneNormal);
		Text->setRotation(m_ReferenceSystem.Rotation());
		Text->setHeight(m_ReferenceSystem.YDirection().length());
		Text->setAlignmentPoint(m_ReferenceSystem.Origin());
		Text->setHorizontalMode(EoDbText::ConvertHorizontalMode(m_FontDefinition.HorizontalAlignment()));
		Text->setVerticalMode(EoDbText::ConvertVerticalMode(m_FontDefinition.VerticalAlignment()));
		auto Group {new EoDbGroup};
		Group->AddTail(EoDbText::Create(Text));
		Document->AddWorkLayerGroup(Group);
		Document->UpdateGroupInAllViews(EoDb::kGroupSafe, Group);
	}
	m_ReferenceSystem.SetOrigin(text_GetNewLinePos(m_FontDefinition, m_ReferenceSystem, 1.0, 0));
	m_TextPrimitive = AeSysView::GetActiveView()->SelectTextUsingPoint(m_ReferenceSystem.Origin());
	if (m_TextPrimitive != nullptr) {
		m_FontDefinition = m_TextPrimitive->FontDefinition();
		m_ReferenceSystem = m_TextPrimitive->ReferenceSystem();
		textEditControl.SetWindowTextW(m_TextPrimitive->Text());
	} else {
		textEditControl.SetWindowTextW(L"");
	}
	textEditControl.SetFocus();
	CDialog::OnOK();
}

void EoDlgModeRevise::OnSize(const unsigned type, const int cx, const int cy) {
	CDialog::OnSize(type, cx, cy);
	if (IsWindow(textEditControl.GetSafeHwnd()) != 0) {
		textEditControl.MoveWindow(0, 0, cx, cy, TRUE);
	}
}
