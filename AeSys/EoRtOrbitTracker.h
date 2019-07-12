#pragma once
#include "EoOrbitCtrl.h"
#include <DbDatabase.h>
#include <DbObjectId.h>
#include <DbViewportTableRecord.h>
#include <ExTrackers.h>
#include <Ge/GeBoundBlock3d.h>
#include <Ge/GeExtents3d.h>
#include <Gs/GsModel.h>

class RtOrbitTracker : public OdEdPointTracker {
	OdGsView* m_View {nullptr};
	OdGePoint3d m_Point;
	OdGiDrawablePtr m_Drawable;
	OdGePoint3d m_Position;
	OdGePoint3d m_Target;
	OdGeVector3d m_UpVector;
	OdGeVector3d m_X;
	OdGePoint3d m_ViewCenter;
	OdGeMatrix3d m_InitialViewingMatrixInverted;
	double m_D {0.0}; // diameter of orbit control in projected coordinates
	OdGsModelPtr m_Model;

	enum Axis {
		kHorizontal, kVertical, kPerpDir, // orbit around perpendicular to mouse direction
		kEye,
	} m_Axis {kHorizontal};

	void ViewportDcCorners(OdGePoint2d& lowerLeft, OdGePoint2d& upperRight) const;

public:
	RtOrbitTracker() = default;

	void Reset() noexcept { m_View = nullptr; }

	void Initialize(OdGsView* view, const OdGePoint3d& pt);

	[[nodiscard]] double Angle(const OdGePoint3d& value) const;

	[[nodiscard]] double AngleZ(const OdGePoint3d& value) const;

	[[nodiscard]] double AnglePerpendicular(const OdGePoint3d& value) const;

	void setValue(const OdGePoint3d& value) override;

	int addDrawables(OdGsView* pView) override;

	void removeDrawables(OdGsView* pView) override;
};

