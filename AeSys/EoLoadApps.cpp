#include "stdafx.h"
#include "AeSys.h"
#include "EoLoadApps.h"
#include "RxModule.h"

EoLoadApps::LoadedApps* EoLoadApps::m_LoadedApps = 0;

void EoLoadApps::rxInit() {
	m_LoadedApps = new LoadedApps;
	odrxDynamicLinker()->addReactor(m_LoadedApps);
}
void EoLoadApps::rxUninit() {
	odrxDynamicLinker()->removeReactor(m_LoadedApps);
	delete m_LoadedApps;
}
EoLoadApps::EoLoadApps(CWnd* parent)
	: CDialog(EoLoadApps::IDD, parent) {
}
void EoLoadApps::LoadedApps::rxAppLoaded(OdRxModule* appModule) {
	append(appModule->moduleName());
	if (m_pListBox) {
		m_pListBox->SetCurSel(m_pListBox->AddString(appModule->moduleName()));
	}
}
void EoLoadApps::LoadedApps::rxAppUnloaded(const OdString& appName) {
	remove(appName);
	if (m_pListBox) {
		int n = m_pListBox->FindString(0, OdString(appName));
		if (n != LB_ERR) {
			m_pListBox->DeleteString(n);
		}
	}
}
void EoLoadApps::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_UNLOAD_APP, m_UnloadButton);
	DDX_Control(pDX, IDC_APPS_LIST, m_AppsList);
}

BEGIN_MESSAGE_MAP(EoLoadApps, CDialog)
	ON_BN_CLICKED(IDC_LOAD_APP, OnLoadApp)
	ON_BN_CLICKED(IDC_UNLOAD_APP, OnUnloadApp)
	ON_LBN_SETFOCUS(IDC_APPS_LIST, OnAppsListEvent)
	ON_LBN_SELCHANGE(IDC_APPS_LIST, OnAppsListEvent)
	ON_LBN_KILLFOCUS(IDC_APPS_LIST, OnAppsListEvent)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL EoLoadApps::OnInitDialog() {
	CDialog::OnInitDialog();

	m_AppsList.ResetContent();

	m_LoadedApps->m_pListBox = &m_AppsList;

	for (unsigned i = 0; i < m_LoadedApps->size(); ++i) {
		const int n = m_AppsList.AddString(m_LoadedApps->at(i));
		OdRxModulePtr pModule = ::odrxDynamicLinker()->loadModule(m_LoadedApps->at(i));
		m_AppsList.SetItemData(n, (LPARAM) pModule.get());
	}
	OnAppsListEvent();

	return TRUE;
}

void EoLoadApps::OnLoadApp() {
	CFileDialog FileDialog(TRUE, nullptr, nullptr, OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST, L"Run-time Extentions (*.dll,*.tx)|*.dll;*.tx|Any file (*.*)|*.*||", this);

	FileDialog.m_ofn.lpstrTitle = L"Load application";
	auto Path {AeSys::getApplicationPath()};
	FileDialog.m_ofn.lpstrInitialDir = Path.GetBuffer(Path.GetLength());

	if (FileDialog.DoModal() == IDOK) {
		try {
			::odrxDynamicLinker()->loadModule((const wchar_t*)FileDialog.GetPathName(), false);
		} catch (const OdError & Error) {
			theApp.reportError(L"Error", Error);
		}
	}
	OnAppsListEvent();
}

void EoLoadApps::OnUnloadApp() {
	int nIndex = m_AppsList.GetCurSel();
	if (nIndex != LB_ERR) {
		CString s;
		m_AppsList.GetText(nIndex, s);
		if (::odrxDynamicLinker()->unloadModule((const wchar_t*) s)) {
			//m_AppsList.DeleteString(nIndex);
			if (m_AppsList.GetCount() <= nIndex) {
				nIndex = m_AppsList.GetCount() - 1;
			}
			m_AppsList.SetCurSel(nIndex);
		} else {
			// <tas="theApp.WarningMessageBox(L"Failed", L"Module is referenced.", MB_OK);"</tas>
		}
	}
	OnAppsListEvent();
}

void EoLoadApps::OnAppsListEvent() {
	m_UnloadButton.EnableWindow(m_AppsList.GetCurSel() != LB_ERR);
}

void EoLoadApps::OnDestroy() {
	m_LoadedApps->m_pListBox = nullptr;
	CDialog::OnDestroy();
}
