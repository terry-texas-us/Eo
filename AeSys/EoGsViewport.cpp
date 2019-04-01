#include "stdafx.h"

EoGsViewport::EoGsViewport()
    : m_DeviceHeightInPixels(0.)
    , m_DeviceWidthInPixels(0.)
    , m_DeviceHeightInInches(0.)
    , m_DeviceWidthInInches(0.)
    , m_HeightInPixels(0.)
    , m_WidthInPixels(0.) {
}

EoGsViewport::~EoGsViewport() {
};

EoGsViewport::EoGsViewport(const EoGsViewport& viewport) {
	m_DeviceHeightInPixels = viewport.m_DeviceHeightInPixels;
	m_DeviceWidthInPixels = viewport.m_DeviceWidthInPixels;
	m_DeviceHeightInInches = viewport.m_DeviceHeightInInches;
	m_DeviceWidthInInches = viewport.m_DeviceWidthInInches;
	m_HeightInPixels = viewport.m_HeightInPixels;
	m_WidthInPixels = viewport.m_WidthInPixels;
}

EoGsViewport& EoGsViewport::operator=(const EoGsViewport& viewport) {
	m_DeviceHeightInPixels = viewport.m_DeviceHeightInPixels;
	m_DeviceWidthInPixels = viewport.m_DeviceWidthInPixels;
	m_WidthInPixels = viewport.m_WidthInPixels;
	m_HeightInPixels = viewport.m_HeightInPixels;
	m_DeviceHeightInInches = viewport.m_DeviceHeightInInches;
	m_DeviceWidthInInches = viewport.m_DeviceWidthInInches;

	return *this;
}
CPoint EoGsViewport::DoProjection(const EoGePoint4d& point) const {
	CPoint pnt;
	
	pnt.x = EoRound((point.x / point.W() + 1.) * ((m_WidthInPixels - 1.) / 2.));
	pnt.y = EoRound((- point.y / point.W() + 1.) * ((m_HeightInPixels - 1.) / 2.));

	return pnt;
}
void EoGsViewport::DoProjection(CPoint* pnt, int numberOfPoints, EoGePoint4d* points) const {
	for (int PointIndex = 0; PointIndex < numberOfPoints; PointIndex++) {
		pnt[PointIndex] = DoProjection(points[PointIndex]);
	}
}
void EoGsViewport::DoProjection(CPoint* pnt, EoGePoint4dArray& points) const {
	const int numberOfPoints = (int) points.GetSize();

	for (int PointIndex = 0; PointIndex < numberOfPoints; PointIndex++) {
		pnt[PointIndex] = DoProjection(points[PointIndex]);
	}
}
void EoGsViewport::DoProjectionInverse(OdGePoint3d& point) const {
	point.x = (point.x * 2.) / (m_WidthInPixels - 1.) - 1.;
	point.y = - ((point.y * 2.) / (m_HeightInPixels - 1.) - 1.);
}
double EoGsViewport::HeightInPixels() const {
	return m_HeightInPixels;
}
double EoGsViewport::HeightInInches() const {
	return m_HeightInPixels / (m_DeviceHeightInPixels / m_DeviceHeightInInches);
}
double EoGsViewport::WidthInPixels() const {
	return m_WidthInPixels;
}
double EoGsViewport::WidthInInches() const {
	return m_WidthInPixels / (m_DeviceWidthInPixels / m_DeviceWidthInInches);
}
void EoGsViewport::SetDeviceHeightInInches(const double height) {
	m_DeviceHeightInInches = height;
}
void EoGsViewport::SetDeviceWidthInInches(const double width) {
	m_DeviceWidthInInches = width;
}
void EoGsViewport::SetSize(int width, int height) {
	m_WidthInPixels = static_cast<double>(width);
	m_HeightInPixels = static_cast<double>(height);
}
void EoGsViewport::SetDeviceHeightInPixels(const double height) {
	m_DeviceHeightInPixels = height;
}
void EoGsViewport::SetDeviceWidthInPixels(const double width) {
	m_DeviceWidthInPixels = width;
}
