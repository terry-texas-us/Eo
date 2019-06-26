#pragma once
class EoApOptions {
public:
	EoApOptions();
	~EoApOptions() = default;

	enum TabsStyle { None, Standard, Grouped };

	TabsStyle m_nTabsStyle;
	CMDITabInfo m_MdiTabInfo;
	bool m_TabsContextMenu;
	bool m_DisableSetRedraw;

	void Load();
	void Save();
};
