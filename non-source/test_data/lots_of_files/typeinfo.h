/***
*typeinfo.h - Defines the type_info structure and exceptions used for RTTI
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines the type_info structure and exceptions used for
*       Runtime Type Identification.
*
*       [Public]
*
****/

#pragma once

#include <crtdefs.h>

#ifndef _INC_TYPEINFO
#define _INC_TYPEINFO

#pragma pack(push,_CRT_PACKING)

#ifndef RC_INVOKED

#ifndef __cplusplus
#error This header requires a C++ compiler ...
#endif  /* __cplusplus */

#ifndef _SYSCRT
#include <typeinfo>

#if !defined(_CRTBLD) || !defined(_TICORE)

#ifdef __RTTI_OLDNAMES
/* Some synonyms for folks using older standard */
using std::bad_cast;
using std::bad_typeid;

typedef type_info Type_info;
typedef bad_cast Bad_cast;
typedef bad_typeid Bad_typeid;
#endif  /* __RTTI_OLDNAMES */

#endif  /* !defined(_CRTBLD) || !defined(_TICORE) */

#else  /* _SYSCRT */

class type_info {
public:
    SECURITYCRITICAL_ATTRIBUTE
    _CRTIMP virtual __thiscall ~type_info();
    _CRTIMP int __thiscall operator==(_In_ const type_info& _Rhs) const;
    _CRTIMP int __thiscall operator!=(_In_ const type_info& _Rhs) const;
    _CRTIMP bool __thiscall before(_In_ const type_info& _Rhs) const;
#ifdef _SYSCRT
    _Check_return_ _CRTIMP const char* __thiscall name() const;
#else  /* _SYSCRT */
    _Check_return_ _CRTIMP const char* __thiscall name(_Inout_ __type_info_node* __ptype_info_node = &__type_info_root_node) const;
#endif  /* _SYSCRT */
    _Check_return_ _CRTIMP const char* __thiscall raw_name() const;
private:
    void *_M_data;
    char _M_d_name[1];
    __thiscall type_info(_In_ const type_info& _Rhs);
    type_info& __thiscall operator=(_In_ const type_info& _Rhs);
};
#ifndef _TICORE

/* This include must occur below the definition of class type_info */
#include <stdexcpt.h>

class _CRTIMP bad_cast : public std::exception {
public:
    __CLR_OR_THIS_CALL bad_cast(_In_z_ const char * _Message = "bad cast");
    __CLR_OR_THIS_CALL bad_cast(_In_ const bad_cast & _Bad_cast);
    virtual __CLR_OR_THIS_CALL ~bad_cast();

#ifdef CRTDLL
private:
    /* This is aliased to public:bad_cast(const char * const &) to provide */
    /* the old, non-conformant constructor. */
    bad_cast(_In_z_ const char * const * _Message);
#endif  /* CRTDLL */
};

class _CRTIMP bad_typeid : public std::exception {
public:
    bad_typeid(_In_z_ const char * _Message = "bad typeid");
    bad_typeid(_In_ const bad_typeid &);
    virtual ~bad_typeid();
};

class _CRTIMP __non_rtti_object : public bad_typeid {
public:
    __non_rtti_object(_In_z_ const char * _Message);
    __non_rtti_object(_In_ const __non_rtti_object &);
    virtual ~__non_rtti_object();
};

#ifdef __RTTI_OLDNAMES
/* Some synonyms for folks using older standard */
typedef type_info Type_info;
typedef bad_cast Bad_cast;
typedef bad_typeid Bad_typeid;
#endif  /* __RTTI_OLDNAMES */

#endif  /* _TICORE */

#endif  /* _SYSCRT */

#endif  /* RC_INVOKED */

#pragma pack(pop)

#endif  /* _INC_TYPEINFO */
