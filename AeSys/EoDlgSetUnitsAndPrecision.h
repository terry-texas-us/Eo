#pragma once

// EoDlgSetUnitsAndPrecision dialog
class EoDlgSetUnitsAndPrecision : public CDialog {
DECLARE_DYNAMIC(EoDlgSetUnitsAndPrecision)
	EoDlgSetUnitsAndPrecision(CWnd* parent = nullptr);
	virtual ~EoDlgSetUnitsAndPrecision();

	// Dialog Data
	enum { IDD = IDD_UNITS };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
public:
	CListBox m_MetricUnitsListBoxControl;
	AeSys::Units m_Units;
	int m_Precision;
	void OnBnClickedMetric();
protected:
DECLARE_MESSAGE_MAP()
};
