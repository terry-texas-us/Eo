#include <OdaCommon.h>
#include "DbDgnUnderlayGripPoints.h"
#include <DbUnderlayReference.h>
#include <DbUnderlayDefinition.h>
#include <Gi/GiDummyGeometry.h>
#include <Ge/GeNurbCurve3d.h>

OdResult OdDbDgnUnderlayGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker gsSelectionMark, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& xWorldToEye, OdGePoint3dArray& snapPoints) const {
	const auto DgnGripPointsModule {odrxDynamicLinker()->loadModule(ExDgnGripPointsModuleName)};
	if (DgnGripPointsModule.isNull()) {
		return eTxError;
	}
	const auto Result {OdDbUnderlayGripPointsPE::getOsnapPoints(entity, objectSnapMode, gsSelectionMark, pickPoint, lastPoint, xWorldToEye, snapPoints)};
	if (eOk == Result) {
		auto UnderlayReference {OdDbUnderlayReference::cast(entity)};
		OdDbUnderlayDefinitionPtr UnderlayDefinition {UnderlayReference->definitionId().openObject()};
		auto UnderlayItem {UnderlayDefinition->getUnderlayItem()};
		OdIntArray dummy;
		// NB: last parameter of this call needs to be changed to last parameter of this function
		return UnderlayItem->getOsnapPoints(UnderlayReference->transform(), objectSnapMode, gsSelectionMark, pickPoint, lastPoint, xWorldToEye, OdGeMatrix3d::kIdentity, snapPoints, dummy);
	}
	return Result;
}
