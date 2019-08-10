#pragma once
#include "RxModule.h"
#include "StaticRxObject.h"
#include "Db2LineAngularDimGripPoints.h"
#include "Db3PointAngularDimGripPoints.h"
#include "DbAlignedDimGripPoints.h"
#include "DbArcGripPoints.h"
#include "DbArcDimGripPoints.h"
#include "DbDiametricDimGripPoints.h"
#include "DbLineGripPoints.h"
#include "DbPolylineGripPoints.h"
#include "DbEntityGripPoints.h"
#include "DbMlineGripPoints.h"
#include "DbBlockReferenceGripPoints.h"
#include "DbMleaderGripPoints.h"
#include "DbOrdinateDimGripPoints.h"
#include "DbPolygonMeshGripPoints.h"
#include "DbPdfUnderlayGripPoints.h"
#include "DbRadialDimGripPoints.h"
#include "DbRadialDimLargeGripPoints.h"
#include "DbRotatedDimGripPoints.h"
#include "DbViewportGripPoints.h"
#include "Db2dPolylineGripPoints.h"
#include "DbRasterImageGripPoints.h"
#include "DbSolidGripPoints.h"
#include "DbTraceGripPoints.h"
#include "Db3dPolylineGripPoints.h"
#include "DbCameraGripPoints.h"
#include "DbCircleGripPoints.h"
#include "DbEllipseGripPoints.h"
#include "DbTextGripPoints.h"
#include "DbGeoPositionMarkerGripPoints.h"
#include "DbDgnUnderlayGripPoints.h"
#include "DbOleGripPoints.h"
#include "DbWipeOutGripPoints.h"
#include "DbFaceGripPoints.h"

/**
 * \brief Declaration of the OdGeGripPointsPE interface. Drawings SDK attempts to use this interface for grip points operations; OdDbEntity::getGripPoints, etc.
 */
class OdGripPointsModule : public OdRxModule {
	OdStaticRxObject<OdDbLineGripPointsPE> m_LineGripPoints;
	OdStaticRxObject<OdDbMlineGripPointsPE> m_MlineGripPoints;
	OdStaticRxObject<OdDbMleaderGripPointsPE> m_MleaderGripPoints;
	OdStaticRxObject<OdDbPolygonMeshGripPointsPE> m_PolygonMeshGripPoints;
	OdStaticRxObject<OdDbArcGripPointsPE> m_ArcGripPoints;
	OdStaticRxObject<OdDbPolylineGripPointsPE> m_PolylineGripPoints;
	OdStaticRxObject<OdDbEntityGripPointsPE> m_EntityGripPoints;
	OdStaticRxObject<OdDbRotatedDimGripPointsPE> m_RotatedDimGripPoints;
	OdStaticRxObject<OdDbAlignedDimGripPointsPE> m_AlignedDimGripPoints;
	OdStaticRxObject<OdDbRadialDimGripPointsPE> m_RadialDimGripPoints;
	OdStaticRxObject<OdDbDiametricDimGripPointsPE> m_DiametricDimGripPoints;
	OdStaticRxObject<OdDb3PointAngularDimGripPointsPE> m_3PointAngularDimGripPoints;
	OdStaticRxObject<OdDbOrdinateDimGripPointsPE> m_OrdinateDimGripPoints;
	OdStaticRxObject<OdDb2LineAngularDimGripPointsPE> m_2LineAngularDimGripPoints;
	OdStaticRxObject<OdDbArcDimGripPointsPE> m_ArcDimGripPoints;
	OdStaticRxObject<OdDbRadialDimLargeGripPointsPE> m_RadialDimLargeGripPoints;
	OdStaticRxObject<OdDbBlockReferenceGripPointsPE> m_BlockReferenceGripPoints;
	OdStaticRxObject<OdDbPdfUnderlayGripPointsPE> m_PdfUnderlayGripPoints;
	OdStaticRxObject<OdDbViewportGripPointsPE> m_ViewportGripPoints;
	OdStaticRxObject<OdDb2dPolylineGripPointsPE> m_2dPolylineGripPoints;
	OdStaticRxObject<OdDbRasterImageGripPointsPE> m_RasterImageGripPoints;
	OdStaticRxObject<OdDbTraceGripPointsPE> m_TraceGripPoints;
	OdStaticRxObject<OdDbSolidGripPointsPE> m_SolidGripPoints;
	OdStaticRxObject<OdDb3dPolylineGripPointsPE> m_3dPolylineGripPoints;
	OdStaticRxObject<OdDbCameraGripPointsPE> m_CameraGripPoints;
	OdStaticRxObject<OdDbCircleGripPointsPE> m_CircleGripPoints;
	OdStaticRxObject<OdDbEllipseGripPointsPE> m_EllipseGripPoints;
	OdStaticRxObject<OdDbTextGripPointsPE> m_TextGripPoints;
	OdStaticRxObject<OdDbGeoPositionMarkerPE> m_GeoPositionMarkerGripPoints;
	OdStaticRxObject<OdDbDgnUnderlayGripPointsPE> m_DgnUnderlayGripPoints;
	OdStaticRxObject<OdDbOleGripPointsPE> m_OleGripPoints;
	OdStaticRxObject<OdDbWipeOutGripPointsPE> m_WipeOutGripPoints;
	OdStaticRxObject<OdDbFaceGripPointsPE> m_FaceGripPoints;
protected:
	OdGripPointsModule();

	void initApp() override;

	void uninitApp() override;

public:
	~OdGripPointsModule();
};

/**
 * \brief  For 2D object projects given offset vector on object's plane defined by normal in current view direction.
 * \param database
 * \param normal 
 * \param offset 
 * \return true on success. If current view direction is perpendicular to normal returns false and does not modify offset
 */
bool ProjectOffset(const OdDbDatabase* database, const OdGeVector3d& normal, OdGeVector3d& offset);
