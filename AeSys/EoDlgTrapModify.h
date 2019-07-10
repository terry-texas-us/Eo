#pragma once

/// <summary>Modifies attributes of all group primitives in current trap to current settings.</summary>
/// <remarks>Trap color index is not modified.</remarks>
class EoDlgTrapModify final : public CDialog {
DECLARE_DYNAMIC(EoDlgTrapModify)

	EoDlgTrapModify(CWnd* parent = nullptr) noexcept;

	EoDlgTrapModify(AeSysDoc* document, CWnd* parent = nullptr);

	enum { IDD = IDD_TRAP_MODIFY };

protected:
	void DoDataExchange(CDataExchange* dataExchange) final;

	void OnOK() final;

	AeSysDoc* m_Document {nullptr};
public:
	void ModifyPolygons() const;

DECLARE_MESSAGE_MAP()
};

const int gc_TrapModifyTextAll = 0;
const int gc_TrapModifyTextFont = 1;
const int gc_TrapModifyTextHeight = 2;
