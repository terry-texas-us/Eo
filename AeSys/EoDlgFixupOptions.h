#pragma once

// EoDlgFixupOptions dialog

class EoDlgFixupOptions : public CDialog {
	DECLARE_DYNAMIC(EoDlgFixupOptions)

public:
	EoDlgFixupOptions(CWnd* parent = nullptr);
	virtual ~EoDlgFixupOptions();

// Dialog Data
	enum { IDD = IDD_FIXUP_OPTIONS };

protected:
	void DoDataExchange(CDataExchange* pDX) final;

public:
	double m_AxisTolerance;
	double m_CornerSize;

protected:
	DECLARE_MESSAGE_MAP()
};
