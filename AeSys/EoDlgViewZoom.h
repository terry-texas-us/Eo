#pragma once

// EoDlgViewZoom dialog

class EoDlgViewZoom : public CDialog {
	DECLARE_DYNAMIC(EoDlgViewZoom)

public:
	EoDlgViewZoom(CWnd* parent = NULL);
	virtual ~EoDlgViewZoom();

// Dialog Data
	enum { IDD = IDD_VIEW_ZOOM };

	double m_ZoomFactor;

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;

public:

protected:
	DECLARE_MESSAGE_MAP()
};
