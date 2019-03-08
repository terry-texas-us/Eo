#pragma once

// EoDlgSetupCustomMouseCharacters dialog

class EoDlgSetupCustomMouseCharacters : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetupCustomMouseCharacters)

public:
	EoDlgSetupCustomMouseCharacters(CWnd* pParent = NULL);
	virtual ~EoDlgSetupCustomMouseCharacters();

	enum { IDD = IDD_MOUSEKEYS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

protected:
	DECLARE_MESSAGE_MAP()
};


