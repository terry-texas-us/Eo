#pragma once

class EoDlgPipeOptions final : public CDialog {
DECLARE_DYNAMIC(EoDlgPipeOptions)
	EoDlgPipeOptions(CWnd* parent = nullptr);
	virtual ~EoDlgPipeOptions();

	enum { IDD = IDD_PIPE_OPTIONS };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
public:
	double pipeTicSize {0.0};
	double pipeRiseDropRadius {0.0};
protected:
DECLARE_MESSAGE_MAP()
};
