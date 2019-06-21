#pragma once

// EoDlgViewZoom dialog
class EoDlgViewZoom : public CDialog {
DECLARE_DYNAMIC(EoDlgViewZoom)
	EoDlgViewZoom(CWnd* parent = nullptr);
	virtual ~EoDlgViewZoom();

	// Dialog Data
	enum { IDD = IDD_VIEW_ZOOM };

	double m_ZoomFactor;
protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
DECLARE_MESSAGE_MAP()
};
