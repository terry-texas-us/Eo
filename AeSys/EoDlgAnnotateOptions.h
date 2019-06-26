#pragma once

class EoDlgAnnotateOptions : public CDialog {
DECLARE_DYNAMIC(EoDlgAnnotateOptions)
	EoDlgAnnotateOptions(CWnd* parent = nullptr);
	EoDlgAnnotateOptions(AeSysView* view, CWnd* parent = nullptr);
	~EoDlgAnnotateOptions() = default;

	enum { IDD = IDD_ANNOTATE_OPTIONS };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
	AeSysView* m_ActiveView {nullptr};
	CComboBox m_EndItemTypeComboBox;

  public:
	double gapSpaceFactor {0.0};
	double circleRadius {0.0};
	double endItemSize {0.0};
	double bubbleRadius {0.0};
	int numberOfSides {0};
	CString defaultText {L""};
protected:
DECLARE_MESSAGE_MAP()
};
