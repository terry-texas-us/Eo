#pragma once

// EoDlgTrapModify dialog

/// <summary>Modifies attributes of all group primatives in current trap to current settings.</summary>
/// <remarks>Trap color index is not modified.</remarks>

class EoDlgTrapModify : public CDialog {
	DECLARE_DYNAMIC(EoDlgTrapModify)

public:
	EoDlgTrapModify(CWnd* parent = NULL);
	EoDlgTrapModify(AeSysDoc* document, CWnd* parent = NULL);
	virtual ~EoDlgTrapModify();

// Dialog Data
	enum { IDD = IDD_TRAP_MODIFY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();

	AeSysDoc* m_Document;

public:
	void ModifyPolygons(void);

protected:
	DECLARE_MESSAGE_MAP()
};

const int TM_TEXT_ALL = 0;
const int TM_TEXT_FONT = 1;
const int TM_TEXT_HEIGHT = 2;
