#include "stdafx.h"
#include "MainFrm.h"
#include "EoDlgLineWeight.h"

IMPLEMENT_DYNAMIC(EoDlgLineWeight, CDialog)

BEGIN_MESSAGE_MAP(EoDlgLineWeight, CDialog)
		ON_LBN_DBLCLK(IDC_LIST_LINEWEIGHT, &EoDlgLineWeight::OnLbnDblclkListLineweight)
		ON_BN_CLICKED(IDOK, &EoDlgLineWeight::OnBnClickedOk)
END_MESSAGE_MAP()

EoDlgLineWeight::EoDlgLineWeight(CWnd* parent)
	: CDialog(IDD, parent)
	, m_OriginalLineWeight(0)
	, lineWeight(OdDb::LineWeight(0)) {
}

EoDlgLineWeight::EoDlgLineWeight(int originalLineWeight, CWnd* parent)
	: CDialog(IDD, parent)
	, m_OriginalLineWeight(originalLineWeight)
	, lineWeight(static_cast<OdDb::LineWeight>(originalLineWeight)) {
}

EoDlgLineWeight::~EoDlgLineWeight() = default;

void EoDlgLineWeight::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_LIST_LINEWEIGHT, lineWeightList);
}

BOOL EoDlgLineWeight::OnInitDialog() {
	CDialog::OnInitDialog();
	lineWeightList.InsertString(0, CMainFrame::StringByLineWeight(OdDb::kLnWtByLwDefault, false));
	lineWeightList.SetItemData(0, static_cast<DWORD_PTR>(OdDb::kLnWtByLwDefault));
	for (auto Index = 1; Index < 25; ++Index) {
		lineWeightList.InsertString(Index, CMainFrame::StringByLineWeight(Index - 1, true));
		lineWeightList.SetItemData(Index, static_cast<DWORD_PTR>(CMainFrame::LineWeightByIndex(char(Index - 1))));
	}
	const auto OriginalLineWeight {CMainFrame::StringByLineWeight(m_OriginalLineWeight, false)};
	lineWeightList.SelectString(-1, OriginalLineWeight);
	const auto Text(L"Original : " + OriginalLineWeight);
	GetDlgItem(IDC_STATIC_LINEWEIGHT_ORIGINAL)->SetWindowTextW(Text);
	return TRUE;
}

void EoDlgLineWeight::OnBnClickedOk() {
	const auto Index {lineWeightList.GetCurSel()};
	lineWeight = static_cast<OdDb::LineWeight>(lineWeightList.GetItemData(Index));
	CDialog::OnOK();
}

void EoDlgLineWeight::OnLbnDblclkListLineweight() {
	OnBnClickedOk();
}
