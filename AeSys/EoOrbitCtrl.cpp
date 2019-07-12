#include "stdafx.h"
#include <DbSubDMesh.h>
#include <Gi/GiCommonDraw.h>
#include "EoOrbitCtrl.h"
#include <Gi/GiViewportDraw.h>

unsigned long OrbitCtrl::subSetAttributes(OdGiDrawableTraits* /*drawableTraits*/) const noexcept {
	return kDrawableIsAnEntity | kDrawableRegenDraw;
}

bool OrbitCtrl::subWorldDraw(OdGiWorldDraw* /*worldDraw*/) const noexcept {
	return false;
}

void OrbitCtrl::subViewportDraw(OdGiViewportDraw* viewportDraw) const {
	const auto& Viewport = viewportDraw->viewport();
	OdGiGeometry& Geometry = viewportDraw->geometry();
	viewportDraw->subEntityTraits().setColor(OdCmEntityColor::kACIGreen);
	viewportDraw->subEntityTraits().setFillType(kOdGiFillNever);
	OdGiModelTransformSaver ModelTransformSaver(Geometry, Viewport.getEyeToModelTransform());
	OdGiDrawFlagsHelper DrawFlagsHelper(viewportDraw->subEntityTraits(), OdGiSubEntityTraits::kDrawNoPlotstyle);
	OdGePoint3d LowerLeftCorner;
	OdGePoint2d UpperRightCorner;
	Viewport.getViewportDcCorners(reinterpret_cast<OdGePoint2d&>(LowerLeftCorner), UpperRightCorner);
	UpperRightCorner.x -= LowerLeftCorner.x;
	UpperRightCorner.y -= LowerLeftCorner.y;
	const auto Radius {odmin(UpperRightCorner.x, UpperRightCorner.y) / 9.0 * 7.0 / 2.0};
	reinterpret_cast<OdGePoint2d&>(LowerLeftCorner) += UpperRightCorner.asVector() / 2.0;
	Geometry.circle(LowerLeftCorner, Radius, OdGeVector3d::kZAxis);
	Geometry.circle(LowerLeftCorner + OdGeVector3d(0.0, Radius, 0.0), Radius / 20.0, OdGeVector3d::kZAxis);
	Geometry.circle(LowerLeftCorner + OdGeVector3d(0.0, -Radius, 0.0), Radius / 20.0, OdGeVector3d::kZAxis);
	Geometry.circle(LowerLeftCorner + OdGeVector3d(Radius, 0.0, 0.0), Radius / 20.0, OdGeVector3d::kZAxis);
	Geometry.circle(LowerLeftCorner + OdGeVector3d(-Radius, 0.0, 0.0), Radius / 20.0, OdGeVector3d::kZAxis);
}
