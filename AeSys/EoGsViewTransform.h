#pragma once
#include "EoGeMatrix3d.h"
#include "EoGsAbstractView.h"

class EoGsViewTransform final : public EoGsAbstractView {
	EoGeMatrix3d m_Matrix;
	EoGeMatrix3d m_ViewMatrix;
	EoGeMatrix3d m_ProjectionMatrix;
public:
	EoGsViewTransform() = default;

	EoGsViewTransform(const EoGsViewTransform& other);

	EoGsViewTransform& operator=(const EoGsViewTransform& other) = default;

	~EoGsViewTransform() = default;

	// View space, sometimes called camera space, is similar to world space in that it is typically used for the entire scene.
	// However, in view space, the origin is at the viewer or camera. The view direction (where the viewer is looking) defines the positive Z axis.
	// An "up" direction defined by the application becomes the positive Y axis.
	void BuildTransformMatrix();

	[[nodiscard]] EoGeMatrix3d Matrix() const noexcept;
};

using EoGsViewTransforms = CList<EoGsViewTransform>;
