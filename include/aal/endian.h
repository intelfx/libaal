/* Copyright (C) 2001, 2002, 2003 by Hans Reiser, licensing governed by
   libaal/COPYING.
   
   endian.h -- endianess translation macros. This is a number of macro because
   macro is better for performance than to use functions which are determining
   the translation kind in the run time. */

#ifndef AAL_ENDIAN_H
#define AAL_ENDIAN_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <aal/types.h>

#define __aal_get_octave(x, n)		( ((x) >> (8 * (n))) & 0xff )

#define __aal_swap16(x)			( (__aal_get_octave(x, 0) << 8)		\
					+ (__aal_get_octave(x, 1) << 0) )
	
#define __aal_swap32(x)			( (__aal_get_octave(x, 0) << 24)	\
					+ (__aal_get_octave(x, 1) << 16)	\
					+ (__aal_get_octave(x, 2) << 8)		\
					+ (__aal_get_octave(x, 3) << 0) )
	
#define __aal_swap64(x)			( (__aal_get_octave(x, 0) << 56)	\
					+ (__aal_get_octave(x, 1) << 48)	\
					+ (__aal_get_octave(x, 2) << 40)	\
					+ (__aal_get_octave(x, 3) << 32)	\
					+ (__aal_get_octave(x, 4) << 24)	\
					+ (__aal_get_octave(x, 5) << 16)	\
					+ (__aal_get_octave(x, 6) << 8)		\
					+ (__aal_get_octave(x, 7) << 0) )

#define aal_swap16(x)			((uint16_t) __aal_swap16((uint16_t)x))
#define aal_swap32(x)			((uint32_t) __aal_swap32((uint32_t)x))
#define aal_swap64(x)			((uint64_t) __aal_swap64((uint64_t)x))

/*
  Endianess is determined by configure script in the configuring time, that is 
  before compiling the package.
*/
#ifdef WORDS_BIGENDIAN

#  define CPU_TO_LE16(x)		aal_swap16(x)
#  define CPU_TO_BE16(x)		(x)
#  define CPU_TO_LE32(x)		aal_swap32(x)
#  define CPU_TO_BE32(x)		(x)
#  define CPU_TO_LE64(x)		aal_swap64(x)
#  define CPU_TO_BE64(x)		(x)

#  define LE16_TO_CPU(x)		aal_swap16(x)
#  define BE16_TO_CPU(x)		(x)
#  define LE32_TO_CPU(x)		aal_swap32(x)
#  define BE32_TO_CPU(x)		(x)
#  define LE64_TO_CPU(x)		aal_swap64(x)
#  define BE64_TO_CPU(x)		(x)

#else

#  define CPU_TO_LE16(x)		(x)
#  define CPU_TO_BE16(x)		aal_swap16(x)
#  define CPU_TO_LE32(x)		(x)
#  define CPU_TO_BE32(x)		aal_swap32(x)
#  define CPU_TO_LE64(x)		(x)
#  define CPU_TO_BE64(x)		aal_swap64(x)

#  define LE16_TO_CPU(x)		(x)
#  define BE16_TO_CPU(x)		aal_swap16(x)
#  define LE32_TO_CPU(x)		(x)
#  define BE32_TO_CPU(x)		aal_swap32(x)
#  define LE64_TO_CPU(x)		(x)
#  define BE64_TO_CPU(x)		aal_swap64(x)

#endif

#define aal_get_leXX(xx, p, field)	(LE##xx##_TO_CPU (get_unaligned((&(p)->field))))
#define aal_set_leXX(xx, p, field, val)	put_unaligned(CPU_TO_LE##xx(val), (&(p)->field))

#define aal_get_le16(p, field) 		aal_get_leXX(16, p, field)
#define aal_set_le16(p, field, val) 	aal_set_leXX(16, p, field, val)

#define aal_get_le32(p, field) 		aal_get_leXX(32, p, field)
#define aal_set_le32(p, field, val)	aal_set_leXX(32, p, field, val)

#define aal_get_le64(p, field) 		aal_get_leXX(64, p, field)
#define aal_set_le64(p, field, val) 	aal_set_leXX(64, p, field, val)

#endif
