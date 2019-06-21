#include "stdafx.h"
#include "AeSys.h"
#include "AeSysView.h"

void AeSysView::InitializeConstraints() noexcept {
	m_AxisConstraintInfluenceAngle = 5.;
	m_AxisConstraintOffsetAngle = 0.0;
	m_XGridSnapSpacing = 1.0;
	m_YGridSnapSpacing = 1.0;
	m_ZGridSnapSpacing = 1.0;
	m_XGridLineSpacing = 12.0;
	m_YGridLineSpacing = 12.0;
	m_ZGridLineSpacing = 12.0;
	m_XGridPointSpacing = 3.0;
	m_YGridPointSpacing = 3.0;
	m_ZGridPointSpacing = 0.0;
	m_MaximumDotsPerLine = 64;
	m_DisplayGridWithLines = false;
	m_DisplayGridWithPoints = false;
	m_GridSnap = false;
}

double AeSysView::AxisConstraintInfluenceAngle() const noexcept {
	return m_AxisConstraintInfluenceAngle;
}

void AeSysView::SetAxisConstraintInfluenceAngle(double angle) noexcept {
	m_AxisConstraintInfluenceAngle = angle;
}

double AeSysView::AxisConstraintOffsetAngle() const noexcept {
	return m_AxisConstraintOffsetAngle;
}

void AeSysView::SetAxisConstraintOffsetAngle(double angle) noexcept {
	m_AxisConstraintOffsetAngle = angle;
}

OdGePoint3d AeSysView::GridOrigin() const noexcept {
	return m_GridOrigin;
}

void AeSysView::SetGridOrigin(const OdGePoint3d& origin) noexcept {
	m_GridOrigin = origin;
}

bool AeSysView::DisplayGridWithLines() const noexcept {
	return m_DisplayGridWithLines;
}

void AeSysView::EnableDisplayGridWithLines(bool display) noexcept {
	m_DisplayGridWithLines = display;
}

void AeSysView::EnableDisplayGridWithPoints(bool display) noexcept {
	m_DisplayGridWithPoints = display;
}

bool AeSysView::DisplayGridWithPoints() const noexcept {
	return m_DisplayGridWithPoints;
}

bool AeSysView::GridSnap() const noexcept {
	return m_GridSnap;
}

void AeSysView::EnableGridSnap(bool snap) noexcept {
	m_GridSnap = snap;
}

void AeSysView::GetGridLineSpacing(double& x, double& y, double& z) noexcept {
	x = m_XGridLineSpacing;
	y = m_YGridLineSpacing;
	z = m_ZGridLineSpacing;
}

void AeSysView::SetGridLineSpacing(double x, double y, double z) noexcept {
	m_XGridLineSpacing = x;
	m_YGridLineSpacing = y;
	m_ZGridLineSpacing = z;
}

void AeSysView::GetGridPointSpacing(double& x, double& y, double& z) noexcept {
	x = m_XGridPointSpacing;
	y = m_YGridPointSpacing;
	z = m_ZGridPointSpacing;
}

void AeSysView::SetGridPointSpacing(double x, double y, double z) noexcept {
	m_XGridPointSpacing = x;
	m_YGridPointSpacing = y;
	m_ZGridPointSpacing = z;
}

void AeSysView::GetGridSnapSpacing(double& x, double& y, double& z) noexcept {
	x = m_XGridSnapSpacing;
	y = m_YGridSnapSpacing;
	z = m_ZGridSnapSpacing;
}

void AeSysView::SetGridSnapSpacing(double x, double y, double z) noexcept {
	m_XGridSnapSpacing = x;
	m_YGridSnapSpacing = y;
	m_ZGridSnapSpacing = z;
}

