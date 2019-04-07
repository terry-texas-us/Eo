#pragma once

// EoDlgSetupCustomMouseCharacters dialog

class EoDlgSetupCustomMouseCharacters : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetupCustomMouseCharacters)

public:
	EoDlgSetupCustomMouseCharacters(CWnd* parent = NULL);
	virtual ~EoDlgSetupCustomMouseCharacters();

	enum { IDD = IDD_MOUSEKEYS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

protected:
	DECLARE_MESSAGE_MAP()
};


