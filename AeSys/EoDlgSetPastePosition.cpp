#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysDoc.h"

#include "EoDlgSetPastePosition.h"

// EoDlgSetPastePosition dialog

IMPLEMENT_DYNAMIC(EoDlgSetPastePosition, CDialog)

BEGIN_MESSAGE_MAP(EoDlgSetPastePosition, CDialog)
END_MESSAGE_MAP()

EoDlgSetPastePosition::EoDlgSetPastePosition(CWnd* pParent /*=NULL*/) :
	CDialog(EoDlgSetPastePosition::IDD, pParent) {
}
EoDlgSetPastePosition::~EoDlgSetPastePosition() {
}
void EoDlgSetPastePosition::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}
void EoDlgSetPastePosition::OnOK() {
	AeSysDoc::GetDoc()->SetTrapPivotPoint(theApp.GetCursorPosition());

	CDialog::OnOK();
}
