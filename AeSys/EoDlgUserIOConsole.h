#pragma once
#include "DbUserIO.h"
#include "ExStringIO.h"

class EoDlgUserIoConsole final : public CDialog, public OdEdBaseIO {
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
	EoDlgUserIoConsole(CWnd* parent);
	void addRef() noexcept override;
	long numRefs() const noexcept override;
	void release() override;

	enum { IDD = IDD_CONSOLE_DLG };

	CStatic m_PromptWindow;
	CString m_Input;
	CString m_Prompt;
	CString m_Output;
	void DoDataExchange(CDataExchange* dataExchange) final;
	BOOL OnInitDialog() final;
	void OnPaint(); // hides non-virtual function of parent
	DECLARE_MESSAGE_MAP()
public:
	void Echo(const OdString& string);
	OdString GetLastString();
	static OdSmartPtr<EoDlgUserIoConsole> create(CWnd* parent);
	void OnSize(unsigned type, int cx, int cy); // hides non-virtual function of parent
	void OnDestroy(); // hides non-virtual function of parent
	void OnShowWindow(BOOL show, unsigned status); // hides non-virtual function of parent

	unsigned long getKeyState() noexcept override { return 0; }

	OdString getString(const OdString& prompt, int options, OdEdStringTracker* tracker) override;
	void putString(const OdString& string) override;
};
