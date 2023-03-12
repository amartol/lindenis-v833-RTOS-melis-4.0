/* LzmaLib.c -- LZMA library wrapper
2008-08-05
Igor Pavlov
Public domain */

#include "LzmaEnc.h"
#include "LzmaDec.h"
#include "LzmaLib.h"
#include "stdlib.h"
#include <stdint.h>

static void *SzAlloc(void *p, uint32_t size)
{
    p = p;
    return malloc(size);
}
static void SzFree(void *p, void *address)
{
    p = p;
    free(address);
}
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

MY_STDAPI LzmaCompress(unsigned char *dest, UInt32  *destLen,
                       const unsigned char *src, UInt32  srcLen,
                       unsigned char *outProps, UInt32 *outPropsSize)
{
    CLzmaEncProps props;
    LzmaEncProps_Init(&props);
    props.level = 5;
    props.dictSize = 32 * 1024;
    props.lc = 3;
    props.lp = 0;
    props.pb = 2;
    props.fb = 32;
    props.numThreads = 1;

    return LzmaEncode(dest, destLen, src, srcLen,
                      &props, outProps, outPropsSize,
                      0, NULL, &g_Alloc, &g_Alloc);
}


MY_STDAPI LzmaUncompress(unsigned char *dest, UInt32  *destLen,
                         const unsigned char *src, UInt32 *srcLen,
                         const unsigned char *props, UInt32 propsSize)
{
    ELzmaStatus status;
    return LzmaDecode(dest, destLen,
                      src, srcLen,
                      props, (unsigned)propsSize,
                      LZMA_FINISH_ANY, &status, &g_Alloc);
}
