#pragma once

// EoDlgFixupOptions dialog

class EoDlgFixupOptions : public CDialog {
	DECLARE_DYNAMIC(EoDlgFixupOptions)

public:
	EoDlgFixupOptions(CWnd* pParent = NULL);
	virtual ~EoDlgFixupOptions();

// Dialog Data
	enum { IDD = IDD_FIXUP_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

public:
	double m_FixupAxisTolerance;
	double m_FixupModeCornerSize;

protected:
	DECLARE_MESSAGE_MAP()
};
