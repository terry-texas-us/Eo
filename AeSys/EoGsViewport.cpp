#include "stdafx.h"
#include "EoGsViewport.h"

EoGsViewport::EoGsViewport(const EoGsViewport& other) noexcept {
	m_DeviceHeightInPixels = other.m_DeviceHeightInPixels;
	m_DeviceWidthInPixels = other.m_DeviceWidthInPixels;
	m_DeviceHeightInInches = other.m_DeviceHeightInInches;
	m_DeviceWidthInInches = other.m_DeviceWidthInInches;
	m_HeightInPixels = other.m_HeightInPixels;
	m_WidthInPixels = other.m_WidthInPixels;
}

CPoint EoGsViewport::DoProjection(const EoGePoint4d& point) const noexcept {
	CPoint Point;
	Point.x = lround((point.x / point.W() + 1.0) * ((m_WidthInPixels - 1.0) / 2.0));
	Point.y = lround((-point.y / point.W() + 1.0) * ((m_HeightInPixels - 1.0) / 2.0));
	return Point;
}

void EoGsViewport::DoProjection(CPoint* pnt, const int numberOfPoints, EoGePoint4d* points) const noexcept {
	for (auto PointIndex = 0; PointIndex < numberOfPoints; PointIndex++) {
		pnt[PointIndex] = DoProjection(points[PointIndex]);
	}
}

void EoGsViewport::DoProjection(CPoint* pnt, EoGePoint4dArray& points) const {
	const auto numberOfPoints {points.GetSize()};
	for (auto PointIndex = 0; PointIndex < numberOfPoints; PointIndex++) {
		pnt[PointIndex] = DoProjection(points[PointIndex]);
	}
}

void EoGsViewport::DoProjectionInverse(OdGePoint3d& point) const noexcept {
	point.x = point.x * 2. / (m_WidthInPixels - 1.0) - 1.0;
	point.y = -(point.y * 2. / (m_HeightInPixels - 1.0) - 1.0);
}

double EoGsViewport::HeightInPixels() const noexcept {
	return m_HeightInPixels;
}

double EoGsViewport::HeightInInches() const noexcept {
	return m_HeightInPixels / (m_DeviceHeightInPixels / m_DeviceHeightInInches);
}

double EoGsViewport::WidthInPixels() const noexcept {
	return m_WidthInPixels;
}

double EoGsViewport::WidthInInches() const noexcept {
	return m_WidthInPixels / (m_DeviceWidthInPixels / m_DeviceWidthInInches);
}

void EoGsViewport::SetDeviceHeightInInches(const double height) noexcept {
	m_DeviceHeightInInches = height;
}

void EoGsViewport::SetDeviceWidthInInches(const double width) noexcept {
	m_DeviceWidthInInches = width;
}

void EoGsViewport::SetSize(const int width, const int height) noexcept {
	m_WidthInPixels = static_cast<double>(width);
	m_HeightInPixels = static_cast<double>(height);
}

void EoGsViewport::SetDeviceHeightInPixels(const double height) noexcept {
	m_DeviceHeightInPixels = height;
}

void EoGsViewport::SetDeviceWidthInPixels(const double width) noexcept {
	m_DeviceWidthInPixels = width;
}
