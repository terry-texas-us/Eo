#pragma once

#include "EoGsAbstractView.h"

class EoGsViewTransform : public EoGsAbstractView {
	EoGeMatrix3d m_Matrix;
	EoGeMatrix3d m_ViewMatrix;
	EoGeMatrix3d m_ProjectionMatrix;

public: // Constructors and destructor
	EoGsViewTransform();
	EoGsViewTransform(const EoGsViewTransform& other);

	~EoGsViewTransform();
public: // Operators
	EoGsViewTransform& operator=(const EoGsViewTransform& other);

public: // Methods
	// View space, sometimes called camera space, is similar to world space in that it is typically used for the entire scene.
	// However, in view space, the origin is at the viewer or camera. The view direction (where the viewer is looking) defines the positive Z axis.
	// An "up" direction defined by the application becomes the positive Y axis.
	void BuildTransformMatrix();
	EoGeMatrix3d Matrix() const;
};

typedef CList<EoGsViewTransform> EoGsViewTransforms;
