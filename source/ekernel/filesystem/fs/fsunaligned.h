/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                  File System
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : fsunaligned.h
* By      : Sunny
* Version : v1.0
* Date    : 2011-1-15
* Descript: code is extracted from linux.
* Update  : date                auther      ver     notes
*           2011-3-15 15:39:19   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __FS_UNALIGNED_H__
#define __FS_UNALIGNED_H__

#include    "fstypedef.h"

#undef  __force
#define __force
#undef  __typeof__
#define __typeof__  typeof

/*
 * The ARM can do unaligned accesses itself.
 *
 * The strange macros are there to make sure these can't
 * be misused in a way that makes them not work on other
 * architectures where unaligned accesses aren't as simple.
 */
static inline void bad_unaligned_access_length(void)
{
}

/*
 * What is the most efficient way of loading/storing an unaligned value?
 *
 * That is the subject of this file.  Efficiency here is defined as
 * minimum code size with minimum register usage for the common cases.
 * It is currently not believed that long longs are common, so we
 * trade efficiency for the chars, shorts and longs against the long
 * longs.
 *
 * Current stats with gcc 2.7.2.2 for these functions:
 *
 *  ptrsize get:    code    regs    put:    code    regs
 *  1       1   1       1   2
 *  2       3   2       3   2
 *  4       7   3       7   3
 *  8       20  6       16  6
 *
 * gcc 2.95.1 seems to code differently:
 *
 *  ptrsize get:    code    regs    put:    code    regs
 *  1       1   1       1   2
 *  2       3   2       3   2
 *  4       7   4       7   4
 *  8       19  8       15  6
 *
 * which may or may not be more efficient (depending upon whether
 * you can afford the extra registers).  Hopefully the gcc 2.95
 * is inteligent enough to decide if it is better to use the
 * extra register, but evidence so far seems to suggest otherwise.
 *
 * Unfortunately, gcc is not able to optimise the high word
 * out of long long >> 32, or the low word from long long << 32
 */

#define __get_unaligned_2_le(__p)                   \
    (__p[0] | __p[1] << 8)

#define __get_unaligned_2_be(__p)                   \
    (__p[0] << 8 | __p[1])

#define __get_unaligned_4_le(__p)                   \
    (__p[0] | __p[1] << 8 | __p[2] << 16 | __p[3] << 24)

#define __get_unaligned_4_be(__p)                   \
    (__p[0] << 24 | __p[1] << 16 | __p[2] << 8 | __p[3])

#define __get_unaligned_8_le(__p)                   \
    ((unsigned long long)__get_unaligned_4_le((__p+4)) << 32 |  \
     __get_unaligned_4_le(__p))

#define __get_unaligned_8_be(__p)                   \
    ((unsigned long long)__get_unaligned_4_be(__p) << 32 |      \
     __get_unaligned_4_be((__p+4)))

#define __get_unaligned_le(ptr) ({          \
        const char *__p = (const char *)ptr;    \
        __u64 __val;                            \
        switch (sizeof(*ptr)) {                 \
            case 1:                                 \
                __val = *(const __u8 *)__p;         \
                break;                              \
            case 2:                                 \
                __val = __get_unaligned_2_le(__p);  \
                break;                              \
            case 4:                                 \
                __val = __get_unaligned_4_le(__p);  \
                break;                              \
            case 8:                                 \
                __val = __get_unaligned_8_le(__p);  \
                break;                              \
            default:                                \
                bad_unaligned_access_length();      \
        };                                      \
        (__force __typeof__(*(ptr)))__val;      \
    })

#define __get_unaligned_be(ptr) ({          \
        const char *__p = (const char *)ptr;    \
        __u64 __val;                            \
        switch (sizeof(*ptr)) {                 \
            case 1:                                 \
                __val = *(const __u8 *)__p;         \
                break;                              \
            case 2:                                 \
                __val = __get_unaligned_2_be(__p);  \
                break;                              \
            case 4:                                 \
                __val = __get_unaligned_4_be(__p);  \
                break;                              \
            case 8:                                 \
                __val = __get_unaligned_8_be(__p);  \
                break;                              \
            default:                                \
                bad_unaligned_access_length();      \
        };                                      \
        (__force __typeof__(*(ptr)))__val;      \
    })

static inline void __put_unaligned_2_le(__u32 __v, register __u8 *__p)
{
    *__p++ = __v;
    *__p++ = __v >> 8;
}

static inline void __put_unaligned_2_be(__u32 __v, register __u8 *__p)
{
    *__p++ = __v >> 8;
    *__p++ = __v;
}

static inline void __put_unaligned_4_le(__u32 __v, register __u8 *__p)
{
    __put_unaligned_2_le(__v >> 16, __p + 2);
    __put_unaligned_2_le(__v, __p);
}

static inline void __put_unaligned_4_be(__u32 __v, register __u8 *__p)
{
    __put_unaligned_2_be(__v >> 16, __p);
    __put_unaligned_2_be(__v, __p + 2);
}

static inline void __put_unaligned_8_le(const unsigned long long __v, register __u8 *__p)
{
    /*
     * tradeoff: 8 bytes of stack for all unaligned puts (2
     * instructions), or an extra register in the long long
     * case - go for the extra register.
     */
    __put_unaligned_4_le(__v >> 32, __p + 4);
    __put_unaligned_4_le(__v, __p);
}

static inline void __put_unaligned_8_be(const unsigned long long __v, register __u8 *__p)
{
    /*
     * tradeoff: 8 bytes of stack for all unaligned puts (2
     * instructions), or an extra register in the long long
     * case - go for the extra register.
     */
    __put_unaligned_4_be(__v >> 32, __p);
    __put_unaligned_4_be(__v, __p + 4);
}

/*
 * Try to store an unaligned value as efficiently as possible.
 */
#define __put_unaligned_le(val,ptr)                 \
    ({                          \
        (void)sizeof(*(ptr) = (val));           \
        switch (sizeof(*(ptr))) {           \
            case 1:                     \
                *(ptr) = (val);             \
                break;                  \
            case 2: __put_unaligned_2_le((__force u16)(val),(__u8 *)(ptr)); \
                break;                  \
            case 4: __put_unaligned_4_le((__force u32)(val),(__u8 *)(ptr)); \
                break;                  \
            case 8: __put_unaligned_8_le((__force u64)(val),(__u8 *)(ptr)); \
                break;                  \
            default: __bug_unaligned_x(ptr);        \
                break;                  \
        }                       \
        (void) 0;                   \
    })

#define __put_unaligned_be(val,ptr)                 \
    ({                          \
        (void)sizeof(*(ptr) = (val));           \
        switch (sizeof(*(ptr))) {           \
            case 1:                     \
                *(ptr) = (val);             \
                break;                  \
            case 2: __put_unaligned_2_be((__force u16)(val),(__u8 *)(ptr)); \
                break;                  \
            case 4: __put_unaligned_4_be((__force u32)(val),(__u8 *)(ptr)); \
                break;                  \
            case 8: __put_unaligned_8_be((__force u64)(val),(__u8 *)(ptr)); \
                break;                  \
            default: __bug_unaligned_x(ptr);        \
                break;                  \
        }                       \
        (void) 0;                   \
    })

/*
 * Select endianness
 */
#ifndef __ARMEB__
#define get_unaligned   __get_unaligned_le
#define put_unaligned   __put_unaligned_le
#else
#define get_unaligned   __get_unaligned_be
#define put_unaligned   __put_unaligned_be
#endif

#endif /* __FS_UNALIGNED_H__ */
