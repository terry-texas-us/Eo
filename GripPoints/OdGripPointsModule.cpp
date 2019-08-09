#include <OdaCommon.h>
#include "OdGripPointsModule.h"
#include <RxDynamicModule.h>
#include <DbLine.h>
#include <DbMline.h>
#include <DbArc.h>
#include <DbPolyline.h>
#include <DbRotatedDimension.h>
#include <DbAlignedDimension.h>
#include <DbRadialDimension.h>
#include <DbDiametricDimension.h>
#include <Db3PointAngularDimension.h>
#include <DbOrdinateDimension.h>
#include <Db2LineAngularDimension.h>
#include <DbRadialDimensionLarge.h>
#include <DbArcDimension.h>
#include <DbCircle.h>
#include <DbBlockReference.h>
#include <DbMLeader.h>
#include <DbOle2Frame.h>
#include <DbPolygonMesh.h>
#include <DbUnderlayReference.h>
#include <DbViewport.h>
#include <DbRasterImage.h>
#include <DbTrace.h>
#include <DbSolid.h>
#include <Db3dPolyline.h>
#include <DbCamera.h>
#include <AbstractViewPE.h>
#include <DbGeoPositionMarker.h>
#include <DbWipeout.h>
#include <DbFace.h>
#if defined(_TOOLKIT_IN_DLL_) && defined(_MSC_VER)
#define VC_EXTRALEAN
#include "windows.h"

extern "C" int APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
	switch (reason) {
		case DLL_PROCESS_ATTACH: // remove this if you need per-thread initialization
			DisableThreadLibraryCalls(static_cast<HMODULE>(instance));
			break;
	}
	return TRUE;
}
#endif
#pragma warning (suppress: 5043)
ODRX_DEFINE_DYNAMIC_MODULE(OdGripPointsModule)

OdGripPointsModule::OdGripPointsModule() {}

OdGripPointsModule::~OdGripPointsModule() {}

void OdGripPointsModule::initApp() {
	if (OdDbEntity::desc() == nullptr) {
		throw OdError(eNotInitializedYet);
	}
	odrxDynamicLinker()->loadModule(OdDbModuleName, false)->addRef();
	OdDbEntity::desc()->addX(OdDbGripPointsPE::desc(), &m_EntityGripPoints);
	OdDbLine::desc()->addX(OdDbGripPointsPE::desc(), &m_LineGripPoints);
	OdDbMline::desc()->addX(OdDbGripPointsPE::desc(), &m_MlineGripPoints);
	OdDbArc::desc()->addX(OdDbGripPointsPE::desc(), &m_ArcGripPoints);
	OdDbPolyline::desc()->addX(OdDbGripPointsPE::desc(), &m_PolylineGripPoints);
	OdDbRotatedDimension::desc()->addX(OdDbGripPointsPE::desc(), &m_RotatedDimGripPoints);
	OdDbAlignedDimension::desc()->addX(OdDbGripPointsPE::desc(), &m_AlignedDimGripPoints);
	OdDbRadialDimension::desc()->addX(OdDbGripPointsPE::desc(), &m_RadialDimGripPoints);
	OdDbDiametricDimension::desc()->addX(OdDbGripPointsPE::desc(), &m_DiametricDimGripPoints);
	OdDb3PointAngularDimension::desc()->addX(OdDbGripPointsPE::desc(), &m_3PointAngularDimGripPoints);
	OdDbOrdinateDimension::desc()->addX(OdDbGripPointsPE::desc(), &m_OrdinateDimGripPoints);
	OdDb2LineAngularDimension::desc()->addX(OdDbGripPointsPE::desc(), &m_2LineAngularDimGripPoints);
	OdDbArcDimension::desc()->addX(OdDbGripPointsPE::desc(), &m_ArcDimGripPoints);
	OdDbRadialDimensionLarge::desc()->addX(OdDbGripPointsPE::desc(), &m_RadialDimLargeGripPoints);
	OdDbBlockReference::desc()->addX(OdDbGripPointsPE::desc(), &m_BlockReferenceGripPoints);
	OdDbMLeader::desc()->addX(OdDbGripPointsPE::desc(), &m_MleaderGripPoints);
	OdDbPolygonMesh::desc()->addX(OdDbPolygonMeshGripPointsPE::desc(), &m_PolygonMeshGripPoints);
	OdDbPdfReference::desc()->addX(OdDbPdfUnderlayGripPointsPE::desc(), &m_PdfUnderlayGripPoints);
	OdDbViewport::desc()->addX(OdDbViewportGripPointsPE::desc(), &m_ViewportGripPoints);
	OdDb2dPolyline::desc()->addX(OdDbGripPointsPE::desc(), &m_2dPolylineGripPoints);
	OdDbRasterImage::desc()->addX(OdDbRasterImageGripPointsPE::desc(), &m_RasterImageGripPoints);
	OdDbTrace::desc()->addX(OdDbTraceGripPointsPE::desc(), &m_TraceGripPoints);
	OdDbSolid::desc()->addX(OdDbSolidGripPointsPE::desc(), &m_SolidGripPoints);
	OdDb3dPolyline::desc()->addX(OdDbGripPointsPE::desc(), &m_3dPolylineGripPoints);
	OdDbCamera::desc()->addX(OdDbGripPointsPE::desc(), &m_CameraGripPoints);
	OdDbCircle::desc()->addX(OdDbGripPointsPE::desc(), &m_CircleGripPoints);
	OdDbEllipse::desc()->addX(OdDbGripPointsPE::desc(), &m_EllipseGripPoints);
	OdDbText::desc()->addX(OdDbGripPointsPE::desc(), &m_TextGripPoints);
	OdDbGeoPositionMarker::desc()->addX(OdDbGeoPositionMarkerPE::desc(), &m_GeoPositionMarkerGripPoints);
	OdDbUnderlayReference::desc()->addX(OdDbDgnUnderlayGripPointsPE::desc(), &m_DgnUnderlayGripPoints);
	OdDbOle2Frame::desc()->addX(OdDbOleGripPointsPE::desc(), &m_OleGripPoints);
	OdDbWipeout::desc()->addX(OdDbWipeOutGripPointsPE::desc(), &m_WipeOutGripPoints);
	OdDbFace::desc()->addX(OdDbGripPointsPE::desc(), &m_FaceGripPoints);
	OdDbBlockGripAppData::rxInit();
}

