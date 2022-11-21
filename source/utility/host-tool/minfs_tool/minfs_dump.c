/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                        Mini ROM Image File System
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : minfs_dump.c
* By      : Sunny
* Version : v1.0
* Date    : 2011-3-25
* Descript: Mini rom image dump tool.
* Update  : date                auther      ver     notes
*           2011-3-25 19:18:19  Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include "minfs_tool_i.h"

#define DOTTEXT_OFFSET (strlen(MINFS_DEFAULT_SECTION_NAME) + 1)
#define DOTRODATA_OFFSET (strlen(MINFS_DEFAULT_SECTION_NAME) + 1 + strlen(".text") + 1)
#define DOTDATA_OFFSET (strlen(MINFS_DEFAULT_SECTION_NAME) + 1 + strlen(".text") + 1 + strlen(".rodata") + 1)
#define DOTBSS_OFFSET (strlen(MINFS_DEFAULT_SECTION_NAME) + 1 + strlen(".text") + 1 + strlen(".rodata") + 1 + strlen(".data") + 1)
#define DOTSHSTRTAB_OFFSET (strlen(MINFS_DEFAULT_SECTION_NAME) + 1 + strlen(".text") + 1 + strlen(".rodata") + 1 + strlen(".data") + 1 + strlen(".bss") + 1)
#define MAGIC_OFFSET (strlen(MINFS_DEFAULT_SECTION_NAME) + 1 + strlen(".text") + 1 + strlen(".rodata") + 1 + strlen(".data") + 1 + strlen(".bss") + 1 + strlen(".shstrtab") + 1)

__bool MINFS_Identify(FILE *hFile)
{
    __minfs_hdr_t MFSHdr;

    memset(&MFSHdr, 0, MINFS_HDR_LEN);

    //load minfs header from image file
    fseek(hFile, 0, SEEK_SET);
    fread(&MFSHdr, 1, MINFS_HDR_LEN, hFile);
    if (memcmp(MFSHdr.Magic, MINFS_MAGIC, MINFS_MAGIC_LEN - 1) == 0)
    {
        return 1;
    }

    return 0;
}

__u32 MINFS_GetImageSize(FILE *hFile)
{
    __minfs_hdr_t MFSHdr;

    memset(&MFSHdr, 0, MINFS_HDR_LEN);

    //load minfs header from image file
    fseek(hFile, 0, SEEK_SET);
    fread(&MFSHdr, 1, MINFS_HDR_LEN, hFile);
    if (memcmp(MFSHdr.Magic, MINFS_MAGIC, MINFS_MAGIC_LEN - 1) == 0)
    {
        //valid minfs image file
        return MFSHdr.Size;
    }

    return 0;
}

__s32 DumpDataToFile(const char *pFile, __u8 *pBuffer, __u32 BufferLen)
{
    FILE *hFile;

    hFile = fopen(pFile, "wb+");
    if (hFile == NULL)
    {
        MSG("create file [%s] failed", pFile);
        return EPDK_FAIL;
    }

    fwrite(pBuffer, 1, BufferLen, hFile);
    fclose(hFile);

    return EPDK_OK;
}

__s32 DumpCompressedDataToFile(const char *pFile, __u32 unPackSize,
                               __u8 *pBuffer, __u32 BufferLen)
{
    __u8    *pUncompressData;
    __u32    UncompressLen;

    //allocate buffer to store uncompressed data
    pUncompressData = (__u8 *)malloc(unPackSize);
    if (pUncompressData == NULL)
    {
        MSG("allocate buffer to store uncompressed data failed\n");
        return EPDK_FAIL;
    }

    //uncompress file data
    UncompressLen = unPackSize;
    if (MINFS_Uncompress(pUncompressData, &UncompressLen,
                         pBuffer, &BufferLen) != EPDK_OK)
    {
        MSG("uncompress file data failed\n");
        free(pUncompressData);
        return EPDK_FAIL;
    }

    //store uncompress data to file
    DumpDataToFile(pFile, pUncompressData, UncompressLen);

    //release uncompressed data buffer
    free(pUncompressData);

    return EPDK_OK;
}

