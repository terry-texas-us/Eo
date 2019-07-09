#pragma once
class EoDlgTrapFilter final : public CDialog {
DECLARE_DYNAMIC(EoDlgTrapFilter)

	EoDlgTrapFilter(CWnd* parent = nullptr);

	EoDlgTrapFilter(AeSysDoc* document, OdDbDatabasePtr database, CWnd* parent = nullptr);

	~EoDlgTrapFilter();

	enum { IDD = IDD_TRAP_FILTER };

	CComboBox filterLineComboBoxControl;
	CListBox filterPrimitiveTypeListBoxControl;

	void FilterByColor(short colorIndex) const;

	void FilterByLinetype(short linetypeIndex) const;

	void FilterByPrimitiveType(EoDb::PrimitiveTypes primitiveType) const;

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	BOOL OnInitDialog() final;

	void OnOK() final;

private:
	AeSysDoc* m_Document {nullptr};
	OdDbDatabasePtr m_Database;
DECLARE_MESSAGE_MAP()
};
