#pragma once

// EoDlgLineWeight dialog

class EoDlgLineWeight : public CDialog {
	DECLARE_DYNAMIC(EoDlgLineWeight)

public:
	EoDlgLineWeight(CWnd* parent = NULL);
	EoDlgLineWeight(int originalLineWeight, CWnd* parent = NULL);
	virtual ~EoDlgLineWeight();

// Dialog Data
	enum { IDD = IDD_LINEWEIGHT };

protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog(void) final;

private:
	int m_OriginalLineWeight;
public:
	CListBox m_LineWeightList;
	OdDb::LineWeight m_LineWeight;

	afx_msg void OnBnClickedOk();
	afx_msg void OnLbnDblclkListLineweight();

protected:
	DECLARE_MESSAGE_MAP()
};
