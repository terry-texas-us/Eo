#pragma once

/// <summary>Modifies attributes of all group primitives in current trap to current settings.</summary>
/// <remarks>Trap color index is not modified.</remarks>
class EoDlgTrapModify final : public CDialog {
DECLARE_DYNAMIC(EoDlgTrapModify)
	EoDlgTrapModify(CWnd* parent = nullptr) noexcept;
	EoDlgTrapModify(AeSysDoc* document, CWnd* parent = nullptr);
	~EoDlgTrapModify();

	enum { IDD = IDD_TRAP_MODIFY };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;
	void OnOK() final;
	AeSysDoc* m_Document {nullptr};
public:
	void ModifyPolygons();
protected:
DECLARE_MESSAGE_MAP()
};

const int TM_TEXT_ALL = 0;
const int TM_TEXT_FONT = 1;
const int TM_TEXT_HEIGHT = 2;
