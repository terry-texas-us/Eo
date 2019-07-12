#pragma once
#include <Ed/EdCommandStack.h>

struct OdExZoomCmd : OdEdCommand {
	[[nodiscard]] const OdString groupName() const override;

	[[nodiscard]] const OdString globalName() const override;

	void execute(OdEdCommandContext* edCommandContext) override;
};

void ZoomWindow2(const OdGePoint3d& pt1, const OdGePoint3d& pt2, OdGsView* pView);
