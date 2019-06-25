#pragma once

class EoDlgTrapFilter final : public CDialog {
DECLARE_DYNAMIC(EoDlgTrapFilter)
	EoDlgTrapFilter(CWnd* parent = nullptr);
	EoDlgTrapFilter(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent = nullptr);
	~EoDlgTrapFilter();

	enum { IDD = IDD_TRAP_FILTER };

	AeSysDoc* document {nullptr};
	OdDbDatabasePtr database;
protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
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
