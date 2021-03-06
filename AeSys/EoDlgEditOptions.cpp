#include "stdafx.h"
#include "AeSys.h"
#include "AeSysView.h"
#include "EoDlgEditOptions.h"
IMPLEMENT_DYNAMIC(EoDlgEditOptions, CDialog)

BEGIN_MESSAGE_MAP(EoDlgEditOptions, CDialog)
		ON_COMMAND(IDC_EDIT_OP_ROTATION, &EoDlgEditOptions::OnEditOpRotation)
		ON_COMMAND(IDC_EDIT_OP_MIRRORING, &EoDlgEditOptions::OnEditOpMirroring)
		ON_COMMAND(IDC_EDIT_OP_SIZING, &EoDlgEditOptions::OnEditOpSizing)
		ON_BN_CLICKED(IDC_EDIT_OP_MIR_X, &EoDlgEditOptions::OnBnClickedEditOpMirX)
		ON_BN_CLICKED(IDC_EDIT_OP_MIR_Y, &EoDlgEditOptions::OnBnClickedEditOpMirY)
		ON_BN_CLICKED(IDC_EDIT_OP_MIR_Z, &EoDlgEditOptions::OnBnClickedEditOpMirZ)
END_MESSAGE_MAP()

EoDlgEditOptions::EoDlgEditOptions(CWnd* parent)
	: CDialog(IDD, parent) {}

EoDlgEditOptions::EoDlgEditOptions(AeSysView* view, CWnd* parent)
	: CDialog(IDD, parent)
	, m_ActiveView(view) {}

EoDlgEditOptions::~EoDlgEditOptions() = default;

void EoDlgEditOptions::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_EDIT_OP_MIR_X, m_MirrorXButton);
	DDX_Control(dataExchange, IDC_EDIT_OP_MIR_Y, m_MirrorYButton);
	DDX_Control(dataExchange, IDC_EDIT_OP_MIR_Z, m_MirrorZButton);
	DDX_Control(dataExchange, IDC_EDIT_OP_SIZ_X, m_SizingXEditControl);
	DDX_Control(dataExchange, IDC_EDIT_OP_SIZ_Y, m_SizingYEditControl);
	DDX_Control(dataExchange, IDC_EDIT_OP_SIZ_Z, m_SizingZEditControl);
	DDX_Control(dataExchange, IDC_EDIT_OP_ROT_X, m_RotationXEditControl);
	DDX_Control(dataExchange, IDC_EDIT_OP_ROT_Y, m_RotationYEditControl);
	DDX_Control(dataExchange, IDC_EDIT_OP_ROT_Z, m_RotationZEditControl);
	DDX_Text(dataExchange, IDC_EDIT_OP_SIZ_X, m_ScaleFactorX);
	DDX_Text(dataExchange, IDC_EDIT_OP_SIZ_Y, m_ScaleFactorY);
	DDX_Text(dataExchange, IDC_EDIT_OP_SIZ_Z, m_ScaleFactorZ);
	DDX_Text(dataExchange, IDC_EDIT_OP_ROT_X, m_EditModeRotationAngleX);
	DDX_Text(dataExchange, IDC_EDIT_OP_ROT_Y, m_EditModeRotationAngleY);
	DDX_Text(dataExchange, IDC_EDIT_OP_ROT_Z, m_EditModeRotationAngleZ);
}

BOOL EoDlgEditOptions::OnInitDialog() {
	CDialog::OnInitDialog();
	if (m_ActiveView->m_MirrorScaleFactors.sx < 0.0) {
		m_MirrorXButton.SetCheck(BST_CHECKED);
	} else if (m_ActiveView->m_MirrorScaleFactors.sy < 0.0) {
		m_MirrorYButton.SetCheck(BST_CHECKED);
	} else {
		m_MirrorZButton.SetCheck(BST_CHECKED);
	}
	return TRUE;
}

void EoDlgEditOptions::OnOK() {
	if (m_MirrorXButton.GetCheck() == BST_CHECKED) {
		m_ActiveView->SetEditModeMirrorScaleFactors(-1.0, 1.0, 1.0);
	} else if (m_MirrorYButton.GetCheck() == BST_CHECKED) {
		m_ActiveView->SetEditModeMirrorScaleFactors(1.0, -1.0, 1.0);
	} else {
		m_ActiveView->SetEditModeMirrorScaleFactors(1.0, 1.0, -1.0);
	}
	CDialog::OnOK();
}

void EoDlgEditOptions::OnEditOpRotation() {
	m_SizingXEditControl.EnableWindow(FALSE);
	m_SizingYEditControl.EnableWindow(FALSE);
	m_SizingZEditControl.EnableWindow(FALSE);
	m_RotationXEditControl.EnableWindow(TRUE);
	m_RotationYEditControl.EnableWindow(TRUE);
	m_RotationZEditControl.EnableWindow(TRUE);
	m_MirrorXButton.EnableWindow(FALSE);
	m_MirrorYButton.EnableWindow(FALSE);
	m_MirrorZButton.EnableWindow(FALSE);
}

void EoDlgEditOptions::OnEditOpMirroring() {
	m_SizingXEditControl.EnableWindow(FALSE);
	m_SizingYEditControl.EnableWindow(FALSE);
	m_SizingZEditControl.EnableWindow(FALSE);
	m_RotationXEditControl.EnableWindow(FALSE);
	m_RotationYEditControl.EnableWindow(FALSE);
	m_RotationZEditControl.EnableWindow(FALSE);
	m_MirrorXButton.EnableWindow(TRUE);
	m_MirrorYButton.EnableWindow(TRUE);
	m_MirrorZButton.EnableWindow(TRUE);
}

void EoDlgEditOptions::OnEditOpSizing() {
	m_SizingXEditControl.EnableWindow(TRUE);
	m_SizingYEditControl.EnableWindow(TRUE);
	m_SizingZEditControl.EnableWindow(TRUE);
	m_RotationXEditControl.EnableWindow(FALSE);
	m_RotationYEditControl.EnableWindow(FALSE);
	m_RotationZEditControl.EnableWindow(FALSE);
	m_MirrorXButton.EnableWindow(FALSE);
	m_MirrorYButton.EnableWindow(FALSE);
	m_MirrorZButton.EnableWindow(FALSE);
}

void EoDlgEditOptions::OnBnClickedEditOpMirX() {
	m_MirrorXButton.SetCheck(BST_CHECKED);
	m_MirrorYButton.SetCheck(BST_UNCHECKED);
	m_MirrorZButton.SetCheck(BST_UNCHECKED);
}

void EoDlgEditOptions::OnBnClickedEditOpMirY() {
	m_MirrorXButton.SetCheck(BST_UNCHECKED);
	m_MirrorYButton.SetCheck(BST_CHECKED);
	m_MirrorZButton.SetCheck(BST_UNCHECKED);
}

void EoDlgEditOptions::OnBnClickedEditOpMirZ() {
	m_MirrorXButton.SetCheck(BST_UNCHECKED);
	m_MirrorYButton.SetCheck(BST_UNCHECKED);
	m_MirrorZButton.SetCheck(BST_CHECKED);
}
