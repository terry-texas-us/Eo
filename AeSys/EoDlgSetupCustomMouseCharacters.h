#pragma once

// EoDlgSetupCustomMouseCharacters dialog

class EoDlgSetupCustomMouseCharacters : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetupCustomMouseCharacters)

public:
	EoDlgSetupCustomMouseCharacters(CWnd* parent = nullptr);
	virtual ~EoDlgSetupCustomMouseCharacters();

	enum { IDD = IDD_MOUSEKEYS };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;

protected:
	DECLARE_MESSAGE_MAP()
};


