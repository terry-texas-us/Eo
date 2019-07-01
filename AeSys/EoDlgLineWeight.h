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
	int m_OriginalLineWeight;
public:
	CListBox lineWeightList;
	OdDb::LineWeight lineWeight;

	void OnBnClickedOk();

	void OnLbnDblclkListLineweight();

DECLARE_MESSAGE_MAP()
};
