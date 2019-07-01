#pragma once
class EoGeReferenceSystem;

class EoGeMatrix3d final : public OdGeMatrix3d {
public:
	/// <summary>Builds rotation transformation matrices.</summary>
	/// <remarks>Angles (in degrees) for each axis</remarks>
	EoGeMatrix3d& SetTo3AxisRotation(const OdGeVector3d& rotationAngles);

	EoGeMatrix3d& SetToPerspectiveProjection(double uMin, double uMax, double vMin, double vMax, double nearClipDistance, double farClipDistance);

	EoGeMatrix3d& SetToParallelProjection(double uMin, double uMax, double vMin, double vMax, double nearClipDistance, double farClipDistance);

	EoGeMatrix3d& SetToViewTransform(OdGePoint3d position, OdGePoint3d target, const OdGeVector3d& viewUp);

	static OdGeMatrix3d ReferenceSystemToWorld(const EoGeReferenceSystem& referenceSystem);
};

using EoGeMatrix3dList = CList<EoGeMatrix3d>;
