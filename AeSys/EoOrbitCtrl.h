#pragma once
#include <Gi/GiDrawableImpl.h>

class OrbitCtrl : public OdGiDrawableImpl<> {
public:
	unsigned long subSetAttributes(OdGiDrawableTraits* drawableTraits) const noexcept override;

	bool subWorldDraw(OdGiWorldDraw* worldDraw) const noexcept override;

	void subViewportDraw(OdGiViewportDraw* viewportDraw) const override;
};
