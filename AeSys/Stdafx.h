#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#include "framework.h"

// <tas=uncomment to use Guidelines Support Library https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md">
#pragma warning (push)
#pragma warning (disable: 4003)
#include <gsl/gsl>
#pragma warning (pop)
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

#include <Ge/GePoint3d.h>
#include <Ge/GeVector3d.h>
#include <Ge/GeMatrix3d.h>

unsigned AFXAPI HashKey(CString& string) noexcept;

constexpr auto RADIAN = OdaPI / 180.0;

#include "SafeMath.h"
