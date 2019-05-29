#pragma once

// EoDlgPipeOptions dialog

class EoDlgPipeOptions : public CDialog {
	DECLARE_DYNAMIC(EoDlgPipeOptions)

public:
	EoDlgPipeOptions(CWnd* parent = nullptr);
	virtual ~EoDlgPipeOptions();

// Dialog Data
	enum { IDD = IDD_PIPE_OPTIONS };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;

public:
	double m_PipeTicSize;
	double m_PipeRiseDropRadius;

protected:
	DECLARE_MESSAGE_MAP()
};


