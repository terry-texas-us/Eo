#pragma once

class EoGsViewport {
	double m_DeviceHeightInPixels;
	double m_DeviceWidthInPixels;
	double m_DeviceHeightInInches;
	double m_DeviceWidthInInches;
	double m_HeightInPixels;
	double m_WidthInPixels;

public: // Constructors and destructors

	EoGsViewport();
	EoGsViewport(const EoGsViewport& other);

	~EoGsViewport();
	EoGsViewport& operator=(const EoGsViewport& other);

public: // Methods
	/// <remarks> Window coordinates are rounded to nearest whole number.</remarks>
	CPoint DoProjection(const EoGePoint4d& pt) const;
	/// <remarks>Window coordinates are rounded to nearest whole number. Perspective division to yield normalized device coordinates.</remarks>
	void DoProjection(CPoint* pnt, int iPts, EoGePoint4d* pt) const;
	/// <remarks>Window coordinates are rounded to nearest whole number. Perspective division to yield normalized device coordinates.</remarks>
	void DoProjection(CPoint* pnt, EoGePoint4dArray& pointsArray) const;
	void DoProjectionInverse(OdGePoint3d& point) const;

	double HeightInPixels() const;
	double HeightInInches() const;
	double WidthInPixels() const;
	double WidthInInches() const;
	void SetDeviceHeightInInches(const double height);
	void SetDeviceWidthInInches(const double width);
	void SetSize(int width, int height);
	void SetDeviceHeightInPixels(const double height);
	void SetDeviceWidthInPixels(const double width);
};
typedef CList<EoGsViewport> CViewports;
