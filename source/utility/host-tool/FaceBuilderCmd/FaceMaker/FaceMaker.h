//------------------------------------------------------------------------------------------------------------
//
// facemaker.h
//
// 2009-5-23 9:15:52
//
// sunny
//
//------------------------------------------------------------------------------------------------------------


#ifndef ___FACEMAKER_PARSER_H____
#define ___FACEMAKER_PARSER_H____   1


#include "../if/facemaker_plug_type.h"


//------------------------------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------------------------------
#ifdef PLUGINAPI

#else
//#define PLUGINAPI extern "C" __declspec(dllimport)
#endif //

//------------------------------------------------------------------------------------------------------------
#define PLUGIN_TYPE             FACEMAKER_PLUGIN_TYPE   //
#define PLUGIN_NAME             "facemaker"             //
#define PLUGIN_VERSION          0x0100                  //
#define PLUGIN_AUTHOR           "sunny"                 //
#define PLUGIN_COPYRIGHT        "sunny"                 //


//------------------------------------------------------------------------------------------------------------
//插件的通用接口
//------------------------------------------------------------------------------------------------------------
#include "../if/plugin_common.h"


//------------------------------------------------------------------------------------------------------------
// THE END
//------------------------------------------------------------------------------------------------------------

#endif //___FACEMAKER_PARSER_H____

