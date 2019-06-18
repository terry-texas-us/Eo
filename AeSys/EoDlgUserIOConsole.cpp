#include "stdafx.h"

#include "AeSys.h"
#include "DbCommandContext.h"
#include "DbSSet.h"
#include "ExDbCommandContext.h"
#include "Ge/GeExtents2d.h"

#include "EoDlgUserIOConsole.h"

EoDlgUserIOConsole::EoDlgUserIOConsole(CWnd* parent)
	: CDialog(EoDlgUserIOConsole::IDD, parent)
	, m_RefCounter(1)
	, m_NumberOfStrings(0) {
	m_Input = L"";
	m_Prompt = L"";
}

OdString EoDlgUserIOConsole::GetLastString() {
	const auto EolDelimiter {m_Output.ReverseFind('\r')};
	if (EolDelimiter == -1) { return (const wchar_t*) m_Output; }

	return (const wchar_t*) m_Output.Mid(EolDelimiter + 2);
}

void EoDlgUserIOConsole::addRef() noexcept {
	m_RefCounter++;
}

long EoDlgUserIOConsole::numRefs() const noexcept {
	return static_cast<long>(m_RefCounter);
}

void EoDlgUserIOConsole::release() {
	ODA_ASSERT((m_RefCounter > 0));
	if (!(--m_RefCounter)) { delete this; }
}

void EoDlgUserIOConsole::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROMPT, m_PromptWindow);
	DDX_Text(pDX, IDC_INPUT, m_Input);
	DDX_Text(pDX, IDC_PROMPT, m_Prompt);
}

BEGIN_MESSAGE_MAP(EoDlgUserIOConsole, CDialog)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

OdSmartPtr<EoDlgUserIOConsole> EoDlgUserIOConsole::create(CWnd* parent) {
	return OdSmartPtr<EoDlgUserIOConsole>(new EoDlgUserIOConsole(parent), kOdRxObjAttach);
}

void EoDlgUserIOConsole::Echo(const OdString& string) {
	m_Output += L" ";
	m_Output += (const wchar_t*) string;
}

OdString EoDlgUserIOConsole::getString(const OdString& prompt, int options, OdEdStringTracker* tracker) {
	putString(prompt);
	m_Input.Empty();
	m_Prompt = m_Output;

	if (DoModal() == IDCANCEL) {
		m_Output += L" *Cancel*";
		throw OdEdCancel();
	}
	Echo(OdString(m_Input));
	m_NumberOfStrings = 0;
	return (const wchar_t*) m_Input;
}

const int kMaxStringLength = 128;

void EoDlgUserIOConsole::AddString(const CString& string) {
	CString& OutputString = m_Output;

	if (string.GetLength() <= kMaxStringLength) {
		++m_NumberOfStrings;
		OutputString += "\r\n";
		OutputString += string;
	} else { // break long string
		CString LongString(string);
		CString sMax;
		while (LongString.GetLength() > kMaxStringLength) {
			sMax = LongString.Left(kMaxStringLength);
			const auto n = sMax.ReverseFind(' ');
			if (n > -1) {
				AddString(sMax.Left(n));
				LongString = LongString.Right(LongString.GetLength() - n - 1);
			} else {
				AddString(sMax);
				LongString = LongString.Right(LongString.GetLength() - kMaxStringLength);
			}
		}
		ASSERT(!LongString.IsEmpty());
		AddString(LongString);
	}
}

void EoDlgUserIOConsole::AddOut(const CString& string) {
	int n = 0;
	int n0 = 0;
	while ((n = string.Find('\n', n0)) > -1) {
		AddString(string.Mid(n0, n - n0));
		n0 = ++n;
	}
	n = string.GetLength();
	AddString(string.Mid(n0, n - n0));
}

void EoDlgUserIOConsole::putString(const OdString& string) {
	AddOut((const wchar_t*) string);
}

BOOL EoDlgUserIOConsole::OnInitDialog() {
	CDialog::OnInitDialog();

	if (!m_Font.m_hObject) {
		m_Font.CreateFont(10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Courier");
	}
	return TRUE;
}

void EoDlgUserIOConsole::OnPaint() {
	CPaintDC dc(this);
	m_PromptWindow.SetFont(&m_Font);
}

void EoDlgUserIOConsole::OnSize(unsigned type, int cx, int cy) {
	CRect PromptRect;
	CRect InputRect;
	GetDlgItem(IDC_PROMPT)->GetWindowRect(&PromptRect);
	GetDlgItem(IDC_INPUT)->GetWindowRect(&InputRect);
	ScreenToClient(PromptRect);
	ScreenToClient(InputRect);
	const auto Border {PromptRect.left};
	const auto EditHeight {InputRect.Height()};
	CDialog::OnSize(type, cx, cy);
	PromptRect.right = cx - Border;
	PromptRect.bottom = cy - Border - EditHeight;
	GetDlgItem(IDC_PROMPT)->MoveWindow(PromptRect);
	InputRect.top = PromptRect.bottom;
	InputRect.bottom = cy - Border;
	InputRect.right = cx - Border;
	GetDlgItem(IDC_INPUT)->MoveWindow(InputRect);
}

int EoDlgUserIOConsole::sm_WindowWidth = 660;
int EoDlgUserIOConsole::sm_WindowHeight = 200;

void EoDlgUserIOConsole::OnDestroy() {
	CRect WindowRectangle;
	GetWindowRect(WindowRectangle);
	sm_WindowWidth = WindowRectangle.Width();
	sm_WindowHeight = WindowRectangle.Height();
	CDialog::OnDestroy();
}

void EoDlgUserIOConsole::OnShowWindow(BOOL show, unsigned status) {
	CDialog::OnShowWindow(show, status);
	CRect WindowRectangle;
	GetWindowRect(&WindowRectangle);
	SetWindowPos(0, WindowRectangle.left, WindowRectangle.top, sm_WindowWidth, sm_WindowHeight, SWP_SHOWWINDOW | SWP_NOZORDER);
	auto EditControl {(CEdit*) GetDlgItem(IDC_PROMPT)};
	EditControl->LineScroll(EditControl->GetLineCount());
}
