#pragma once

// EoDlgSelectIsometricView dialog

class EoDlgSelectIsometricView : public CDialog {
	DECLARE_DYNAMIC(EoDlgSelectIsometricView)

	EoDlgSelectIsometricView(CWnd* parent = nullptr);
	virtual ~EoDlgSelectIsometricView();

	// Dialog Data
	enum { IDD = IDD_SELECT_ISOMETRIC_VIEW };

  protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;

  public:
	int m_LeftRight;
	int m_FrontBack;
	int m_AboveUnder;

  protected:
	DECLARE_MESSAGE_MAP()
};