void AeSysView::DisplayGrid(CDC* deviceContext) {
	const auto dHalfPts {m_MaximumDotsPerLine * 0.5};
	if (DisplayGridWithPoints()) {
		OdGePoint3d pt;
		if (fabs(m_YGridPointSpacing) > DBL_EPSILON && fabs(m_ZGridPointSpacing) > DBL_EPSILON) {
			const auto Color {theApp.GetHotColor(1)};
			pt.x = m_GridOrigin.x;
			pt.z = m_GridOrigin.z - dHalfPts * m_ZGridPointSpacing;
			for (auto i = 0; i < m_MaximumDotsPerLine; i++) {
				pt.y = m_GridOrigin.y - dHalfPts * m_YGridPointSpacing;
				for (auto i2 = 0; i2 < m_MaximumDotsPerLine; i2++) {
					DisplayPixel(deviceContext, Color, pt);
					pt.y += m_YGridPointSpacing;
				}
				pt.z += m_ZGridPointSpacing;
			}
		}
		if (fabs(m_XGridPointSpacing) > DBL_EPSILON && fabs(m_ZGridPointSpacing) > DBL_EPSILON) {
			const auto Color {theApp.GetHotColor(2)};
			pt.x = m_GridOrigin.x - dHalfPts * m_XGridPointSpacing;
			pt.y = m_GridOrigin.y;
			for (auto i = 0; i < m_MaximumDotsPerLine; i++) {
				pt.z = m_GridOrigin.z - dHalfPts * m_ZGridPointSpacing;
				for (auto i2 = 0; i2 < m_MaximumDotsPerLine; i2++) {
					DisplayPixel(deviceContext, Color, pt);
					pt.z += m_ZGridPointSpacing;
				}
				pt.x += m_XGridPointSpacing;
			}
		}
		if (fabs(m_XGridPointSpacing) > DBL_EPSILON && fabs(m_YGridPointSpacing) > DBL_EPSILON) {
			const auto Color {theApp.GetHotColor(3)};
			pt.y = m_GridOrigin.y - dHalfPts * m_YGridPointSpacing;
			pt.z = m_GridOrigin.z;
			for (auto i = 0; i < m_MaximumDotsPerLine; i++) {
				pt.x = m_GridOrigin.x - dHalfPts * m_XGridPointSpacing;
				for (auto i2 = 0; i2 < m_MaximumDotsPerLine; i2++) {
					DisplayPixel(deviceContext, Color, pt);
					pt.x += m_XGridPointSpacing;
				}
				pt.y += m_YGridPointSpacing;
			}
		}
	}
	if (DisplayGridWithLines()) {
		if (fabs(m_XGridLineSpacing) > DBL_EPSILON && fabs(m_YGridLineSpacing) > DBL_EPSILON) {
			int i;
			const auto ColorIndex {g_PrimitiveState.ColorIndex()};
			const auto LinetypeIndex {g_PrimitiveState.LinetypeIndex()};
			g_PrimitiveState.SetPen(this, deviceContext, 250, 1);
			OdGePoint3d StartPoint;
			OdGePoint3d EndPoint;
			StartPoint.x = m_GridOrigin.x - dHalfPts * m_XGridLineSpacing;
			EndPoint.x = m_GridOrigin.x + dHalfPts * m_XGridLineSpacing;
			StartPoint.y = m_GridOrigin.y - dHalfPts * m_YGridLineSpacing;
			StartPoint.z = m_GridOrigin.z;
			EndPoint.z = m_GridOrigin.z;
			for (i = 0; i < m_MaximumDotsPerLine; i++) {
				EndPoint.y = StartPoint.y;
				EoGeLineSeg3d(StartPoint, EndPoint).Display(this, deviceContext);
				StartPoint.y += m_YGridLineSpacing;
			}
			StartPoint.y = m_GridOrigin.y - dHalfPts * m_YGridLineSpacing;
			EndPoint.y = m_GridOrigin.y + dHalfPts * m_YGridLineSpacing;
			for (i = 0; i < m_MaximumDotsPerLine; i++) {
				EndPoint.x = StartPoint.x;
				EoGeLineSeg3d(StartPoint, EndPoint).Display(this, deviceContext);
				StartPoint.x += m_XGridLineSpacing;
			}
			g_PrimitiveState.SetPen(this, deviceContext, ColorIndex, LinetypeIndex);
		}
	}
}

OdGePoint3d AeSysView::SnapPointToAxis(const OdGePoint3d& startPoint, const OdGePoint3d& endPoint) {
	EoGeLineSeg3d Line(startPoint, endPoint);
	return Line.ConstrainToAxis(m_AxisConstraintInfluenceAngle, m_AxisConstraintOffsetAngle);
}

OdGePoint3d AeSysView::SnapPointToGrid(const OdGePoint3d& point) noexcept {
	auto pt {point};
	if (GridSnap()) {
		if (fabs(m_XGridSnapSpacing) > DBL_EPSILON) {
			pt.x -= fmod(point.x - m_GridOrigin.x, m_XGridSnapSpacing);
			if (fabs(pt.x - point.x) > m_XGridSnapSpacing * 0.5) pt.x += EoSignTransfer(m_XGridSnapSpacing, point.x - m_GridOrigin.x);
		}
		if (fabs(m_YGridSnapSpacing) > DBL_EPSILON) {
			pt.y -= fmod(point.y - m_GridOrigin.y, m_YGridSnapSpacing);
			if (fabs(pt.y - point.y) > m_YGridSnapSpacing * 0.5) pt.y += EoSignTransfer(m_YGridSnapSpacing, point.y - m_GridOrigin.y);
		}
		if (fabs(m_ZGridSnapSpacing) > DBL_EPSILON) {
			pt.z -= fmod(point.z - m_GridOrigin.z, m_ZGridSnapSpacing);
			if (fabs(pt.z - point.z) > m_ZGridSnapSpacing * 0.5) pt.z += EoSignTransfer(m_ZGridSnapSpacing, point.z - m_GridOrigin.z);
		}
	}
	return pt;
}
