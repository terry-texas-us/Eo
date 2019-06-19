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

BOOL EoCtrlFindComboBox::NotifyCommand(int notifyCode) {
	TRACE1("EoCtrlFindComboBox::NotifyCommand(%i)\n", notifyCode);
	auto CommandProcessed {CMFCToolBarComboBoxButton::NotifyCommand(notifyCode)};

	switch (notifyCode) {
	case CBN_SELCHANGE: // notifyCode 1
	case CBN_DBLCLK: // notifyCode 2
		break;
	case CBN_SETFOCUS: // notifyCode 3
		m_HasFocus = TRUE;
		CommandProcessed = TRUE;
		break;
	case CBN_KILLFOCUS: // notifyCode 4
		m_HasFocus = FALSE;
		CommandProcessed = TRUE;
		break;
	case CBN_EDITCHANGE: // notifyCode 5
	case CBN_EDITUPDATE: // notifyCode 6
	case CBN_DROPDOWN: // notifyCode 7
	case CBN_CLOSEUP: // notifyCode 8
	case CBN_SELENDOK: // notifyCode 9
	case CBN_SELENDCANCEL: // notifyCode 10
		break;
	}
	return CommandProcessed;
}