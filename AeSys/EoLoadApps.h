#pragma once
#include "RxDLinkerReactor.h"

class EoLoadApps : public CDialog {
public:
	class LoadedApps : public OdArray<OdString>, public OdStaticRxObject<OdRxDLinkerReactor> {
	public:
		ODRX_HEAP_OPERATORS()

		LoadedApps() = default;
		CListBox* m_pListBox {nullptr};
		void rxAppLoaded(OdRxModule* appModule) override;
		void rxAppUnloaded(const OdString& appName) override;
	};

private:
	friend class LoadedApps;
	static LoadedApps* m_LoadedApps;
public:
	EoLoadApps(CWnd* parent = nullptr);
	static void rxInit();
	static void rxUninit();

	enum { IDD = IDD_LOAD_APPS };

	CButton m_UnloadButton;
	CListBox m_AppsList;
protected:
	void DoDataExchange(CDataExchange* pDX) final;
	BOOL OnInitDialog() final;
	void OnLoadApp();
	void OnUnloadApp();
	void OnAppsListEvent();
	void OnDestroy();
DECLARE_MESSAGE_MAP()
};
