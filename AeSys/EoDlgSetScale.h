#pragma once

class EoDlgSetScale final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetScale)
	EoDlgSetScale(CWnd* parent = nullptr);
	virtual ~EoDlgSetScale();

	enum { IDD = IDD_SET_SCALE };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
public:
	double scale {0.0};
DECLARE_MESSAGE_MAP()
};
