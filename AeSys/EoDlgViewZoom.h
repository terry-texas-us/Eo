#pragma once

// EoDlgViewZoom dialog

class EoDlgViewZoom : public CDialog {
	DECLARE_DYNAMIC(EoDlgViewZoom)

public:
	EoDlgViewZoom(CWnd* pParent = NULL);
	virtual ~EoDlgViewZoom();

// Dialog Data
	enum { IDD = IDD_VIEW_ZOOM };

	double m_ZoomFactor;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

public:

protected:
	DECLARE_MESSAGE_MAP()
};
