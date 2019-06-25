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
public:
	CComboBox m_EndItemTypeComboBox;
	double m_GapSpaceFactor {0.0};
	double m_CircleRadius {0.0};
	double m_EndItemSize {0.0};
	double m_BubbleRadius {0.0};
	int m_NumberOfSides {0};
	CString m_DefaultText {L""};
protected:
DECLARE_MESSAGE_MAP()
};
