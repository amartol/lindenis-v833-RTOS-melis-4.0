//----------------------------------------------------------------------------------------------------------//
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//    I BBMMBB      BBMMBBI     BBI EEMMFFI BBMMI     BBI EEMMFFI BBMMI       BBMMBBI   EEMMI EEMMLL        //
//    EEI   EEFF  EEI   LLBB  EEMMBBI I BBEE  I EE  EEMMBBI I BBEE  I EE    EEI   LLBB    EEBBI I BB        //
//  I FF        I EE      BBI   EEI     EEI   I EE    EEI     EEI   I EE  I EE      BBI   EEI   I EE        //
//  LLFF        LLEE      EELL  EEI     EEI   I EE    EEI     EEI   I EE  LLEE      EELL  EEI   I EE        //
//  I EE      I I BB      EEI   EEI     EEI   I EE    EEI     EEI   I EE  I BB      EEI   EEI   I EE        //
//    BBLL  I I   BBFF  I EE    EEI     EEI   I BB    EEI     EEI   I BB    BBFF  I EE    BBI   I EE        //
//    I BBMMEE    I BBMMBB    EEMMMMLLBBMMBBLLMMMMEEEEMMMMLLBBMMBBLLMMMMEE  I BBMMBB    EEMMMMLLBBMMBB      //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//----------------------------------------------------------------------------------------------------------//
//                                                                                                          //
//                                                Scott System                                              //
//                                                                                                          //
//                               (c) Copyright 2006-2007, Scottyu China                                     //
//                                                                                                          //
//                                           All Rights Reserved                                            //
//                                                                                                          //
// File    : error.h                                                                                        //
// By      : scottyu                                                                                        //
// Version : V1.00                                                                                          //
// Time    : 2008-04-14 13:32:00                                                                            //
//                                                                                                          //
//----------------------------------------------------------------------------------------------------------//
//                                                                                                          //
// HISTORY                                                                                                  //
//                                                                                                          //
// 1 2008-04-14 13:32:00                                                                                    //
//                                                                                                          //
//                                                                                                          //
//                                                                                                          //
//----------------------------------------------------------------------------------------------------------//

#ifndef ___ERROR__H___
#define ___ERROR__H___  1

#include <stdio.h>
#include <string.h>
#include <memory.h>

#define MSGLEN                      1024

//------------------------------------------------------------------------------------------------------------
//#define USE_COLOR_CONSOLE         1 //使用彩色控制台窗口
//------------------------------------------------------------------------------------------------------------

#ifndef SCOTT_DEBUG
#define SCOTT_DEBUG
#endif
//------------------------------------------------------------------------------------------------------------
//错误输出形式
//------------------------------------------------------------------------------------------------------------
#ifdef SCOTT_DEBUG
#define CONSOLE_OUTPUT_ERROR        1
#define GUI_OUTPUT_ERROR            2
#define ERROR_OUTPUT                CONSOLE_OUTPUT_ERROR
//#define ERROR_OUTPUT              GUI_OUTPUT_ERROR
#else //SOFT_RELEASE
#endif //

void Msg(const char *fmt, ...);

#ifdef SCOTT_DEBUG
void Warn(const char *fun, const char *file, int line, const char *fmt, ...);
#else //SOFT_RELEASE
#define Warn(x)            {}
#endif //

#ifdef SCOTT_DEBUG
void Err(const char *fun, const char *file, int line, const char *fmt, ...);
#else //SOFT_RELEASE
#define Err(x)             {}
//void Err(char * fun, char * file, int line, char *fmt, ...);
#endif //

#ifdef SCOTT_DEBUG
void Dbg(char *fun, char *file, int line, char *fmt, ...);
#else //SOFT_RELEASE
#define Dbg(x)             {}
#endif //


#ifdef SCOTT_DEBUG
void SPRINTF(char *szErr, char *fmt, ...);
#else //SOFT_RELEASE
#define SPRINTF(x)          {}
#endif //

#endif //___ERROR__H___
