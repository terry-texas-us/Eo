#include "stdafx.h"
#include <DbCommandContext.h>
#include <DbSSet.h>
#include <ExDbCommandContext.h>
#include <Ge/GeExtents2d.h>
#include "AeSys.h"
#include "EoDlgUserIoConsole.h"

EoDlgUserIoConsole::EoDlgUserIoConsole(CWnd* parent)
	: CDialog(IDD, parent)
	, m_RefCounter(1)
	, m_NumberOfStrings(0) {
	m_Input = L"";
	m_Prompt = L"";
}

OdString EoDlgUserIoConsole::GetLastString() const {
	const auto EolDelimiter {m_Output.ReverseFind('\r')};
	if (EolDelimiter == -1) {
		return static_cast<const wchar_t*>(m_Output);
	}
	return static_cast<const wchar_t*>(m_Output.Mid(EolDelimiter + 2));
}

void EoDlgUserIoConsole::addRef() noexcept {
	m_RefCounter++;
}

long EoDlgUserIoConsole::numRefs() const noexcept {
	return static_cast<long>(m_RefCounter);
}

void EoDlgUserIoConsole::release() {
	ODA_ASSERT(m_RefCounter > 0);
	if (--m_RefCounter == 0U) {
		delete this;
	}
}

void EoDlgUserIoConsole::DoDataExchange(CDataExchange* dataExchange) {
	CDialog::DoDataExchange(dataExchange);
	DDX_Control(dataExchange, IDC_PROMPT, m_PromptWindow);
	DDX_Text(dataExchange, IDC_INPUT, m_Input);
	DDX_Text(dataExchange, IDC_PROMPT, m_Prompt);
}
#pragma warning(push)
#pragma warning(disable : 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
BEGIN_MESSAGE_MAP(EoDlgUserIoConsole, CDialog)
		ON_WM_PAINT()
		ON_WM_SIZE()
		ON_WM_DESTROY()
		ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()
#pragma warning (pop)
OdSmartPtr<EoDlgUserIoConsole> EoDlgUserIoConsole::create(CWnd* parent) {
	return OdSmartPtr<EoDlgUserIoConsole>(new EoDlgUserIoConsole(parent), kOdRxObjAttach);
}

void EoDlgUserIoConsole::Echo(const OdString& string) {
	m_Output += L" ";
	m_Output += static_cast<const wchar_t*>(string);
}

OdString EoDlgUserIoConsole::getString(const OdString& prompt, int /*options*/, OdEdStringTracker* /*tracker*/) {
	putString(prompt);
	m_Input.Empty();
	m_Prompt = m_Output;
	if (DoModal() == IDCANCEL) {
		m_Output += L" *Cancel*";
		throw OdEdCancel();
	}
	Echo(OdString(m_Input));
	m_NumberOfStrings = 0;
	return static_cast<const wchar_t*>(m_Input);
}

void EoDlgUserIoConsole::AddString(const CString& string) {
	const auto MaxStringLength {128};
	auto& OutputString {m_Output};
	if (string.GetLength() <= MaxStringLength) {
		++m_NumberOfStrings;
		OutputString += "\r\n";
		OutputString += string;
	} else { // break long string
		auto LongString {string};
		while (LongString.GetLength() > MaxStringLength) {
			auto sMax {LongString.Left(MaxStringLength)};
			const auto n = sMax.ReverseFind(' ');
			if (n > -1) {
				AddString(sMax.Left(n));
				LongString = LongString.Right(LongString.GetLength() - n - 1);
			} else {
				AddString(sMax);
				LongString = LongString.Right(LongString.GetLength() - MaxStringLength);
			}
		}
		ASSERT(!LongString.IsEmpty());
		AddString(LongString);
	}
}

void EoDlgUserIoConsole::AddOut(const CString& string) {
	int Count;
	auto First {0};
	while ((Count = string.Find('\n', First)) > -1) {
		AddString(string.Mid(First, Count - First));
		First = ++Count;
	}
	Count = string.GetLength();
	AddString(string.Mid(First, Count - First));
}

void EoDlgUserIoConsole::putString(const OdString& string) {
	AddOut(static_cast<const wchar_t*>(string));
}

BOOL EoDlgUserIoConsole::OnInitDialog() {
	CDialog::OnInitDialog();
	if (m_Font.m_hObject == nullptr) {
		m_Font.CreateFont(10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Courier");
	}
	return TRUE;
}

void EoDlgUserIoConsole::OnPaint() {
	CPaintDC dc(this);
	m_PromptWindow.SetFont(&m_Font);
}

void EoDlgUserIoConsole::OnSize(const unsigned type, const int cx, const int cy) {
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

int EoDlgUserIoConsole::sm_WindowWidth = 660;
int EoDlgUserIoConsole::sm_WindowHeight = 200;

void EoDlgUserIoConsole::OnDestroy() {
	CRect WindowRectangle;
	GetWindowRect(WindowRectangle);
	sm_WindowWidth = WindowRectangle.Width();
	sm_WindowHeight = WindowRectangle.Height();
	CDialog::OnDestroy();
}

void EoDlgUserIoConsole::OnShowWindow(const BOOL show, const unsigned status) {
	CDialog::OnShowWindow(show, status);
	CRect WindowRectangle;
	GetWindowRect(&WindowRectangle);
	SetWindowPos(nullptr, WindowRectangle.left, WindowRectangle.top, sm_WindowWidth, sm_WindowHeight, SWP_SHOWWINDOW | SWP_NOZORDER);
	auto EditControl {static_cast<CEdit*>(GetDlgItem(IDC_PROMPT))};
	EditControl->LineScroll(EditControl->GetLineCount());
}
