#pragma once

class EoDlgSetPastePosition final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetPastePosition)
	EoDlgSetPastePosition(CWnd* parent = nullptr);
	virtual ~EoDlgSetPastePosition();

	enum { IDD = IDD_PASTE_POSITION };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	void OnOK() final;
DECLARE_MESSAGE_MAP()
};
