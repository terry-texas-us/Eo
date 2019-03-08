#pragma once

// EoDlgPipeOptions dialog

class EoDlgPipeOptions : public CDialog {
	DECLARE_DYNAMIC(EoDlgPipeOptions)

public:
	EoDlgPipeOptions(CWnd* pParent = NULL);
	virtual ~EoDlgPipeOptions();

// Dialog Data
	enum { IDD = IDD_PIPE_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();

public:
	double m_PipeTicSize;
	double m_PipeRiseDropRadius;

protected:
	DECLARE_MESSAGE_MAP()
};


