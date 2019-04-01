#pragma once

// EoDlgSetUnitsAndPrecision dialog

class EoDlgSetUnitsAndPrecision : public CDialog {
	DECLARE_DYNAMIC(EoDlgSetUnitsAndPrecision)

public:
	EoDlgSetUnitsAndPrecision(CWnd* parent = NULL);
	virtual ~EoDlgSetUnitsAndPrecision();

// Dialog Data
	enum { IDD = IDD_UNITS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
public:
	CListBox m_MetricUnitsListBoxControl;
	AeSysApp::Units m_Units;
	int m_Precision;

	afx_msg void OnBnClickedMetric();

protected:
	DECLARE_MESSAGE_MAP()
};


