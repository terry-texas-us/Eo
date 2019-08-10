#include <OdaCommon.h>
#include <DbViewport.h>
#include "DbViewportGripPoints.h"
#include <Gs/GsDefs.h>
#include <AbstractViewPE.h>

OdResult OdDbViewportGripPointsPE::getGripPoints(const OdDbEntity* entity, OdGePoint3dArray& gripPoints) const {
	OdDbViewportPtr Viewport {entity};
	if (!Viewport->isNonRectClipOn()) {
		const auto CenterPoint {Viewport->centerPoint()};
		const auto Width {Viewport->width()};
		const auto Height {Viewport->height()};
		const auto XAxis {OdGeVector3d::kXAxis * (Width * 0.5)};
		const auto YAxis {OdGeVector3d::kYAxis * (Height * 0.5)};
		gripPoints.reserve(4);
		gripPoints.append(CenterPoint - XAxis - YAxis);
		gripPoints.append(CenterPoint + XAxis - YAxis);
		gripPoints.append(CenterPoint + XAxis + YAxis);
		gripPoints.append(CenterPoint - XAxis + YAxis);
	}
	return eOk;
}

OdResult OdDbViewportGripPointsPE::moveGripPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	OdDbViewportPtr Viewport {entity};
	OdGsDCRectDouble Rectangle;
	const auto CenterPoint {Viewport->centerPoint()};
	auto Width {Viewport->width() * 0.5};
	auto Height {Viewport->height() * 0.5};
	Rectangle.m_min.x = CenterPoint.x - Width;
	Rectangle.m_min.y = CenterPoint.y - Height;
	Rectangle.m_max.x = CenterPoint.x + Width;
	Rectangle.m_max.y = CenterPoint.y + Height;
	double* logX[4] = {
		&Rectangle.m_min.x,
		&Rectangle.m_max.x,
		&Rectangle.m_max.x,
		&Rectangle.m_min.x
	};
	double* logY[4] = {
		&Rectangle.m_min.y,
		&Rectangle.m_min.y,
		&Rectangle.m_max.y,
		&Rectangle.m_max.y
	};
	bool Movement[4] = {false, false, false, false};
	bool* bMovX[4] = {Movement + 0, Movement + 2, Movement + 2, Movement + 0};
	bool* bMovY[4] = {Movement + 1, Movement + 1, Movement + 3, Movement + 3};
	for (auto Index : indices) {
		if (!*bMovX[Index]) {
			*logX[Index] += offset.x;
			*bMovX[Index] = true;
		}
		if (!*bMovY[Index]) {
			*logY[Index] += offset.y;
			*bMovY[Index] = true;
		}
	}
	if (Rectangle.m_min.x > Rectangle.m_max.x) {
		const auto tmp {Rectangle.m_min.x};
		Rectangle.m_min.x = Rectangle.m_max.x;
		Rectangle.m_max.x = tmp;
	}
	if (Rectangle.m_min.y > Rectangle.m_max.y) {
		const auto tmp {Rectangle.m_min.y};
		Rectangle.m_min.y = Rectangle.m_max.y;
		Rectangle.m_max.y = tmp;
	}
	const auto NewWidth {Rectangle.m_max.x - Rectangle.m_min.x};
	const auto NewHeight {Rectangle.m_max.y - Rectangle.m_min.y};
	const OdGePoint3d NewCenter(Rectangle.m_min.x + NewWidth * 0.5, Rectangle.m_min.y + NewHeight * 0.5, CenterPoint.z);
	if (OdNonZero(NewWidth) && OdNonZero(NewHeight) && !Viewport->isPerspectiveOn()) {
		Width *= 2;
		Height *= 2;
		OdAbstractViewPEPtr AbstractView(Viewport);
		const auto ZAxis {AbstractView->direction(Viewport)};
		const auto YAxis {AbstractView->upVector(Viewport)};
		const auto XAxis {YAxis.crossProduct(ZAxis).normal()};
		const auto FieldHeight {AbstractView->fieldHeight(Viewport) / Height * NewHeight};
		const auto FieldWidth {FieldHeight / Height * Width};
		const auto Difference {(NewCenter - CenterPoint) / NewHeight * FieldHeight};
		AbstractView->setView(Viewport, AbstractView->target(Viewport) + XAxis * Difference.x + YAxis * Difference.y, ZAxis, YAxis, FieldWidth, FieldHeight, false);
	}
	Viewport->setCenterPoint(NewCenter);
	Viewport->setWidth(NewWidth);
	Viewport->setHeight(NewHeight);
	return eOk;
}

OdResult OdDbViewportGripPointsPE::getStretchPoints(const OdDbEntity* entity, OdGePoint3dArray& stretchPoints) const {
	return getGripPoints(entity, stretchPoints);
}

OdResult OdDbViewportGripPointsPE::moveStretchPointsAt(OdDbEntity* entity, const OdIntArray& indices, const OdGeVector3d& offset) {
	return moveGripPointsAt(entity, indices, offset);
}

OdResult OdDbViewportGripPointsPE::getOsnapPoints(const OdDbEntity* /*entity*/, OdDb::OsnapMode /*objectSnapMode*/, OdGsMarker /*selectionMarker*/, const OdGePoint3d& /*pickPoint*/, const OdGePoint3d& /*lastPoint*/, const OdGeMatrix3d& /*worldToEyeTransform*/, OdGePoint3dArray& /*snapPoints*/) const {
	return eOk;
}
