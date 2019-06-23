#include "stdafx.h"
#include "EoCtrlFindComboBox.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// EoCtrlFindComboBox
IMPLEMENT_SERIAL(EoCtrlFindComboBox, CMFCToolBarComboBoxButton, 1)

BOOL EoCtrlFindComboBox::m_HasFocus = FALSE;

BOOL EoCtrlFindComboBox::NotifyCommand(const int notifyCode) {
	TRACE1("EoCtrlFindComboBox::NotifyCommand(%i)\n", notifyCode);
	auto CommandProcessed {CMFCToolBarComboBoxButton::NotifyCommand(notifyCode)};
	switch (notifyCode) {
		case CBN_SELCHANGE: case CBN_DBLCLK:
			break;
		case CBN_SETFOCUS:
			m_HasFocus = TRUE;
			CommandProcessed = TRUE;
			break;
		case CBN_KILLFOCUS:
			m_HasFocus = FALSE;
			CommandProcessed = TRUE;
			break;
		case CBN_EDITCHANGE: case CBN_EDITUPDATE: case CBN_DROPDOWN: case CBN_CLOSEUP: case CBN_SELENDOK: case CBN_SELENDCANCEL:
			break;
		default: ;
	}
	return CommandProcessed;
}
