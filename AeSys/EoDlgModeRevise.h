#pragma once
class EoDlgModeRevise final : public CDialog {
DECLARE_DYNAMIC(EoDlgModeRevise)

	EoDlgModeRevise(CWnd* parent = nullptr);

	enum { IDD = IDD_ADD_NOTE };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

	void OnOK() final;

	static EoDbFontDefinition m_FontDefinition;
	static EoGeReferenceSystem m_ReferenceSystem;
	static EoDbText* m_TextPrimitive;
public:
	CEdit textEditControl;
	/// <summary> Effectively resizes the edit control to use the entire client area of the dialog.</summary>
	/// <remarks> OnSize can be called before OnInitialUpdate so check is made for valid control window.</remarks>
	void OnSize(unsigned type, int cx, int cy); // hides non-virtual function of parent
protected:
DECLARE_MESSAGE_MAP()
};
