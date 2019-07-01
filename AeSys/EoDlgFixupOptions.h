#pragma once
class EoDlgFixupOptions final : public CDialog {
DECLARE_DYNAMIC(EoDlgFixupOptions)

	explicit EoDlgFixupOptions(CWnd* parent = nullptr);

	enum { IDD = IDD_FIXUP_OPTIONS };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

public:
	double axisTolerance {0.0};
	double cornerSize {0.0};
DECLARE_MESSAGE_MAP()
};
