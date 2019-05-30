#pragma once

#include "DbUserIO.h"
#include "RxObjectImpl.h"
#include "ExStringIO.h"

class EoDlgUserIOConsole
	: public CDialog
	, public OdEdBaseIO {

	static int sm_WindowWidth;
	static int sm_WindowHeight;

	using CDialog::operator new;
	using CDialog::operator delete;

	unsigned m_RefCounter;
	int m_NumberOfStrings;
	CFont m_Font;

	void AddOut(const CString& string);
	void AddString(const CString& string);

protected:

	EoDlgUserIOConsole(CWnd* parent);

	void addRef() noexcept override;
	long numRefs() const noexcept override;
	void release() override;

	enum { IDD = IDD_CONSOLE_DLG };
	CStatic	m_PromptWindow;
	CString	m_Input;
	CString	m_Prompt;
	CString	m_Output;

protected:

	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnPaint();
	DECLARE_MESSAGE_MAP()

public:

	void Echo(const OdString& string);
	OdString GetLastString();
	static OdSmartPtr<EoDlgUserIOConsole> create(CWnd* parent);
	void OnSize(unsigned type, int cx, int cy);
	void OnDestroy();
	void OnShowWindow(BOOL show, unsigned status);

public: // Methods - virtuals

	unsigned long getKeyState() noexcept override { return 0; }
	OdString getString(const OdString& prompt, int options, OdEdStringTracker* tracker) override;
	void putString(const OdString& string) override;
};
