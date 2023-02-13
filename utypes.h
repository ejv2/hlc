/*
 * utypes.h - useful type aliases
 * Copyright (C) Ethan Marshall - 2023
 *
 * Requirements: stdint.h stddef.h
 *
 * This header may be more useful to you if you edit it for your purposes.
 * Different styles which I often use are split into blocks. These are mainly
 * the Go blocks and the Rust blocks.
 *
 * Some types are rough approximations of the true versions (such as uintptr_t,
 * which is optional according to POSIX).
 */

#ifdef HLC_AUTO_INCLUDE
#define UTYPE_AUTO_INCLUDE
#endif

#ifdef UTYPE_AUTO_INCLUDE
#include <stddef.h>
#include <stdint.h>
#endif

/* shorthand integer width aliases */
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

/* shorthand unsigned integer width aliases */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* misc integer shorthands */
typedef unsigned int uint;
typedef uint8_t      byte;
typedef int64_t      isize;
typedef size_t       usize;

/* floating point shorthands */
typedef float            f32;
typedef double           f64;
typedef long double      f128;

/* somewhat portable integer pointers */
#if _POSIX_C_SOURCE >= 200112L
typedef intptr_t  iptr;
typedef uintptr_t uptr;
#else
typedef long long          iptr;
typedef unsigned long long uptr;
#endif

/* longer integer width aliases */
typedef i8  int8;
typedef i16 int16;
typedef i32 int32;
typedef i64 int64;

/* longer unsigned integer width aliases */
typedef u8  uint8;
typedef u16 uint16;
typedef u32 uint32;
typedef u64 uint64;

/* longer floating point aliases */
typedef f32  float32;
typedef f64  float64;
typedef f128 float128;

/* longer integer pointer aliases */
typedef iptr intptr;
typedef uptr uintptr;
