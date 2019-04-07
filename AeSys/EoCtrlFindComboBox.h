#pragma once

class EoCtrlFindComboBox : public CMFCToolBarComboBoxButton {
	DECLARE_SERIAL(EoCtrlFindComboBox)

public:
	EoCtrlFindComboBox() : CMFCToolBarComboBoxButton(ID_EDIT_FIND_COMBO, GetCmdMgr()->GetCmdImage(ID_EDIT_FIND), CBS_DROPDOWN) {
	}

protected:
	static BOOL m_HasFocus;

public:
	static BOOL HasFocus() noexcept {
		return m_HasFocus;
	}
protected:
	virtual BOOL NotifyCommand(int notifyCode) override;
};

