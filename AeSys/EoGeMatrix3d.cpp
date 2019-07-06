#include "stdafx.h"
#include <Ge/GeScale3d.h>
#include "EoGeReferenceSystem.h"
#include "EoGeMatrix3d.h"

EoGeMatrix3d& EoGeMatrix3d::SetTo3AxisRotation(const OdGeVector3d& rotationAngles) {
	setToIdentity();
	EoGeMatrix3d RotationMatrix;
	if (fabs(rotationAngles.x) != 0.0) {
		RotationMatrix.setToRotation(EoArcLength(rotationAngles.x), OdGeVector3d::kXAxis);
		preMultBy(RotationMatrix);
	}
	if (fabs(rotationAngles.y) != 0.0) {
		RotationMatrix.setToRotation(EoArcLength(rotationAngles.y), OdGeVector3d::kYAxis);
		preMultBy(RotationMatrix);
	}
	if (fabs(rotationAngles.z) != 0.0) {
		RotationMatrix.setToRotation(EoArcLength(rotationAngles.z), OdGeVector3d::kZAxis);
		preMultBy(RotationMatrix);
	}
	return *this;
}

EoGeMatrix3d& EoGeMatrix3d::SetToParallelProjection(const double uMin, const double uMax, const double vMin, const double vMax, const double nearClipDistance, const double farClipDistance) {
	setToIdentity();
	entry[0][0] = 2.0 / (uMax - uMin);
	entry[0][3] = - (uMax + uMin) / (uMax - uMin);
	entry[1][1] = 2.0 / (vMax - vMin);
	entry[1][3] = - (vMax + vMin) / (vMax - vMin);
	entry[2][2] = - 2.0 / (farClipDistance - nearClipDistance);
	entry[2][3] = - (farClipDistance + nearClipDistance) / (farClipDistance - nearClipDistance);
	return *this;
}

EoGeMatrix3d& EoGeMatrix3d::SetToPerspectiveProjection(const double uMin, const double uMax, const double vMin, const double vMax, const double nearClipDistance, const double farClipDistance) {
	setToIdentity();
	const auto FieldWidth {uMax - uMin};
	const auto FieldHeight {vMax - vMin};
	const auto NExtent {farClipDistance - nearClipDistance};
	entry[0][0] = 2.0 * nearClipDistance / FieldWidth;
	entry[0][2] = (uMax + uMin) / FieldWidth;
	entry[1][1] = 2.0 * nearClipDistance / FieldHeight;
	entry[1][2] = (vMax + vMin) / FieldHeight;
	entry[2][2] = - (farClipDistance + nearClipDistance) / NExtent;
	entry[2][3] = - 2.0 * farClipDistance * nearClipDistance / NExtent;
	entry[3][2] = - 1.0;
	entry[3][3] = 0.0;
	return *this;
}

EoGeMatrix3d& EoGeMatrix3d::SetToViewTransform(OdGePoint3d position, const OdGePoint3d target, const OdGeVector3d& viewUp) {
	setToIdentity();
	auto Normal {position - target};
	Normal.normalize();
	auto U {viewUp.crossProduct(Normal)};
	U.normalize();
	auto V {Normal.crossProduct(U)};
	V.normalize();
	const auto PositionAsVector {-position.asVector()};
	entry[0][0] = U.x;
	entry[0][1] = U.y;
	entry[0][2] = U.z;
	entry[0][3] = PositionAsVector.dotProduct(U);
	entry[1][0] = V.x;
	entry[1][1] = V.y;
	entry[1][2] = V.z;
	entry[1][3] = PositionAsVector.dotProduct(V);
	entry[2][0] = Normal.x;
	entry[2][1] = Normal.y;
	entry[2][2] = Normal.z;
	entry[2][3] = PositionAsVector.dotProduct(Normal);
	return *this;
}

OdGeMatrix3d EoGeMatrix3d::ReferenceSystemToWorld(const EoGeReferenceSystem& referenceSystem) {
	OdGeMatrix3d ScaleMatrix;
	const auto XDirectionLength {referenceSystem.XDirection().length()};
	const auto YDirectionLength {referenceSystem.YDirection().length()};
	if (XDirectionLength > DBL_EPSILON && YDirectionLength > DBL_EPSILON) {
		ScaleMatrix.setToScaling(OdGeScale3d(1.0 / XDirectionLength, 1.0 / YDirectionLength, 1.0));
		OdGeMatrix3d ToWorld(referenceSystem.TransformMatrix());
		ToWorld.preMultBy(ScaleMatrix);
		ToWorld.invert();
		return ToWorld;
	}
	return kIdentity;
}
