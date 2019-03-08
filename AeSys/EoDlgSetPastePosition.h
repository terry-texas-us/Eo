#pragma once

// EoDlgSetPastePosition dialog

class EoDlgSetPastePosition : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetPastePosition)

public:
	EoDlgSetPastePosition(CWnd* pParent = NULL);
	virtual ~EoDlgSetPastePosition();

// Dialog Data
	enum { IDD = IDD_PASTE_POSITION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();

public:

protected:
	DECLARE_MESSAGE_MAP()
};
