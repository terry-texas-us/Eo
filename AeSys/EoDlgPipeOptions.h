#pragma once

// EoDlgPipeOptions dialog

class EoDlgPipeOptions : public CDialog {
	DECLARE_DYNAMIC(EoDlgPipeOptions)

public:
	EoDlgPipeOptions(CWnd* parent = NULL);
	virtual ~EoDlgPipeOptions();

// Dialog Data
	enum { IDD = IDD_PIPE_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

public:
	double m_PipeTicSize;
	double m_PipeRiseDropRadius;

protected:
	DECLARE_MESSAGE_MAP()
};


