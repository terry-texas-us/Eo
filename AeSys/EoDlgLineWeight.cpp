#include "stdafx.h"
#include "MainFrm.h"

#include "EoDlgLineWeight.h"

// EoDlgLineWeight dialog

IMPLEMENT_DYNAMIC(EoDlgLineWeight, CDialog)

BEGIN_MESSAGE_MAP(EoDlgLineWeight, CDialog)
	ON_LBN_DBLCLK(IDC_LIST_LINEWEIGHT, &EoDlgLineWeight::OnLbnDblclkListLineweight)
	ON_BN_CLICKED(IDOK, &EoDlgLineWeight::OnBnClickedOk)
END_MESSAGE_MAP()

EoDlgLineWeight::EoDlgLineWeight(CWnd* parent /*=NULL*/)
	: CDialog(EoDlgLineWeight::IDD, parent), m_OriginalLineWeight(0) {
}

EoDlgLineWeight::EoDlgLineWeight(int originalLineWeight, CWnd* parent)
	: CDialog(EoDlgLineWeight::IDD, parent)
	, m_OriginalLineWeight(originalLineWeight)
	, m_LineWeight((OdDb::LineWeight) originalLineWeight) {
}

EoDlgLineWeight::~EoDlgLineWeight() {
}

void EoDlgLineWeight::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LINEWEIGHT, m_LineWeightList);
}


BOOL EoDlgLineWeight::OnInitDialog(void) {
	CDialog::OnInitDialog();

	m_LineWeightList.InsertString(0, CMainFrame::StringByLineWeight(OdDb::kLnWtByLwDefault, false));
	m_LineWeightList.SetItemData(0, (DWORD_PTR) OdDb::kLnWtByLwDefault);
	for (int Index = 1; Index < 25; ++Index) {
		m_LineWeightList.InsertString(Index, CMainFrame::StringByLineWeight(Index - 1, true));
		m_LineWeightList.SetItemData(Index, (DWORD_PTR) CMainFrame::LineWeightByIndex(char(Index - 1)));
	}
	m_LineWeightList.SelectString(- 1, CMainFrame::StringByLineWeight(m_OriginalLineWeight, false));
	CString Text;
#pragma warning(suppress: 6284)
	Text.Format(L"Original : %s", CMainFrame::StringByLineWeight(m_OriginalLineWeight, false));
	GetDlgItem(IDC_STATIC_LINEWEIGHT_ORIGINAL)->SetWindowTextW(Text);
	return TRUE;
}

void EoDlgLineWeight::OnBnClickedOk() {
	int Index = m_LineWeightList.GetCurSel();
	m_LineWeight = (OdDb::LineWeight) m_LineWeightList.GetItemData(Index);

	CDialog::OnOK();
}

void EoDlgLineWeight::OnLbnDblclkListLineweight() {
	OnBnClickedOk();
}