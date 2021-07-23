//------------------------------------------------------------------------------------------------------------
//
// fsprocess_if.h
//
// scott
//
// 2009-12-24 9:49:11
//
//------------------------------------------------------------------------------------------------------------


#ifndef __FSPROCESS_IF___H____
#define __FSPROCESS_IF___H____  1


#include "fsprocess_plug_type.h"

//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------

typedef void   *(*pGetInterface)(const char *plug_guid);

typedef u32(*pBuildFileSystem)(pGetInterface GetInterface,
                               const char *config_file,
                               u8 *SplitBuf, u32 SplitLen, bool bCutSplit);
//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------

#pragma pack(push, 1)
typedef struct tag_FSPROCESS_IF
{
    pBuildFileSystem    BuildFileSystem;    //

} FSPROCESS_IF;
#pragma pack(pop)

//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
#endif //__FSPROCESS_IF___H____






