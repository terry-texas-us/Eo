#pragma once

#include "EoGePoint4d.h"

class EoGsViewport {
	double m_DeviceHeightInPixels;
	double m_DeviceWidthInPixels;
	double m_DeviceHeightInInches;
	double m_DeviceWidthInInches;
	double m_HeightInPixels;
	double m_WidthInPixels;

public: // Constructors and destructors

	EoGsViewport() noexcept;
	EoGsViewport(const EoGsViewport& other) noexcept;

	~EoGsViewport();
	EoGsViewport& operator=(const EoGsViewport& other) noexcept;

public: // Methods
	/// <remarks> Window coordinates are rounded to nearest whole number.</remarks>
	CPoint DoProjection(const EoGePoint4d& pt) const noexcept;
	/// <remarks>Window coordinates are rounded to nearest whole number. Perspective division to yield normalized device coordinates.</remarks>
	void DoProjection(CPoint* pnt, int iPts, EoGePoint4d* pt) const noexcept;
	/// <remarks>Window coordinates are rounded to nearest whole number. Perspective division to yield normalized device coordinates.</remarks>
	void DoProjection(CPoint* pnt, EoGePoint4dArray& pointsArray) const;
	void DoProjectionInverse(OdGePoint3d& point) const noexcept;

	double HeightInPixels() const noexcept;
	double HeightInInches() const noexcept;
	double WidthInPixels() const noexcept;
	double WidthInInches() const noexcept;
	void SetDeviceHeightInInches(const double height) noexcept;
	void SetDeviceWidthInInches(const double width) noexcept;
	void SetSize(int width, int height) noexcept;
	void SetDeviceHeightInPixels(const double height) noexcept;
	void SetDeviceWidthInPixels(const double width) noexcept;
};
typedef CList<EoGsViewport> CViewports;
