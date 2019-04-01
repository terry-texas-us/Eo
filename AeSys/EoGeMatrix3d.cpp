#include "stdafx.h"

EoGeMatrix3d::EoGeMatrix3d()
	: OdGeMatrix3d() {
}
EoGeMatrix3d::~EoGeMatrix3d() {
}

// Methods
EoGeMatrix3d& EoGeMatrix3d::SetTo3AxisRotation(const OdGeVector3d& rotationAngles) {
	setToIdentity();
	EoGeMatrix3d RotationMatrix;
	if (fabs(rotationAngles.x) != 0.) {
		RotationMatrix.setToRotation(EoArcLength(rotationAngles.x), OdGeVector3d::kXAxis);
		preMultBy(RotationMatrix);
	}
	if (fabs(rotationAngles.y) != 0.) {
		RotationMatrix.setToRotation(EoArcLength(rotationAngles.y), OdGeVector3d::kYAxis);
		preMultBy(RotationMatrix);
	}
	if (fabs(rotationAngles.z) != 0.) {
		RotationMatrix.setToRotation(EoArcLength(rotationAngles.z), OdGeVector3d::kZAxis);
		preMultBy(RotationMatrix);
	}
	return *this;
}
EoGeMatrix3d& EoGeMatrix3d::SetToParallelProjection(double uMin, double uMax, double vMin, double vMax, double nearClipDistance, double farClipDistance) {
	setToIdentity();

	entry[0][0] = 2. / (uMax - uMin);
	entry[0][3] = - (uMax + uMin) / (uMax - uMin);

	entry[1][1] = 2. / (vMax - vMin);
	entry[1][3] = - (vMax + vMin) / (vMax - vMin);

	entry[2][2] = - 2. / (farClipDistance - nearClipDistance);
	entry[2][3] = - (farClipDistance + nearClipDistance) / (farClipDistance - nearClipDistance);

	return *this;
}
EoGeMatrix3d& EoGeMatrix3d::SetToPerspectiveProjection(double uMin, double uMax, double vMin, double vMax, double nearClipDistance, double farClipDistance) {
	setToIdentity();

	double FieldWidth = uMax - uMin;
	double FieldHeight = vMax - vMin;
	double NExtent = farClipDistance - nearClipDistance;

	entry[0][0] = 2. * nearClipDistance / FieldWidth;
	entry[0][2] = (uMax + uMin) / FieldWidth;

	entry[1][1] = (2. * nearClipDistance) / FieldHeight;
	entry[1][2] = (vMax + vMin) / FieldHeight;

	entry[2][2] = - (farClipDistance + nearClipDistance) / NExtent;
	entry[2][3] = - 2. * farClipDistance * nearClipDistance / NExtent;

	entry[3][2] = - 1.;
	entry[3][3] = 0.;

	return *this;
}
EoGeMatrix3d& EoGeMatrix3d::SetToViewTransform(const OdGePoint3d position, const OdGePoint3d target, const OdGeVector3d& viewUp) {
	setToIdentity();

	OdGeVector3d Normal = position - target;
	Normal.normalize();

	OdGeVector3d vU = viewUp.crossProduct(Normal);

	vU.normalize();

	OdGeVector3d vV = Normal.crossProduct(vU);
	vV.normalize();

	const OdGeVector3d PositionAsVector = - position.asVector();

	entry[0][0] = vU.x;
	entry[0][1] = vU.y;
	entry[0][2] = vU.z;
	entry[0][3] = PositionAsVector.dotProduct(vU);

	entry[1][0] = vV.x;
	entry[1][1] = vV.y;
	entry[1][2] = vV.z;
	entry[1][3] = PositionAsVector.dotProduct(vV);

	entry[2][0] = Normal.x;
	entry[2][1] = Normal.y;
	entry[2][2] = Normal.z;
	entry[2][3] = PositionAsVector.dotProduct(Normal);

	return *this;
}

OdGeMatrix3d EoGeMatrix3d::ReferenceSystemToWorld(const EoGeReferenceSystem& referenceSystem) {
	OdGeMatrix3d ScaleMatrix;
	const double XDirectionLength = referenceSystem.XDirection().length();
	const double YDirectionLength = referenceSystem.YDirection().length();
	if (XDirectionLength > DBL_EPSILON && YDirectionLength > DBL_EPSILON) {
		ScaleMatrix.setToScaling(OdGeScale3d(1. / XDirectionLength, 1. / YDirectionLength, 1.));

		OdGeMatrix3d ToWorld(referenceSystem.TransformMatrix());
		ToWorld.preMultBy(ScaleMatrix);
		ToWorld.invert();

		return ToWorld;
	}
	return OdGeMatrix3d::kIdentity;
}

