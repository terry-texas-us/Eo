#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h> // MFC core and standard components
#include <afxext.h> // MFC extensions

#include <afxdisp.h> // MFC Automation classes

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h> // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h> // MFC support for ribbons and control bars

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <cfloat>
#include <math.h>
#include <memory>

// OD_OLE_SUPPORT
// Vectorization support for OLE objects on Windows can be obtained by including this module: OdOleItemHandler
// Source for this module is located in [kernel root]/Extensions/win/OleItemHandler.
// OLE support can be enabled by linking in the OdOleItemHandler module and registering "OdOleItemHandler" using the ODRX_DEFINE_STATIC_APPLICATION macro.
// For the DLL version, place the OdOleItemHandler.tx module in the same directory as the DLLs (no explicit registration required).
// Uncomment #define for support 
// #define OD_OLE_SUPPORT 1

#include "Resource.h"

#include "OdaCommon.h"
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
#include "DbSortentsTable.h"
#include "DbSpatialFilter.h"
#include "StringArray.h"
#include "DbSymUtl.h"
#include "DbViewport.h"
#include "DbViewportTable.h"
#include "DbViewportTableRecord.h"
#include "LyLayerFilter.h"

typedef unsigned char EoByte;
typedef char EoSbyte;
typedef short EoInt16;
typedef unsigned short EoUInt16;

UINT AFXAPI HashKey(CString& str);

const double PI = 3.14159265358979323846;
const double HALF_PI = PI / 2.;
const double QUARTER_PI = PI / 4.;
const double RADIAN = PI / 180.;
const double TWOPI = PI + PI;

const double EoMmPerInch = 25.4;

#include "SafeMath.h"

#include "EoVaxFloat.h"

#include "EoGePoint3d.h"
#include "EoGePoint4d.h"
#include "EoGeMatrix3d.h"
#include "EoGeReferenceSystem.h"
#include "EoGeLineSeg3d.h"
#include "EoGeNurbCurve3d.h"
#include "EoGePolyline.h"
#include "EoGeUniquePoint.h"

#include "EoGsViewport.h"
#include "EoGsModelTransform.h"
#include "EoGsViewTransform.h"

#include "EoDb.h"
#include "EoDbCharacterCellDefinition.h"
#include "EoDbFontDefinition.h"

#include "EoDbPrimitive.h"
#include "EoDbBlockReference.h"
#include "EoDbDimension.h"
#include "EoDbEllipse.h"
#include "EoDbLine.h"
#include "EoDbPoint.h"
#include "EoDbHatch.h"
#include "EoDbPolyline.h"
#include "EoDbSpline.h"
#include "EoDbText.h"
#include "EoDbMaskedPrimitive.h"
#include "EoDbGroup.h"
#include "EoDbBlock.h"
#include "EoDbGroupList.h"
#include "EoDbLayer.h"
#include "EoDbFile.h"
#include "EoDbTracingFile.h"
#include "EoDbPegFile.h"

#include "PrimState.h"
#include "Section.h"
