#pragma once

class EoDlgFixupOptions final : public CDialog {
DECLARE_DYNAMIC(EoDlgFixupOptions)
	EoDlgFixupOptions(CWnd* parent = nullptr);
	virtual ~EoDlgFixupOptions();

	enum { IDD = IDD_FIXUP_OPTIONS };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
public:
	double axisTolerance;
	double cornerSize;
protected:
DECLARE_MESSAGE_MAP()
};
