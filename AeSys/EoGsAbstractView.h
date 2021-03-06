#pragma once
#include <Gs/Gs.h>

/*
The view target and the direction create a display coordinate system (DCS).
The z-direction equals the direction from target to camera (points toward viewer)
The DCS x-direction is calculated as cross product of (0,0,1) and the view direction.
If the view direction is parallel to (0,0,1), the x-direction is (1,0,0).
The DCS origin is the target point.

Parallel projections:
	The view may be offset from the line of sight (target point may be off the screen)
	A view need additional data to define DCS rectangle to be displayed. The rectangle's
	center is a 2d point (located on the DCS xy plane). Its size is given by its height and width;
	its rotation around its center is given be the twist angle.

Perspective projections:
	The DCS rectangle is always centered around the target point (DCS origin).
	Its size is taken from the views length. Start with a rectangle of 42 units diagonal length
	located at lens length distance from camera point.
	By dividing 42 times the view distance by the lens length, getting the diagonal length of the DCS rectangle.
	Next, select a rectangle with the same proportions as the views width and height, and rotate this by view twist angle.
*/
class EoGsAbstractView {
public:
	static const unsigned mc_Perspective = 0x01; // bit 1 Perspective mode flag for this view
	static const unsigned mc_NearClipping = 0x02; // bit 2 Near (Front) clipping plane status for this view
	static const unsigned mc_FarClipping = 0x04; // bit 3 Far (Back) clipping plane status for this view
	static const unsigned mc_NearClippingAtEye = 0x10; // bit 16 Front clipping plane is located at the camera
protected:
	unsigned m_ViewMode {0}; // Parallel projection, No front clipping, No back clipping
	OdGsView::RenderMode m_RenderMode {OdGsView::k2DOptimized};
	double m_Elevation {0.0}; // elevation of the UCS plane for this view
	double m_LensLength {50.0}; // lens length used for perspective mode in this view
	OdGePoint3d m_Position {OdGePoint3d::kOrigin + OdGeVector3d::kZAxis * m_LensLength};
	OdGePoint3d m_Target {OdGePoint3d::kOrigin};
	OdGeVector3d m_ViewUp {OdGeVector3d::kYAxis};
	double m_TwistAngle {0.0}; // in radians
	// View-Specific coordinate systems
	double m_FieldWidthMinimum {-0.5};
	double m_FieldHeightMinimum {-0.5};
	double m_FieldWidthMaximum {0.5};
	double m_FieldHeightMaximum {0.5};
	double m_NearClipDistance {20.0}; // distance from the target to the near (front) clipping plane along the target-camera line.
	double m_FarClipDistance {100.0}; // distance from the target to the far (back) clipping plane along the target-camera line
public:
	EoGsAbstractView();

	EoGsAbstractView(const EoGsAbstractView& other);

	EoGsAbstractView& operator=(const EoGsAbstractView& other) = default;

	virtual ~EoGsAbstractView() = default;

	void AdjustWindow(double aspectRatio) noexcept;

	void EnablePerspective(bool enabled) noexcept;

	[[nodiscard]] double FarClipDistance() const noexcept;

	[[nodiscard]] double FieldHeight() const noexcept;

	[[nodiscard]] double FieldHeightMaximum() const noexcept;

	[[nodiscard]] double FieldHeightMinimum() const noexcept;

	[[nodiscard]] double FieldWidth() const noexcept;

	[[nodiscard]] double FieldWidthMaximum() const noexcept;

	[[nodiscard]] double FieldWidthMinimum() const noexcept;

	[[nodiscard]] bool IsFarClipOn() const noexcept;

	[[nodiscard]] bool IsNearClipAtEyeOn() const noexcept;

	[[nodiscard]] bool IsNearClipOn() const noexcept;

	[[nodiscard]] bool IsPerspectiveOn() const noexcept;

	[[nodiscard]] double LensLength() const noexcept;

	[[nodiscard]] double NearClipDistance() const noexcept;

	[[nodiscard]] OdGePoint3d Position() const noexcept;

	[[nodiscard]] OdGsView::RenderMode RenderMode() const noexcept;

	void SetFarClipDistance(double distance) noexcept;

	void SetLensLength(double length) noexcept;

	void SetNearClipDistance(double distance) noexcept;

	void SetPosition_(const OdGePoint3d& position) noexcept;

	void SetProjectionPlaneField(double fieldWidth, double fieldHeight) noexcept;

	void SetProjectionPlaneField(double uMin, double vMin, double uMax, double vMax) noexcept;

	void SetRenderMode(const OdGsView::RenderMode& renderMode) noexcept;

	void SetTarget(const OdGePoint3d& target) noexcept;

	void SetView(const OdGePoint3d& position, const OdGePoint3d& target, const OdGeVector3d& viewUp, double fieldWidth, double fieldHeight);

	void SetViewUp(const OdGeVector3d& viewUp);

	[[nodiscard]] OdGePoint3d Target() const noexcept;

	[[nodiscard]] OdGeVector3d ViewUp() const noexcept;
};
