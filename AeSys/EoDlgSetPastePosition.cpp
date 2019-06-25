#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "EoDlgSetPastePosition.h"

IMPLEMENT_DYNAMIC(EoDlgSetPastePosition, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetPastePosition, CDialog)
END_MESSAGE_MAP()

EoDlgSetPastePosition::EoDlgSetPastePosition(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgSetPastePosition::~EoDlgSetPastePosition() = default;

void EoDlgSetPastePosition::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
}

void EoDlgSetPastePosition::OnOK() {
	AeSysDoc::GetDoc()->SetTrapPivotPoint(AeSys::GetCursorPosition());
	CDialog::OnOK();
}
