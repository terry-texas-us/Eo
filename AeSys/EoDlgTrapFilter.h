#pragma once

// EoDlgTrapFilter dialog

class EoDlgTrapFilter : public CDialog {
	DECLARE_DYNAMIC(EoDlgTrapFilter)

public:
	EoDlgTrapFilter(CWnd* parent = nullptr);
	EoDlgTrapFilter(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent = nullptr);
	virtual ~EoDlgTrapFilter();

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

	void FilterByColor(OdInt16 colorIndex);
	void FilterByLinetype(OdInt16 linetypeIndex);
	void FilterByPrimitiveType(const EoDb::PrimitiveTypes primitiveType);

protected:
	DECLARE_MESSAGE_MAP()
};