void OdGripPointsModule::uninitApp() {
	OdDbBlockGripAppData::rxUninit();
	OdDbFace::desc()->delX(OdDbFaceGripPointsPE::desc());
	OdDbWipeout::desc()->delX(OdDbWipeOutGripPointsPE::desc());
	OdDbOle2Frame::desc()->delX(OdDbOleGripPointsPE::desc());
	OdDbGeoPositionMarker::desc()->delX(OdDbGeoPositionMarkerPE::desc());
	OdDbGeoPositionMarker::desc()->delX(OdDbGeoPositionMarkerPE::desc());
	OdDbText::desc()->delX(OdDbGripPointsPE::desc());
	OdDbEllipse::desc()->delX(OdDbGripPointsPE::desc());
	OdDbCircle::desc()->delX(OdDbGripPointsPE::desc());
	OdDbCamera::desc()->delX(OdDbGripPointsPE::desc());
	OdDb3dPolyline::desc()->delX(OdDbGripPointsPE::desc());
	OdDbSolid::desc()->delX(OdDbSolidGripPointsPE::desc());
	OdDbTrace::desc()->delX(OdDbTraceGripPointsPE::desc());
	OdDbViewport::desc()->delX(OdDbViewportGripPointsPE::desc());
	OdDbPdfReference::desc()->delX(OdDbPdfUnderlayGripPointsPE::desc());
	OdDbPolyline::desc()->delX(OdDbGripPointsPE::desc());
	OdDbLine::desc()->delX(OdDbGripPointsPE::desc());
	OdDbMline::desc()->delX(OdDbGripPointsPE::desc());
	OdDbArc::desc()->delX(OdDbGripPointsPE::desc());
	OdDbEntity::desc()->delX(OdDbGripPointsPE::desc());
	OdDbRotatedDimension::desc()->delX(OdDbGripPointsPE::desc());
	OdDbAlignedDimension::desc()->delX(OdDbGripPointsPE::desc());
	OdDbRadialDimension::desc()->delX(OdDbGripPointsPE::desc());
	OdDbDiametricDimension::desc()->delX(OdDbGripPointsPE::desc());
	OdDb3PointAngularDimension::desc()->delX(OdDbGripPointsPE::desc());
	OdDbOrdinateDimension::desc()->delX(OdDbGripPointsPE::desc());
	OdDb2LineAngularDimension::desc()->delX(OdDbGripPointsPE::desc());
	OdDbArcDimension::desc()->delX(OdDbGripPointsPE::desc());
	OdDbRadialDimensionLarge::desc()->delX(OdDbGripPointsPE::desc());
	OdDbBlockReference::desc()->delX(OdDbGripPointsPE::desc());
	OdDbMLeader::desc()->delX(OdDbGripPointsPE::desc());
	OdDbPolygonMesh::desc()->delX(OdDbPolygonMeshGripPointsPE::desc());
	OdDb2dPolyline::desc()->delX(OdDbGripPointsPE::desc());
	OdDbRasterImage::desc()->delX(OdDbRasterImageGripPointsPE::desc());
	OdDbUnderlayReference::desc()->delX(OdDbDgnUnderlayGripPointsPE::desc());
	odrxDynamicLinker()->loadModule(OdDbModuleName, false)->release();
}

bool ProjectOffset(const OdDbDatabase* database, const OdGeVector3d& normal, OdGeVector3d& offset) {
	const auto ActiveViewportId {database->activeViewportId()};
	auto ActiveViewport(ActiveViewportId.openObject());
	OdAbstractViewPEPtr AbstractView(ActiveViewport);
	if (!AbstractView.isNull()) {
		const auto ViewDirection {AbstractView->direction(ActiveViewport)};
		if (!ViewDirection.isPerpendicularTo(normal)) {
			const OdGePlane Plane(OdGePoint3d::kOrigin, normal);
			OdGeMatrix3d ProjectionTransform;
			ProjectionTransform.setToProjection(Plane, ViewDirection);
			offset.transformBy(ProjectionTransform);
			return true;
		}
	}
	return false;
}
