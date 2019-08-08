#include <OdaCommon.h>
#include "DbPdfUnderlayGripPoints.h"
#include <DbUnderlayReference.h>
#include <Gi/GiDummyGeometry.h>
#include <Ge/GeNurbCurve3d.h>
#include <Ge/GeCircArc3d.h>
#include <StaticRxObject.h>
#include "../Extensions/PdfUnderlayCommon/PdfUnderlay.h"
#include <DbUnderlayDefinition.h>

OdResult OdDbPdfUnderlayGripPointsPE::getOsnapPoints(const OdDbEntity* entity, OdDb::OsnapMode objectSnapMode, OdGsMarker selectionMarker, const OdGePoint3d& pickPoint, const OdGePoint3d& lastPoint, const OdGeMatrix3d& worldToEyeTransform, OdGePoint3dArray& snapPoints) const {
	if (objectSnapMode != OdDb::kOsModeEnd) { // FLYSDK version
		return eNotImplemented;
	}
	//PdfUnderlayModulePtr pModule = odrxDynamicLinker()->loadModule(OdPdfModuleVIModuleName);
	//
	PdfUnderlayModulePtr pModule = OdDbPdfDefinition::loadPdfUnderlayModule();
	if (pModule.isNull()) {
		return eTxError;
	}
	const auto Result {OdDbUnderlayGripPointsPE::getOsnapPoints(entity, objectSnapMode, selectionMarker, pickPoint, lastPoint, worldToEyeTransform, snapPoints)};
	if (eOk == Result) {
		class PdfSnapGrabberImpl : public OdStaticRxObject<OdGiDummyGeometry<OdGiGeometry> > {
			OdGePoint3dArray& snapPoints;
			OdGeMatrix3d mx;
		public:
			PdfSnapGrabberImpl(OdGePoint3dArray& sp, const OdGeMatrix3d& m)
				: snapPoints(sp)
				, mx(m) {}

			void polyline(OdInt32 numVertices, const OdGePoint3d* vertexList, const OdGeVector3d* /*pNormal*/, OdGsMarker /*baseSubEntMarker*/) override {
				for (auto f = 0; f < numVertices; ++f) {
					auto p {vertexList[f]};
					snapPoints.push_back(p.transformBy(mx));
				}
			}

			void nurbs(const OdGeNurbCurve3d& nurbsCurve) override {
				OdGePoint3d point;
				if (nurbsCurve.hasStartPoint(point)) {
					snapPoints.push_back(point.transformBy(mx));
				}
				if (nurbsCurve.hasEndPoint(point)) {
					snapPoints.push_back(point.transformBy(mx));
				}
			}

			void circularArc(const OdGePoint3d& start, const OdGePoint3d&, const OdGePoint3d& end, OdGiArcType) override {
				auto p {start};
				snapPoints.push_back(p.transformBy(mx));
				p = end;
				snapPoints.push_back(p.transformBy(mx));
			}

			void circle(const OdGePoint3d& center, double radius, const OdGeVector3d& normal) override {
				const OdGeCircArc3d CircularArc(center, normal, radius);
				OdGePoint3d point;
				if (CircularArc.hasStartPoint(point)) {
					snapPoints.push_back(point.transformBy(mx));
				}
			}
		};
		const auto UnderlayReference {OdDbUnderlayReference::cast(entity)};
		const OdGeMatrix3d transformRef;
		PdfSnapGrabberImpl PdfSnapGrabber(snapPoints, transformRef);
		//transformation is not needed because it has been applied to pickPoint (and will be applied to snap points) on app level
		return pModule->getSnapGeometry(UnderlayReference, &PdfSnapGrabber, &pickPoint);
	}
	return Result;
}
