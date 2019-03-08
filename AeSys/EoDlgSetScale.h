#pragma once

// EoDlgSetScale dialog

class EoDlgSetScale : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetScale)

public:
	EoDlgSetScale(CWnd* pParent = NULL);
	virtual ~EoDlgSetScale();

// Dialog Data
	enum { IDD = IDD_SET_SCALE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

public:
	double m_Scale;

protected:
	DECLARE_MESSAGE_MAP()
};
