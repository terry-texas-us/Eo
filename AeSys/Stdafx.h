#pragma once
#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif
#include "framework.h"
#include <strsafe.h>

// <tas=uncomment to use Guidelines Support Library https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md">
#pragma warning (push)
#pragma warning (disable: 4003)
//#include <gsl/gsl>
//#include <gsl/gsl_algorithm> // copy
#include <gsl/gsl_assert> // Ensures/Expects
//#include <gsl/gsl_byte> // byte
#include <gsl/gsl_util> // finally()/narrow()/narrow_cast()...
//#include <gsl/multi_span> // multi_span, strided_span...
#include <gsl/pointers> // owner, not_null
//#include <gsl/span> // span
#include <gsl/string_span> // zstring, string_span, zstring_builder...
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
#include <OdaCommon.h>
#include <Ge/GePoint3d.h>
#include <Ge/GeVector3d.h>
#include <Ge/GeMatrix3d.h>

unsigned AFXAPI HashKey(CString& string) noexcept;

#include "SafeMath.h"

inline double MillimetersToInches(const double millimeters) {
	return millimeters / kMmPerInch;
}

inline double InchesToMillimeters(const double inches) {
	return inches * kMmPerInch;
}

// <tas="Static analysis"/>
// Compiler warnings that are off by default (https://docs.microsoft.com/en-us/cpp/preprocessor/compiler-warnings-that-are-off-by-default?view=vs-2019)
#pragma warning (default: 4165) // (level 1) 'HRESULT' is being converted to 'bool'; are you sure this is what you want ?
#pragma warning (default: 4264) // (level 1) 'virtual_function': no override available for virtual member function from base 'class'; function is hidden
#pragma warning (default: 4342) // (level 1) behavior change : 'function' called, but a member operator was called in previous versions
#pragma warning (default: 4350) // (level 1) behavior change : 'member1' called instead of 'member2'
#pragma warning (default: 4426) // (level 1) optimization flags changed after including header, may be due to #pragma optimize()
#pragma warning (default: 4472) // (level 1) 'identifier' is a native enum : add an access specifier(private / public) to declare a managed enum
#pragma warning (default: 4545) // (level 1) expression before comma evaluates to a function which is missing an argument list
#pragma warning (default: 4545) // (level 1) expression before comma evaluates to a function which is missing an argument list
#pragma warning (default: 4546) // (level 1) function call before comma missing argument list
#pragma warning (default: 4547) // (level 1) 'operator': operator before comma has no effect; expected operator with side - effect
#pragma warning (default: 4548) // (level 1) expression before comma has no effect; expected expression with side - effect
#pragma warning (default: 4549) // (level 1) 'operator1': operator before comma has no effect; did you intend 'operator2' ?
#pragma warning (default: 4555) // (level 1) expression has no effect; expected expression with side - effect
#pragma warning (default: 4577) // (level 1) 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed.Specify / EHsc
#pragma warning (default: 4587) // (level 1) 'anonymous_structure': behavior change : constructor is no longer implicitly called
#pragma warning (default: 4588) // (level 1) 'anonymous_structure' : behavior change : destructor is no longer implicitly called
#pragma warning (default: 4598) // (level 1 and level 3) '#include "header"': header number number in the precompiled header does not match current compilation at that position
#pragma warning (default: 4605) // (level 1) '/Dmacro' specified on current command line, but was not specified when precompiled header was built
#pragma warning (default: 4628) // (level 1) digraphs not supported with - Ze.Character sequence 'digraph' not interpreted as alternate token for 'char'
#pragma warning (default: 4692) // (level 1) 'function': signature of non - private member contains assembly private native type 'native_type'
#pragma warning (default: 4822) // (level 1) 'member' : local class member function does not have a body
#pragma warning (default: 4905) // (level 1) wide string literal cast to 'LPSTR'
#pragma warning (default: 4906) // (level 1) string literal cast to 'LPWSTR'
#pragma warning (default: 4917) // (level 1) 'declarator' : a GUID can only be associated with a class, interface, or namespace
#pragma warning (default: 4928) // (level 1) illegal copy - initialization; more than one user - defined conversion has been implicitly applied
#pragma warning (default: 4946) // (level 1) reinterpret_cast used between related classes : 'class1'and 'class2'
#pragma warning (default: 5026) // (level 1 and level 4) 'type' : move constructor was implicitly defined as deleted
#pragma warning (default: 5027) // (level 1 and level 4) 'type' : move assignment operator was implicitly defined as deleted
#pragma warning (default: 5036) // (level 1) varargs function pointer conversion when compiling with / hybrid : x86arm64 'type1' to 'type2'
#pragma warning (default: 4412) // (level 2) 'function': function signature contains type 'type'; C++ objects are unsafe to pass between pure code and mixed or native
#pragma warning (default: 4826) // (level 2) Conversion from 'type1' to 'type2' is sign - extended.This may cause unexpected runtime behavior.
#pragma warning (default: 4191) // (level 3) 'operator': unsafe conversion from 'type_of_expression' to 'type_required'
#pragma warning (default: 4265) // (level 3) 'class' : class has virtual functions, but destructor is not virtual
#pragma warning (default: 4287) // (level 3) 'operator' : unsigned / negative constant mismatch
#pragma warning (default: 4370) // (level 3) layout of class has changed from a previous version of the compiler due to better packing
#pragma warning (default: 4371) // (level 3) 'classname' : layout of class may have changed from a previous version of the compiler due to better packing of member 'member'
#pragma warning (default: 4444) // (level 3) top level '__unaligned' is not implemented in this context
#pragma warning (default: 4557) // (level 3) '__assume' contains effect 'effect'
#pragma warning (default: 4599) // (level 3) 'option path' : command - line argument number number does not match pre - compiled header 14.3
#pragma warning (default: 4608) // (level 3) 'union_member' has already been initialized by another union member in the initializer list, 'union_member' Perm
#pragma warning (default: 4619) // (level 3) #pragma warning : there is no warning number 'number'
#pragma warning (default: 4640) // (level 3) 'instance' : construction of local static object is not thread - safe
#pragma warning (default: 4647) // (level 3) behavior change : __is_pod(type) has different value in previous versions
#pragma warning (default: 4686) // (level 3) 'user-defined type' : possible change in behavior, change in UDT return calling convention
#pragma warning (default: 4738) // (level 3) storing 32 - bit float result in memory, possible loss of performance
#pragma warning (default: 4768) // (level 3) __declspec attributes before linkage specification are ignored
#pragma warning (default: 4786) // (level 3) 'symbol' : object name was truncated to 'number' characters in the debug information
#pragma warning (default: 5042) // (level 3) 'function' : function declarations at block scope cannot be specified 'inline' in standard C++; remove 'inline' specifier
#pragma warning (default: 4061) // (level 4) enumerator 'identifier' in a switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning (default: 4062) // (level 4) enumerator 'identifier' in a switch of enum 'enumeration' is not handled
#pragma warning (default: 4242) // (level 4) 'identifier': conversion from 'type1' to 'type2', possible loss of data
#pragma warning (default: 4254) // (level 4) 'operator' : conversion from 'type1' to 'type2', possible loss of data
#pragma warning (default: 4255) // (level 4) 'function': no function prototype given: converting '()' to '(void)'
#pragma warning (default: 4263) // (level 4) 'function' : member function does not override any base class virtual member function
#pragma warning (default: 4266) // (level 4) 'function': no override available for virtual member function from base 'type'; function is hidden
#pragma warning (default: 4289) // (level 4) nonstandard extension used : 'var' : loop control variable declared in the for - loop is used outside the for - loop scope
#pragma warning (default: 4296) // (level 4) 'operator' : expression is always false
#pragma warning (default: 4339) // (level 4) 'type' : use of undefined type detected in CLR meta - data - use of this type may lead to a runtime exception
#pragma warning (default: 4365) // (level 4) 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#pragma warning (default: 4388) // (level 4) signed / unsigned mismatch
#pragma warning (default: 4435) // (level 4) 'class1' : Object layout under / vd2 will change due to virtual base 'class2'
#pragma warning (default: 4437) // (level 4) dynamic_cast from virtual base 'class1' to 'class2' could fail in some contexts
#pragma warning (default: 4464) // (level 4) relative include path contains '..'
#pragma warning (default: 4471) // (level 4) a forward declaration of an unscoped enumeration must have an underlying type(int assumed) Perm
#pragma warning (default: 4514) // (level 4) 'function' : unreferenced inline function has been removed
#pragma warning (default: 4536) // (level 4) 'type name' : type - name exceeds meta - data limit of 'limit' characters
#pragma warning (default: 4571) // (level 4) informational : catch (...) semantics changed since Visual C++ 7.1; structured exceptions(SEH) are no longer caught
#pragma warning (default: 4574) // (level 4) 'identifier' is defined to be '0': did you mean to use '#if identifier' ?
#pragma warning (default: 4582) // (level 4) 'type' : constructor is not implicitly called
#pragma warning (default: 4583) // (level 4) 'type' : destructor is not implicitly called
#pragma warning (default: 4596) // (level 4) 'identifier' : illegal qualified name in member declaration 14.3 Perm
#pragma warning (default: 4623) // (level 4) 'derived class' : default constructor could not be generated because a base class default constructor is inaccessible
#pragma warning (default: 4625) // (level 4) 'derived class' : copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning (default: 4626) // (level 4) 'derived class' : assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning (default: 4643) // (level 4) Forward declaring 'identifier' in namespace std is not permitted by the C++ Standard.
#pragma warning (default: 4654) // (level 4) Code placed before include of precompiled header line will be ignored.Add code to precompiled header.
#pragma warning (default: 4668) // (level 4) 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#pragma warning (default: 4682) // (level 4) 'symbol' : no directional parameter attribute specified, defaulting to[in]
#pragma warning (default: 4710) // (level 4) 'function' : function not inlined
#pragma warning (default: 4749) // (level 4) conditionally supported : offsetof applied to non - standard - layout type 'type'
#pragma warning (default: 4767) // (level 4) section name 'symbol' is longer than 8 characters and will be truncated by the linker
#pragma warning (default: 4774) // (level 4) 'string' : format string expected in argument number is not a string literal
#pragma warning (default: 4777) // (level 4) 'function' : format string 'string' requires an argument of type 'type1', but variadic argument number has type 'type2'
// <tas="requires VS 2019"/>#pragma warning (default: 4800) // (level 4) Implicit conversion from 'type' to bool.Possible information loss
#pragma warning (default: 4820) // (level 4) 'bytes' bytes padding added after construct 'member_name'
#pragma warning (default: 4837) // (level 4) trigraph detected : '??character' replaced by 'character'
#pragma warning (default: 4841) // (level 4) non - standard extension used : compound member designator used in offsetof
#pragma warning (default: 4842) // (level 4) the result of 'offsetof' applied to a type using multiple inheritance is not guaranteed to be consistent between compiler releases
#pragma warning (default: 4868) // (level 4) 'file(line_number)' compiler may not enforce left - to - right evaluation order in braced initialization list
#pragma warning (default: 4931) // (level 4) we are assuming the type library was built for number - bit pointers
#pragma warning (default: 4986) // (level 4) 'symbol' : exception specification does not match previous declaration
#pragma warning (default: 4987) // (level 4) nonstandard extension used : 'throw (...)'
#pragma warning (default: 4988) // (level 4) 'symbol' : variable declared outside class / function scope
#pragma warning (default: 5024) // (level 4) 'type' : move constructor was implicitly defined as deleted
#pragma warning (default: 5025) // (level 4) 'type' : move assignment operator was implicitly defined as deleted
#pragma warning (default: 5029) // (level 4) nonstandard extension used : alignment attributes in C++ apply to variables, data members and tag types only
#pragma warning (default: 5031) // (level 4) #pragma warning(pop) : likely mismatch, popping warning state pushed in different file
#pragma warning (default: 5032) // (level 4) detected #pragma warning(push) with no corresponding #pragma warning(pop)
#pragma warning (default: 5039) // (level 4) 'function' : pointer or reference to potentially throwing function passed to extern C function under - EHc.Undefined behavior may occur if this function throws an exception.
#pragma warning (default: 5038) // (level 4) data member 'member1' will be initialized after data member 'member2'
#pragma warning (default: 4355) // 'this' : used in base member initializer list
#pragma warning (default: 4746) // volatile access of 'expression' is subject to / volatile : <iso | ms> setting; consider using __iso_volatile_load / store intrinsic functions
#pragma warning (default: 4962) // 'function': profile - guided optimizations disabled because optimizations caused profile data to become inconsistent
#pragma warning (default: 5022) // 'type' : multiple move constructors specified
#pragma warning (default: 5023) // 'type' : multiple move assignment operators specified
#pragma warning (default: 5034) // use of intrinsic 'intrinsic' causes function function to be compiled as guest code
#pragma warning (default: 5035) // use of feature 'feature' causes function function to be compiled as guest code

// <tas="Following default warnings count many thousands. For later evaluation separately"/>
#pragma warning(disable : 5026) // (level 1 and level 4) 'type': move constructor was implicitly defined as deleted
#pragma warning (disable: 5027) // (level 1 and level 4) 'type' : move assignment operator was implicitly defined as deleted
#pragma warning (disable: 5031) // #pragma warning(pop) : likely mismatch, popping warning state pushed in different file

// <tas="Following warnings occur many times in Oda libraries"/>
#pragma warning(disable : 4619) // (level 3) there is no warning number 'number'

// <tas="Following off by default warnings (defaulted above) occur many times in Oda libraries. For later evaluation separately"/>
#pragma warning(disable : 4514) // (level 4) 'function' : unreferenced inline function has been removed
#pragma warning (disable: 4625) // (level 4) 'derived class' : copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning (disable: 4626) // (level 4) 'derived class' : assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning (disable: 4800) // (level 4) Implicit conversion from 'type' to bool.Possible information loss
#pragma warning (disable: 4820) // (level 4) 'bytes' bytes padding added after construct 'member_name'
