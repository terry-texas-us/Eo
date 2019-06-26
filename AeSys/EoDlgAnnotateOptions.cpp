#include "stdafx.h"
#include "AeSys.h"
#include "AeSysView.h"
#include "EoDlgAnnotateOptions.h"

IMPLEMENT_DYNAMIC(EoDlgAnnotateOptions, CDialog)

BEGIN_MESSAGE_MAP(EoDlgAnnotateOptions, CDialog)
END_MESSAGE_MAP()

EoDlgAnnotateOptions::EoDlgAnnotateOptions(CWnd* parent)
	: CDialog(IDD, parent) {
}

EoDlgAnnotateOptions::EoDlgAnnotateOptions(AeSysView* view, CWnd* parent)
	: CDialog(IDD, parent)
	, m_ActiveView(view) {
	gapSpaceFactor = view->GapSpaceFactor();
	circleRadius = view->CircleRadius();
	endItemSize = view->EndItemSize();
	bubbleRadius = view->BubbleRadius();
	numberOfSides = view->NumberOfSides();
	defaultText = view->DefaultText();
}

void EoDlgAnnotateOptions::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_ANN_ARR_TYP, m_EndItemTypeComboBox);
	DDX_Text(dataExchange, IDC_ANN_GAP_SPACE_FAC, gapSpaceFactor);
	DDX_Text(dataExchange, IDC_ANN_HOOK_RAD, circleRadius);
	DDX_Text(dataExchange, IDC_ANN_ARR_SIZ, endItemSize);
	DDX_Text(dataExchange, IDC_ANN_BUB_RAD, bubbleRadius);
	DDX_Text(dataExchange, IDC_ANN_BUB_FACETS, numberOfSides);
	DDX_Text(dataExchange, IDC_ANN_DEF_TXT, defaultText);
}

BOOL EoDlgAnnotateOptions::OnInitDialog() {
	CDialog::OnInitDialog();
	m_EndItemTypeComboBox.SetCurSel(m_ActiveView->EndItemType() - 1);
	return TRUE;
}

void EoDlgAnnotateOptions::OnOK() {
	CDialog::OnOK();
	m_ActiveView->SetDefaultText(defaultText);
	m_ActiveView->SetNumberOfSides(numberOfSides);
	m_ActiveView->SetBubbleRadius(bubbleRadius);
	m_ActiveView->SetEndItemSize(endItemSize);
	m_ActiveView->SetCircleRadius(circleRadius);
	m_ActiveView->SetGapSpaceFactor(gapSpaceFactor);
	m_ActiveView->SetEndItemType(m_EndItemTypeComboBox.GetCurSel() + 1);
}