__s32 DumpModuleDataToFile(__dump_context_t *pContext,
                           __minfs_dentry_t *pDEntry,
                           const char *pStorePath)
{
    __u32       index;
    __u32       offset;
    __u32       SectionNumber;
    Elf32_Ehdr *pELFHdr;
    __u8       *pBuffer;
    Elf32_Shdr *pSectionHdr;
    __u8       *pSectionData;
    __minfs_section_hdr_t *pMFSSectionEntry;

#if DEBUG
    printf("Unpacking module %s of size %d\n", pDEntry->MetaData, pDEntry->UnPackSize);
#endif
    
    //allocate buffer for elf file data
    pBuffer = (__u8 *)malloc(pDEntry->UnPackSize + 300);
    if (pBuffer == NULL)
    {
        MSG("allocate buffer failed\n");
        return EPDK_FAIL;
    }
    memset(pBuffer, 0, pDEntry->UnPackSize + 300);
    
    Elf32_Phdr* programHeader = (Elf32_Phdr*)(pBuffer + sizeof(__elf32_head_t));
    
    SectionNumber = (pDEntry->ExtentLen / MINFS_SECTION_HDR_LEN);
    offset        = (sizeof(__elf32_head_t) + sizeof(Elf32_Phdr) + \
                     (SectionNumber + 2) * sizeof(Elf32_Shdr));
    pSectionHdr   = (Elf32_Shdr *)(pBuffer + sizeof(__elf32_head_t) + sizeof(Elf32_Phdr));
    pSectionData  = pBuffer + offset;
    pMFSSectionEntry = (__minfs_section_hdr_t *)(pDEntry->MetaData + \
                       MINFS_ALIGN(pDEntry->NameLen, MINFS_NAME_ALIGN));
    programHeader->p_offset = offset;
    
    u32 totalFileSize = 0;
    u32 totalMemSize = 0;
    
    pSectionHdr->size = 0;
    pSectionHdr->type = 0;
    pSectionHdr->name = 0;
    pSectionHdr->flags = 0;
    pSectionHdr->offset = 0;
    pSectionHdr->addr = 0;
    pSectionHdr->addralign = 0;
    pSectionHdr->link = 0;
    
    pSectionHdr++;

    for (index = 0; index < SectionNumber; index++)
    {
#if DEBUG
        printf("Section %d\n", index);
        printf("    ELF offset %d\n", offset);
        printf("    file offset %d\n", pMFSSectionEntry->Offset);
        printf("    flags 0x%x\n", pMFSSectionEntry->Flags);
        printf("    size %d\n", pMFSSectionEntry->Size);
        printf("    recsize %d\n", pMFSSectionEntry->RecSize);
        printf("    recunpacksize %d\n", pMFSSectionEntry->RecUnPackSize);
        printf("    type %d\n", pMFSSectionEntry->Type);
        printf("    compressed %d\n", pMFSSectionEntry->Attribute & MINFS_SECTION_ATTR_COMPRESS);
        printf("    magic %d\n", pMFSSectionEntry->Attribute & MINFS_SECTION_ATTR_MAGIC);
#endif
        
        pSectionHdr->offset = offset;
        pSectionHdr->name   = 0;
        pSectionHdr->size   = pMFSSectionEntry->Size;
        pSectionHdr->addr   = pMFSSectionEntry->VAddr;
        pSectionHdr->flags  = pMFSSectionEntry->Flags;
        pSectionHdr->type   = pMFSSectionEntry->Type;
        pSectionHdr->addralign = MINFS_DATA_ALIGN;
        
        if (index == 0) {
            programHeader->p_vaddr = programHeader->p_paddr = pSectionHdr->addr;
        }
        
        switch (pSectionHdr->flags)
        {
            case 0x6:
                pSectionHdr->name = DOTTEXT_OFFSET;
                break;
                
            case 0x2:
                pSectionHdr->name = DOTRODATA_OFFSET;
                break;
                
            case 0x3:
                if (pMFSSectionEntry->RecSize == 0)
                    pSectionHdr->name = DOTBSS_OFFSET;
                else
                    pSectionHdr->name = DOTDATA_OFFSET;
                break;
        }
        
        if (pMFSSectionEntry->Attribute & MINFS_SECTION_ATTR_MAGIC)
            pSectionHdr->name = MAGIC_OFFSET;
        
        totalMemSize += pSectionHdr->size;
        totalFileSize += pMFSSectionEntry->RecUnPackSize;

        //copy section data to elf file data buffer
        if (pSectionHdr->size)
        {
            __u8    *pMFSSectionData =  pContext->pBuffer + \
                                        pDEntry->Offset + \
                                        pMFSSectionEntry->Offset;
            __u32    SectionDataLen  = 0;
            if (pMFSSectionEntry->Attribute & MINFS_SECTION_ATTR_COMPRESS)
            {
                __u8    *unCompressBuffer;
                __u32    unCompressLen;

                //uncompress section data
                unCompressLen = pMFSSectionEntry->RecUnPackSize;
                unCompressBuffer = (__u8 *)malloc(unCompressLen);
                if (unCompressBuffer == NULL)
                {
                    free(pBuffer);
                    return EPDK_FAIL;
                }
                MINFS_Uncompress(unCompressBuffer, &unCompressLen, \
                                 pMFSSectionData, &(pMFSSectionEntry->RecSize));

                //store uncompress data to elf buffer
                memcpy(pSectionData, unCompressBuffer, unCompressLen);
                SectionDataLen = unCompressLen;
            }
            else
            {
                //copy section data directly
                if (pMFSSectionEntry->RecSize)
                {
                    memcpy(pSectionData, pMFSSectionData, pMFSSectionEntry->RecSize);
                }
                SectionDataLen = pMFSSectionEntry->RecSize;
            }
            //adjust allocate location for next section
            offset += MINFS_ALIGN(SectionDataLen, MINFS_DATA_ALIGN);
            pSectionData += MINFS_ALIGN(SectionDataLen, MINFS_DATA_ALIGN);
        }
        //next section
        pSectionHdr++;
        pMFSSectionEntry++;
    }

    //strtab section
    pSectionHdr->offset = offset;
    strcpy(pSectionData, MINFS_DEFAULT_SECTION_NAME);
    pSectionData += (strlen(MINFS_DEFAULT_SECTION_NAME) + 1);
    offset       += (strlen(MINFS_DEFAULT_SECTION_NAME) + 1);
    strcpy(pSectionData, ".text");
    pSectionData += (strlen(".text") + 1);
    offset += (strlen(".text") + 1);
    strcpy(pSectionData, ".rodata");
    pSectionData += (strlen(".rodata") + 1);
    offset += (strlen(".rodata") + 1);
    strcpy(pSectionData, ".data");
    pSectionData += (strlen(".data") + 1);
    offset += (strlen(".data") + 1);
    strcpy(pSectionData, ".bss");
    pSectionData += (strlen(".bss") + 1);
    offset += (strlen(".bss") + 1);
    strcpy(pSectionData, ".shstrtab");
    pSectionData       += (strlen(".shstrtab") + 1);
    offset       += (strlen(".shstrtab") + 1);
    strcpy(pSectionData, MINFS_MODULE_MAGIC);
    pSectionData       += (strlen(MINFS_MODULE_MAGIC) + 1);
    offset       += (strlen(MINFS_MODULE_MAGIC) + 1);
    pSectionHdr->name = DOTSHSTRTAB_OFFSET;
    pSectionHdr->size = offset - pSectionHdr->offset;
    pSectionHdr->type = EELF_SHT_STRTAB;
    MINFS_ALIGN(offset, MINFS_DATA_ALIGN);

    //construct elf file header
    pELFHdr = (Elf32_Ehdr *)(pBuffer);
    pELFHdr->ident[EI_MAG0] = ELFMAG0;
    pELFHdr->ident[EI_MAG1] = ELFMAG1;
    pELFHdr->ident[EI_MAG2] = ELFMAG2;
    pELFHdr->ident[EI_MAG3] = ELFMAG3;
    pELFHdr->ident[EI_CLASS] = ELFCLASS32;
    pELFHdr->ident[EI_DATA] = ELFDATA2LSB;
    pELFHdr->ident[EI_OSABI] = ELFOSABI_SYSV;
    pELFHdr->ident[EI_VERSION] = EV_CURRENT;
    pELFHdr->type           = ET_EXEC;
    pELFHdr->machine        = EM_RISCV;
    pELFHdr->version        = EV_CURRENT;
    pELFHdr->entry          = 0;
    pELFHdr->phoff          = sizeof(__elf32_head_t);
    pELFHdr->shoff          = sizeof(__elf32_head_t) + sizeof(Elf32_Phdr);
    pELFHdr->flags          = 5;
    pELFHdr->ehsize         = sizeof(__elf32_head_t);
    pELFHdr->phentsize      = sizeof(Elf32_Phdr);
    pELFHdr->phnum          = 1;
    pELFHdr->shentsize      = sizeof(Elf32_Shdr);
    pELFHdr->shnum          = index + 2;
    pELFHdr->shstrndx       = index + 1;
    
    programHeader->p_type = PT_LOAD;
    programHeader->p_memsz = totalMemSize;
    programHeader->p_filesz = totalFileSize;
    programHeader->p_flags = PF_R | PF_W | PF_X;
    programHeader->p_align = MINFS_DATA_ALIGN;

    DumpDataToFile(pStorePath, pBuffer, offset);

    //release elf file buffer
    free(pBuffer);

    return EPDK_OK;
}

