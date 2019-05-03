#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#include "framework.h"

#include <cfloat>
#include <math.h>
#include <memory>

// <tas=uncomment to use Guidelines Support Library https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md">
#include <gsl/gsl>
using namespace gsl;
// </tas>

#include "Resource.h"

// OD_OLE_SUPPORT
// Vectorization support for OLE objects on Windows can be obtained by including this module: OdOleItemHandler
// Source for this module is located in [kernel root]/Extensions/win/OleItemHandler.
// OLE support can be enabled by linking in the OdOleItemHandler module and registering "OdOleItemHandler" using the ODRX_DEFINE_STATIC_APPLICATION macro.
// For the DLL version, place the OdOleItemHandler.tx module in the same directory as the DLLs (no explicit registration required).
// Uncomment #define for support 
// #define OD_OLE_SUPPORT 1

#include "OdaCommon.h"
#include "DbExport.h"
#include "DbObject.h"

#include "Ge/GeTol.h"
#include "Ge/GePoint3d.h"
#include "Ge/GeScale3d.h"
#include "Ge/GeLineSeg3d.h"
#include "Ge/GeKnotVector.h"
#include "Ge/GeNurbCurve3d.h"
#include "Ge/GePlane.h"	
#include "Ge/GeRay3d.h"
#include "DbBlockIterator.h"
#include "DbBlockReference.h"
#include "DbBlockTable.h"
#include "DbBlockTableRecord.h"
#include "DbDatabase.h"
#include "DbLayerTable.h"
#include "DbLayerTableRecord.h"
#include "DbLinetypeTable.h"
#include "DbLinetypeTableRecord.h"
#include "DbTextStyleTable.h"
#include "DbTextStyleTableRecord.h"
#include "DbPoint.h"
#include "DbLine.h"
#include "DbMText.h"
#include "DbPolyline.h"
#include "DbText.h"
#include "DbSpline.h"
#include "DbSortentsTable.h"
#include "DbSpatialFilter.h"
#include "StringArray.h"
#include "DbSymUtl.h"
#include "DbViewport.h"
#include "DbViewportTable.h"
#include "DbViewportTableRecord.h"
#include "LyLayerFilter.h"

UINT AFXAPI HashKey(CString& str) noexcept;

const double PI = 3.14159265358979323846;
const double HALF_PI = PI / 2.;
const double QUARTER_PI = PI / 4.;
const double RADIAN = PI / 180.;
const double TWOPI = PI + PI;

const double EoMmPerInch = 25.4;

#include "SafeMath.h"
