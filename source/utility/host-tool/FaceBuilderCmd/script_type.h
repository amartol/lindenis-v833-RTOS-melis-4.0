//------------------------------------------------------------------------------------------------------------
//
// script_type.h
//
// 2009-5-22 17:16:38
//
// sunny
//
//------------------------------------------------------------------------------------------------------------

#ifndef __SCRIPT_TYPE__H__
#define __SCRIPT_TYPE__H__     1


#ifndef MAX_PATH
#define MAX_PATH                260                 //最大路径长度
#endif //MAX_PATH

#define SECTION_NAME_LEN        32                  //
#define RES_NAME_LEN            64                  // 资源名最大长度
#define COMMENT_LEN             256                 // Comment最大长度
#define EMPTY_PATH              "NULL"              // 空路径


//------------------------------------------------------------------------------------------------------------
//  资源系统信息 ： 512 Bytes
//------------------------------------------------------------------------------------------------------------
//#pragma pack(push, 1)
typedef struct tag_FACE_SYS_INFO
{
    u16  Scriptver;             //
    char UIstyle[32];           //
    char Name[32];              //
    u8   Zero0;
    char Author[32];            //
    char Solution[256];         //
    u8   Zero1;
    char Stylename[32];         //
    u16  StyleID;               //
    u8   Reserved[512 - 390];   // align by : 512
} __attribute__((packed)) FACE_SYS_INFO;
//#pragma pack(pop)


//------------------------------------------------------------------------------------------------------------
// 本结构的大小
//------------------------------------------------------------------------------------------------------------
#define FACE_SYS_INFO_SIZE  sizeof(FACE_SYS_INFO)



//------------------------------------------------------------------------------------------------------------
// 资源信息 ： 1024 Bytes
//------------------------------------------------------------------------------------------------------------
#define FACE_RES_ITEM_VER   0x0100

//#pragma pack(push, 1)
typedef struct tag_FACE_RES_ITEM
{
    u16   Ver;                      //
    u16   Size;                     //
    u16   ID;                       //
    char  Section[32];              //
    char  Name[RES_NAME_LEN];                   //
    char  File[MAX_PATH];           //
    u8    Zero0;
    char  CommentCN[COMMENT_LEN];   //
    u8    Zero1;
    char  CommentEN[COMMENT_LEN];   //
    u8    Reserved[1024 - MAX_PATH - 72 - 2 * COMMENT_LEN];
} __attribute__((packed))FACE_RES_ITEM;
//#pragma pack(pop)

//------------------------------------------------------------------------------------------------------------
// 本结构的大小
//------------------------------------------------------------------------------------------------------------
#define FACE_RES_ITEM_SIZE          sizeof(FACE_RES_ITEM)


//------------------------------------------------------------------------------------------------------------
// THE END !
//------------------------------------------------------------------------------------------------------------

#endif //__SCRIPT_TYPE__H__


