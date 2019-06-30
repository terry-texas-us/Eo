#pragma once
struct EoApOptions {
	EoApOptions();

	enum TabsStyle { kNone, kStandard, kGrouped };
	TabsStyle tabsStyle;
	CMDITabInfo mdiTabInfo;
	bool tabsContextMenu;
	bool disableSetRedraw;

	void Load();
	void Save();
};
