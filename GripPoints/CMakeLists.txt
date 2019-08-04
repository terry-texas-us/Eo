#
#  GripPoints library
#

tdrawing_sources(${TD_GRIPPOINTS_LIB}
    StdAfx.cpp
    DbLineGripPoints.cpp
    DbTextGripPoints.cpp
    DbCircleGripPoints.cpp
    DbDimGripPoints.cpp
    DbArcGripPoints.cpp
    DbPolylineGripPoints.cpp
    Db2dPolylineGripPoints.cpp
    Db3dPolylineGripPoints.cpp
    DbMlineGripPoints.cpp
    OdGripPointsModule.cpp
    DbEntityGripPoints.cpp
    DbBlockReferenceGripPoints.cpp
    DbMleaderGripPoints.cpp
    DbUnderlayGripPoints.cpp
    DbPdfUnderlayGripPoints.cpp
    DbDgnUnderlayGripPoints.cpp
    DbPolygonMeshGripPoints.cpp
    DbRasterImageGripPoints.cpp
    DbViewportGripPoints.cpp
    DbTraceGripPoints.cpp
    DbCameraGripPoints.cpp
    DbGeoPositionMarkerGripPoints.cpp
    DbOleGripPoints.cpp
    DbWipeOutGripPoints.cpp
    DbEllipseGripPoints.cpp
    DbFaceGripPoints.cpp

    StdAfx.h
    OdGripPointsModule.h
    DbArcGripPoints.h
    DbMlineGripPoints.h
    DbLineGripPoints.h
    DbTextGripPoints.h
    DbCircleGripPoints.h
    DbDimGripPoints.h
    DbBlockReferenceGripPoints.h
    DbEntityGripPoints.h
    DbPolylineGripPoints.h
    Db2dPolylineGripPoints.h
    Db3dPolylineGripPoints.h
    DbPolygonMeshGripPoints.h
    DbRasterImageGripPoints.h
    DbUnderlayGripPoints.h
    DbPdfUnderlayGripPoints.h
    DbDgnUnderlayGripPoints.h
    DbViewportGripPoints.h
    DbTraceGripPoints.h
    DbCameraGripPoints.h
    DbMleaderGripPoints.h
    DbGeoPositionMarkerGripPoints.h
    DbOleGripPoints.h
    DbWipeOutGripPoints.h
    DbEllipseGripPoints.h
    DbFaceGripPoints.h
  )

include_directories()

if(ODA_SHARED AND MSVC)
tdrawing_sources(${TD_GRIPPOINTS_LIB}
        GripPoints.rc
        )
endif(ODA_SHARED AND MSVC)

add_definitions(-DGRIPPOINTS_DLL_EXPORTS)

if(NOT WINCE)
add_definitions(-DODA_LINT)
endif(NOT WINCE)

tdrawing_tx(${TD_GRIPPOINTS_LIB} ${TD_DB_LIB} ${TD_DBROOT_LIB} ${TD_GE_LIB} ${TD_ROOT_LIB} ${TD_ALLOC_LIB} )

tdrawing_project_group(${TD_GRIPPOINTS_LIB} "Examples")