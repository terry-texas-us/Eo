#pragma once
#include "RxModule.h"
#include "StaticRxObject.h"
#include "DbArcGripPoints.h"
#include "DbLineGripPoints.h"
#include "DbPolylineGripPoints.h"
#include "DbEntityGripPoints.h"
#include "DbDimGripPoints.h"
#include "DbMlineGripPoints.h"
#include "DbBlockReferenceGripPoints.h"
#include "DbMleaderGripPoints.h"
#include "DbPolygonMeshGripPoints.h"
#include "DbPdfUnderlayGripPoints.h"
#include "DbViewportGripPoints.h"
#include "Db2dPolylineGripPoints.h"
#include "DbRasterImageGripPoints.h"
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
	OdStaticRxObject<OdDbLineGripPointsPE> _lgp;
	OdStaticRxObject<OdDbMlineGripPointsPE> _mlgp;
	OdStaticRxObject<OdDbMleaderGripPointsPE> _mleadgp;
	OdStaticRxObject<OdDbPolygonMeshGripPointsPE> _mmeshdgp;
	OdStaticRxObject<OdDbArcGripPointsPE> _agp;
	OdStaticRxObject<OdDbPolylineGripPointsPE> _plgp;
	OdStaticRxObject<OdDbEntityGripPointsPE> _egp;
	OdStaticRxObject<OdDbRotatedDimGripPointsPE> _rdgp;
	OdStaticRxObject<OdDbAlignedDimGripPointsPE> _adgp;
	OdStaticRxObject<OdDbRadialDimGripPointsPE> _rcdgp;
	OdStaticRxObject<OdDbDiametricDimGripPointsPE> _dcdgp;
	OdStaticRxObject<OdDb3PointAngularDimGripPointsPE> _3padgp;
	OdStaticRxObject<OdDbOrdinateDimGripPointsPE> _odgp;
	OdStaticRxObject<OdDb2LineAngularDimGripPointsPE> _2ladgp;
	OdStaticRxObject<OdDbArcDimGripPointsPE> _arcdgp;
	OdStaticRxObject<OdDbRadialDimLargeGripPointsPE> _rdlgp;
	OdStaticRxObject<OdDbBlockReferenceGripPointsPE> _insertgp;
	OdStaticRxObject<OdDbPdfUnderlayGripPointsPE> _pdfunderlaygp;
	OdStaticRxObject<OdDbViewportGripPointsPE> _vptgp;
	OdStaticRxObject<OdDb2dPolylineGripPointsPE> _2dplgp;
	OdStaticRxObject<OdDbRasterImageGripPointsPE> _rimggp;
	OdStaticRxObject<OdDbTraceGripPointsPE> _trcgp;
	OdStaticRxObject<OdDbSolidGripPointsPE> _sldgp;
	OdStaticRxObject<OdDb3dPolylineGripPointsPE> _3dplgp;
	OdStaticRxObject<OdDbCameraGripPointsPE> _camgp;
	OdStaticRxObject<OdDbCircleGripPointsPE> _cgp;
	OdStaticRxObject<OdDbEllipseGripPointsPE> _elgp;
	OdStaticRxObject<OdDbTextGripPointsPE> _txtgp;
	OdStaticRxObject<OdDbGeoPositionMarkerPE> _geoposmarkgp;
	OdStaticRxObject<OdDbDgnUnderlayGripPointsPE> _dgnunderlaygp;
	OdStaticRxObject<OdDbOleGripPointsPE> _olegp;
	OdStaticRxObject<OdDbWipeOutGripPointsPE> _wipeoutgp;
	OdStaticRxObject<OdDbFaceGripPointsPE> _fgp;
protected:
	OdGripPointsModule();

	void initApp() override;

	void uninitApp();

public:
	~OdGripPointsModule();
};

/**
 * \brief  For 2D object projects given offset vector on object's plane defined by normal in current view direction.
 * \param database
 * \param normal 
 * \param offset 
 * \return Returns true on success. If current view direction is perpendicular to vNormal returns false and does not modify vOffset
 */
bool projectOffset(const OdDbDatabase* database, const OdGeVector3d& normal, OdGeVector3d& offset);
