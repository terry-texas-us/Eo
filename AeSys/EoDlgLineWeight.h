#pragma once
class EoDlgLineWeight final : public CDialog {
DECLARE_DYNAMIC(EoDlgLineWeight)

	EoDlgLineWeight(CWnd* parent = nullptr);

	EoDlgLineWeight(int originalLineWeight, CWnd* parent = nullptr);

	enum { IDD = IDD_LINEWEIGHT };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

private:
	int m_OriginalLineWeight {0};
public:
	CListBox lineWeightList;
	OdDb::LineWeight lineWeight {OdDb::LineWeight::kLnWt000};

	void OnBnClickedOk();

	void OnLbnDoubleClickListLineweight();

DECLARE_MESSAGE_MAP()
};
