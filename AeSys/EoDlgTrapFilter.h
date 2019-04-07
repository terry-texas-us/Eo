#pragma once

// EoDlgTrapFilter dialog

class EoDlgTrapFilter : public CDialog {
	DECLARE_DYNAMIC(EoDlgTrapFilter)

public:
	EoDlgTrapFilter(CWnd* parent = NULL);
	EoDlgTrapFilter(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent = NULL);
	virtual ~EoDlgTrapFilter();

// Dialog Data
	enum { IDD = IDD_TRAP_FILTER };

	AeSysDoc* m_Document;
	OdDbDatabasePtr m_Database;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

public:
	CComboBox m_FilterLineComboBoxControl;
	CListBox m_FilterPrimitiveTypeListBoxControl;

	void FilterByColor(OdInt16 colorIndex);
	void FilterByLinetype(OdInt16 linetypeIndex);
	void FilterByPrimitiveType(const EoDb::PrimitiveTypes primitiveType);

protected:
	DECLARE_MESSAGE_MAP()
};
