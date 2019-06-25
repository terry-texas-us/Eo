#pragma once
class EoDlgSetUnitsAndPrecision final : public CDialog {
DECLARE_DYNAMIC(EoDlgSetUnitsAndPrecision)
	EoDlgSetUnitsAndPrecision(CWnd* parent = nullptr);
	virtual ~EoDlgSetUnitsAndPrecision();

	enum { IDD = IDD_UNITS };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
	CListBox m_MetricUnitsListBoxControl;
public:
	AeSys::Units units {AeSys::kInches};
	int precision {8};
	void OnBnClickedMetric();
protected:
DECLARE_MESSAGE_MAP()
};
