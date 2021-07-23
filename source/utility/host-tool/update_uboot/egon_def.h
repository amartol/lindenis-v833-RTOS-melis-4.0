/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*                                                  All Rights Reserved
*
* File Name   : egon_def.h
*
* Author      : Gary.Wang
*
* Version     : 1.1.0
*
* Date        : 2009.05.21
*
* Description :
*
* Others      : None at present.
*
*
* History     :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang      2009.05.21       1.1.0        build the file
*
************************************************************************************************************************
*/
#ifndef  __egon_def_h
#define  __egon_def_h

#include "egon_mem_distribute.h"

#define BOOT_PUB_HEAD_VERSION           "1100"    // X.X.XX
#define EGON_VERSION                    "1100"    // X.X.XX

#define SUNII_PLATFORM_VALUE           "SUNII"
#ifndef PLATFORM
#define PLATFORM                        SUNII_PLATFORM_VALUE
#endif

#define EGON2_DRAM_BASE                 0x40000000
#define EGON2_DRAM_SIZE                 0x02000000
#define EGON2_SRAM_BASE                 0x0
#define EGON2_SRAM_SIZE                 ( 32 * 1024 )
#define EGON2_RESET_BASE                0xFFFF0000

#define EGON2_MMU_BASE                  0x20000
#define EGON2_REGS_BASE                 0x01c00000
#define EGON2_REGS_SIZE                 0x02000000

#define NF_ALIGN_SIZE                   SZ_8K
#define SF_ALIGN_SIZE                   512       // change 256 to 512. by Gary, 2009-12-10 11:46:51
#define IE_ALIGN_SIZE                   512       // change  64 to 512. by Gary, 2009-12-10 11:46:51

#define SPI_NOR_SCT_SIZE                512       // add for spi nor. by Gary,2009-12-8 11:47:17
#define SPI_NOR_SCT_SIZE_WIDTH          9         // add for spi nor. by Gary,2009-12-8 11:47:17

#define BOOT_SECTOR_SIZE                512
#define BOOT_SECTOR_SIZE_WIDTH          9

#define MASK_TOTAL                      0x80000000
#define DEFAULT_VALUE                   0x0
#define STAMP_VALUE                     0x5F0A6C39

#define BOOT0_BASE                      ( EGON2_SRAM_BASE                       )
#define BOOT1_BASE                      ( EGON2_DRAM_BASE  + EGON2_BOOT1_OFFSET )
#define FEL_BASE                        ( EGON2_RESET_BASE + 0x20               )
#define SCRIPT_BASE                     ( BOOT1_BASE       + 0x00B00000         )


//通用的，和GPIO相关的数据结构
typedef struct _normal_gpio_cfg
{
    __u8      port;                       //端口号
    __u8      port_num;                   //端口内编号
    __s8      mul_sel;                    //功能编号
    __s8      pull;                       //电阻状态
    __s8      drv_level;                  //驱动驱动能力
    __s8      data;                       //输出电平
    __u8      reserved[2];                //保留位，保证对齐
}
normal_gpio_cfg;

/*
typedef enum __DRAM_TYPE
{
    DRAM_TYPE_DDR   =1,
    DRAM_TYPE_DDR2  =2,
    DRAM_TYPE_DDR3  =3
}__dram_type_e;
*/

typedef struct _boot_dram_para_t
{
    __u32           dram_baseaddr;
    __u32           dram_clk;
    __u32           dram_type;
    __u32           dram_rank_num;
    __u32           dram_chip_density;
    __u32           dram_io_width;
    __u32           dram_bus_width;
    __u32           dram_cas;
    __u32           dram_zq;
    __u32           dram_odt_en;
    __u32           dram_size;
    __u32           dram_tpr0;
    __u32           dram_tpr1;
    __u32           dram_tpr2;
    __u32           dram_tpr3;
    __u32           dram_tpr4;
    __u32           dram_tpr5;
    __u32           dram_emr1;
    __u32           dram_emr2;
    __u32           dram_emr3;
} boot_dram_para_t;

/******************************************************************************/
/*                              file head of Boot                             */
/******************************************************************************/
typedef struct _Boot_file_head
{
    __u32  jump_instruction;   // one intruction jumping to real code
    __u8   magic[8];           // ="eGON.BT0" or "eGON.BT1",  not C-style string.
    __u32  check_sum;          // generated by PC
    __u32  length;             // generated by PC
    __u32  pub_head_size;      // the size of boot_file_head_t
    __u8   pub_head_vsn[4];    // the version of boot_file_head_t
    __u8   file_head_vsn[4];   // the version of boot0_file_head_t or boot1_file_head_t
    __u8   Boot_vsn[4];        // Boot version
    __u8   eGON_vsn[4];        // eGON version
    __u8   platform[8];        // platform information
} boot_file_head_t;

typedef struct _boot_para_info_t
{
    __u8   blkmagic[16];          // "ePDK-Magic-Block", not C-style string.
    __u8   magic[8];
    __u8   eGON_vsn[4];           // eGON version
    __u8   Boot_vsn[4];           // Boot version
    __u32  reserved[20];
} boot_para_info_t;

#endif     //  ifndef __egon_def_h
