#pragma once

// EoDlgTrapFilter dialog
class EoDlgTrapFilter : public CDialog {
DECLARE_DYNAMIC(EoDlgTrapFilter)
	EoDlgTrapFilter(CWnd* parent = nullptr);
	EoDlgTrapFilter(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent = nullptr);
	~EoDlgTrapFilter();

	// Dialog Data
	enum { IDD = IDD_TRAP_FILTER };

	AeSysDoc* m_Document;
	OdDbDatabasePtr m_Database;
protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnOK() final;
public:
	CComboBox m_FilterLineComboBoxControl;
	CListBox m_FilterPrimitiveTypeListBoxControl;
	void FilterByColor(short colorIndex);
	void FilterByLinetype(short linetypeIndex);
	void FilterByPrimitiveType(EoDb::PrimitiveTypes primitiveType);
protected:
DECLARE_MESSAGE_MAP()
};
