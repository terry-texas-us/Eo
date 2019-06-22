#pragma once
#include "EoGePoint4d.h"

class EoGsViewport {
	double m_DeviceHeightInPixels {0.0};
	double m_DeviceWidthInPixels {0.0};
	double m_DeviceHeightInInches {0.0};
	double m_DeviceWidthInInches {0.0};
	double m_HeightInPixels {0.0};
	double m_WidthInPixels {0.0};
public: // Constructors and destructors
	EoGsViewport() = default;
	EoGsViewport(const EoGsViewport& other) noexcept;
	~EoGsViewport() = default;
	EoGsViewport& operator=(const EoGsViewport& other) = default;

	// Methods
	/// <remarks> Window coordinates are rounded to nearest whole number.</remarks>
	[[nodiscard]] CPoint DoProjection(const EoGePoint4d& pt) const noexcept;
	/// <remarks>Window coordinates are rounded to nearest whole number. Perspective division to yield normalized device coordinates.</remarks>
	void DoProjection(CPoint* pnt, int iPts, EoGePoint4d* pt) const noexcept;
	/// <remarks>Window coordinates are rounded to nearest whole number. Perspective division to yield normalized device coordinates.</remarks>
	void DoProjection(CPoint* pnt, EoGePoint4dArray& pointsArray) const;
	void DoProjectionInverse(OdGePoint3d& point) const noexcept;
	[[nodiscard]] double HeightInPixels() const noexcept;
	[[nodiscard]] double HeightInInches() const noexcept;
	[[nodiscard]] double WidthInPixels() const noexcept;
	[[nodiscard]] double WidthInInches() const noexcept;
	void SetDeviceHeightInInches(double height) noexcept;
	void SetDeviceWidthInInches(double width) noexcept;
	void SetSize(int width, int height) noexcept;
	void SetDeviceHeightInPixels(double height) noexcept;
	void SetDeviceWidthInPixels(double width) noexcept;
};

using CViewports = CList<EoGsViewport>;
