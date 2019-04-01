#include "stdafx.h"

#include "EoDlgFixupOptions.h"

// EoDlgFixupOptions dialog

IMPLEMENT_DYNAMIC(EoDlgFixupOptions, CDialog)

BEGIN_MESSAGE_MAP(EoDlgFixupOptions, CDialog)
END_MESSAGE_MAP()

EoDlgFixupOptions::EoDlgFixupOptions(CWnd* parent) 
    : CDialog(EoDlgFixupOptions::IDD, parent)
    , m_FixupAxisTolerance(0.)
    , m_FixupModeCornerSize(0.) {
}

EoDlgFixupOptions::~EoDlgFixupOptions() {
}

void EoDlgFixupOptions::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FIX_AX_TOL, m_FixupAxisTolerance);
	DDX_Text(pDX, IDC_FIX_SIZ, m_FixupModeCornerSize);
}
