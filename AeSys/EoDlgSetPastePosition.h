#pragma once

// EoDlgSetPastePosition dialog

class EoDlgSetPastePosition : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetPastePosition)

	EoDlgSetPastePosition(CWnd* parent = nullptr);
	virtual ~EoDlgSetPastePosition();

	// Dialog Data
	enum { IDD = IDD_PASTE_POSITION };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	void OnOK() final;

	DECLARE_MESSAGE_MAP()
};
