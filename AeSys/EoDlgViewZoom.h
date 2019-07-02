#pragma once
class EoDlgViewZoom final : public CDialog {
DECLARE_DYNAMIC(EoDlgViewZoom)

	EoDlgViewZoom(CWnd* parent = nullptr);

	virtual ~EoDlgViewZoom();

	enum { IDD = IDD_VIEW_ZOOM };

	double m_ZoomFactor {1.0};
protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

DECLARE_MESSAGE_MAP()
};