__s32 DumpFile(__dump_context_t *pContext,
               __minfs_dentry_t *pDEntry,
               const char *pStorePath)
{
    if (pDEntry->Attribute & MINFS_ATTR_MODULE)
    {
        DumpModuleDataToFile(pContext, pDEntry, pStorePath);
    }
    else
    {
        __u8 *pBuffer = pContext->pBuffer + pDEntry->Offset;
        if (pDEntry->Attribute & MINFS_ATTR_COMPRESS)
        {
            DumpCompressedDataToFile(pStorePath, pDEntry->UnPackSize,
                                     pBuffer, pDEntry->Size);
        }
        else
        {
            //save data to buffer directly
            DumpDataToFile(pStorePath, pBuffer, pDEntry->Size);
        }
    }

    return EPDK_OK;
}

__s32 DumpDirNest(__dump_context_t *pContext, __minfs_dentry_t *pDir, __u8 *pPn)
{
    char             *tmpPath;
    __minfs_dentry_t *pDEntry;
    __u32             DEntryLen;
    char              MFSDEntryName[MAX_PATH];
    char              DumpName[MAX_PATH];

    tmpPath   = malloc(MAX_PATH);
    pDEntry   = (__minfs_dentry_t *)(pContext->pBuffer + pDir->Offset);
    DEntryLen = 0;
    while (DEntryLen < pDir->Size)
    {
        memcpy(MFSDEntryName, pDEntry->MetaData, pDEntry->NameLen);
        MFSDEntryName[pDEntry->NameLen] = 0;
        sprintf(tmpPath, "%s/%s", pPn, MFSDEntryName);
        sprintf(DumpName, "%s/%s", pContext->StoreDir, tmpPath);
        if (pDEntry->Attribute & MINFS_ATTR_DIR)
        {
            pContext->DirNum++;
            //_mkdir(DumpName);
            printf("Unpacking %s\n", tmpPath);
            mkdir(DumpName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

            //nest
            DumpDirNest(pContext, pDEntry, tmpPath);
        }
        else
        {
            pContext->FileNum++;
            DumpFile(pContext, pDEntry, DumpName);
        }

        //next dentry
        DEntryLen += pDEntry->RecLen;
        pDEntry = (__minfs_dentry_t *)((__u8 *)(pDEntry) + pDEntry->RecLen);
    }

    free(tmpPath);
    return EPDK_OK;
}

__s32 DumpMinfsImage(__dump_context_t *pContext)
{
    __minfs_hdr_t    *MFSHdr;
    __minfs_dentry_t  pRootDir;
    __u8              tmpPath[MAX_PATH];

    //minfs header (super block)
    MFSHdr = (__minfs_hdr_t *)(pContext->pBuffer);

    //the root have no parset
    memset(tmpPath, 0, MAX_PATH);

    //construct root directory
    memset(&pRootDir, 0, sizeof(__minfs_dentry_t));
    pRootDir.Offset    = MFSHdr->RootDirOffset;
    pRootDir.Size      = MFSHdr->RootDirSize;
    pRootDir.RecLen    = 0;
    pRootDir.Attribute = MINFS_ATTR_DIR;
    pRootDir.NameLen   = 0;
    pRootDir.ExtentLen = 0;

    return DumpDirNest(pContext, &pRootDir, tmpPath);
}

__s32 MINFS_DumpImage(const char *pDstDir, const char *pSrcFile)
{
    FILE            *hFile;
    __u32            ImageLen;
    __dump_context_t Context;

    if ((pDstDir == NULL) || (pSrcFile == NULL))
    {
        return EPDK_FAIL;
    }

    //open image file
    hFile = fopen(pSrcFile, "rb");
    if (hFile == NULL)
    {
        MSG("MINFS_DumpImage : open file [%s] failed\n", pSrcFile);
        return 0;
    }

    //identify minfs image file
    if (!MINFS_Identify(hFile))
    {
        MSG("invalid minfs image to dump\n");
        return EPDK_FAIL;
    }
    memset(&Context, 0, sizeof(__dump_context_t));

    //get image file length
    ImageLen = MINFS_GetImageSize(hFile);
    if (ImageLen == 0)
    {
        MSG("invalid minfs image length\n");
        return EPDK_FAIL;
    }

    //allocate buffer for image file data
    Context.pBuffer = (__u8 *)malloc(ImageLen);
    if (Context.pBuffer == NULL)
    {
        MSG("allocate buffer for minfs image dump failed\n");
        fclose(hFile);
        return EPDK_FAIL;
    }
    memset(Context.pBuffer, 0, ImageLen);
    Context.BufferLen = ImageLen;

    //load image data to buffer
    fseek(hFile, SEEK_SET, 0);
    fread(Context.pBuffer, 1, ImageLen, hFile);
    fclose(hFile);

    strcpy(Context.StoreDir, pDstDir);
    if (DumpMinfsImage(&Context) != EPDK_OK)
    {
        MSG("dump minfs image failed\n");
        free(Context.pBuffer);
        return EPDK_FAIL;
    }

    free(Context.pBuffer);

    //dump debug information
    MSG("------------------------------------------------------------------\n");
    MSG("dump minfs image file [%s] succeeded\n", pSrcFile);
    MSG("Directory Number   : [%8d]\n", Context.DirNum);
    MSG("File number        : [%8d]\n", Context.FileNum);
    MSG("------------------------------------------------------------------\n");

    return EPDK_OK;
}
