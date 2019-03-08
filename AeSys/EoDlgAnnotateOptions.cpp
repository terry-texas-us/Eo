#include "stdafx.h"
#include "AeSysApp.h"
#include "AeSysView.h"

#include "EoDlgAnnotateOptions.h"

// EoDlgAnnotateOptions dialog

IMPLEMENT_DYNAMIC(EoDlgAnnotateOptions, CDialog)

BEGIN_MESSAGE_MAP(EoDlgAnnotateOptions, CDialog)
END_MESSAGE_MAP()

EoDlgAnnotateOptions::EoDlgAnnotateOptions(CWnd* pParent /* = NULL */) :
	CDialog(EoDlgAnnotateOptions::IDD, pParent)
	, m_GapSpaceFactor(0), m_CircleRadius(0), m_EndItemSize(0), m_BubbleRadius(0), m_NumberOfSides(0), m_DefaultText(L"") {
}
EoDlgAnnotateOptions::EoDlgAnnotateOptions(AeSysView* view, CWnd* pParent /* = NULL */) :
	CDialog(EoDlgAnnotateOptions::IDD, pParent), m_ActiveView(view) {
	m_GapSpaceFactor = view->GapSpaceFactor();
	m_CircleRadius = view->CircleRadius();
	m_EndItemSize = view->EndItemSize();
	m_BubbleRadius = view->BubbleRadius();
	m_NumberOfSides = view->NumberOfSides();
	m_DefaultText = view->DefaultText();
}
EoDlgAnnotateOptions::~EoDlgAnnotateOptions() {
}
void EoDlgAnnotateOptions::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANN_ARR_TYP, m_EndItemTypeComboBox);
	DDX_Text(pDX, IDC_ANN_GAP_SPACE_FAC, m_GapSpaceFactor);
	DDX_Text(pDX, IDC_ANN_HOOK_RAD, m_CircleRadius);
	DDX_Text(pDX, IDC_ANN_ARR_SIZ, m_EndItemSize);
	DDX_Text(pDX, IDC_ANN_BUB_RAD, m_BubbleRadius);
	DDX_Text(pDX, IDC_ANN_BUB_FACETS, m_NumberOfSides);
	DDX_Text(pDX, IDC_ANN_DEF_TXT, m_DefaultText);
}
BOOL EoDlgAnnotateOptions::OnInitDialog() {
	CDialog::OnInitDialog();

	m_EndItemTypeComboBox.SetCurSel(m_ActiveView->EndItemType() - 1);

	return TRUE;
}
void EoDlgAnnotateOptions::OnOK() {
	CDialog::OnOK();

	m_ActiveView->SetDefaultText(m_DefaultText);
	m_ActiveView->SetNumberOfSides(m_NumberOfSides);
	m_ActiveView->SetBubbleRadius(m_BubbleRadius);
	m_ActiveView->SetEndItemSize(m_EndItemSize);
	m_ActiveView->SetCircleRadius(m_CircleRadius);
	m_ActiveView->SetGapSpaceFactor(m_GapSpaceFactor);

	m_ActiveView->SetEndItemType(m_EndItemTypeComboBox.GetCurSel() + 1);
}
