#pragma once

// EoDlgSetPastePosition dialog

class EoDlgSetPastePosition : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetPastePosition)

public:
	EoDlgSetPastePosition(CWnd* parent = NULL);
	virtual ~EoDlgSetPastePosition();

// Dialog Data
	enum { IDD = IDD_PASTE_POSITION };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	void OnOK() final;

public:

protected:
	DECLARE_MESSAGE_MAP()
};
