#include "stdafx.h"
#include "AeSys.h"
#include "AeSysDoc.h"
#include "EoDlgSetPastePosition.h"

// EoDlgSetPastePosition dialog
IMPLEMENT_DYNAMIC(EoDlgSetPastePosition, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetPastePosition, CDialog)
END_MESSAGE_MAP()

EoDlgSetPastePosition::EoDlgSetPastePosition(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgSetPastePosition::~EoDlgSetPastePosition() {
}

void EoDlgSetPastePosition::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

void EoDlgSetPastePosition::OnOK() {
	AeSysDoc::GetDoc()->SetTrapPivotPoint(AeSys::GetCursorPosition());
	CDialog::OnOK();
}
