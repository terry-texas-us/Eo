#pragma once

#include "EoGeReferenceSystem.h"

class EoGeMatrix3d : public OdGeMatrix3d {
public: // Constructors and destructor
	EoGeMatrix3d();
	virtual ~EoGeMatrix3d();

public: // Methods
	/// <summary>Builds rotation transformation matrices.</summary>
	/// <remarks>Angles (in degrees) for each axis</remarks>
	EoGeMatrix3d& SetTo3AxisRotation(const OdGeVector3d& rotationAngles);
	EoGeMatrix3d& SetToPerspectiveProjection(double uMin, double uMax, double vMin, double vMax, double nearClipDistance, double farClipDistance);
	EoGeMatrix3d& SetToParallelProjection(double uMin, double uMax, double vMin, double vMax, double nearClipDistance, double farClipDistance);

	EoGeMatrix3d& SetToViewTransform(const OdGePoint3d position, const OdGePoint3d target, const OdGeVector3d& viewUp);

public: // Methods - static
	static OdGeMatrix3d ReferenceSystemToWorld(const EoGeReferenceSystem& referenceSystem);
};

typedef CList<EoGeMatrix3d> EoGeMatrix3dList;
