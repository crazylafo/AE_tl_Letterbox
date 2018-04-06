

#pragma once

#ifndef LETTERBOX_H
#define LETTERBOX_H

#include "AEConfig.h"
#ifdef AE_OS_WIN
    typedef unsigned short PixelType;
	#include <windows.h>
	#include <wingdi.h>
	#include <assert.h>
	#include <math.h>
	#include "stdlib.h"
#endif

#define PF_DEEP_COLOR_AWARE 1
//#include "AE_Effect.h"
//#include "AE_EffectCB.h"
//#include "AE_EffectCBSuites.h"


#include "letterbox_Strings.h"
#include "entry.h"
#include "AE_EffectUI.h"
#include "AE_Macros.h"
#include "AE_EffectSuites.h"
#include "AE_AdvEffectSuites.h"
#include "AEFX_SuiteHelper.h"
#include "Smart_Utils.h"
#include "Param_Utils.h"
#include "String_Utils.h"
#include "AEGP_SuiteHandler.h"

//FOR PREMIER
#include "PrSDKAESupport.h"




#define	MAJOR_VERSION		2
#define	MINOR_VERSION		3
#define	BUG_VERSION			5
#define	STAGE_VERSION		PF_Stage_RELEASE
#define	BUILD_VERSION		2

typedef enum {
    REF_LAYER= 1,
    REF_COMPOSITION,
    REF_SIZE = REF_COMPOSITION
} current_source;


typedef enum {
	MODE_BASIC= 1,
	MODE_ADVANCED,
	MODE_SIZE = MODE_ADVANCED
} current_mode;



typedef enum {
	LETB_PRESET_FOT = 1,     //4/3
	LETB_PRESET_SON,         //16/9
	LETB_PRESET_OHF,         //1.85
	LETB_PRESET_TTF,         //2.35
	LETB_PRESET_TTN,         //2.39
	LETB_PRESET_TFZ,         //2.40
	LETB_PRESET_SIZE = LETB_PRESET_TFZ
} current_preset;


typedef enum {
    TIME_FRAME= 1,
    TIME_LAYER,
    TIME_SIZE = TIME_LAYER
} time_mode;





typedef struct {
    A_u_char	blue, green, red, alpha;
} PF_Pixel_BGRA_8u;

typedef struct {
    A_u_char	Pr, Pb, luma, alpha;
} PF_Pixel_VUYA_8u;

typedef struct {
    PF_FpShort	blue, green, red, alpha;
} PF_Pixel_BGRA_32f;

typedef struct {
    A_u_short	Pr, Pb, luma, alpha;
} PF_Pixel_VUYA_16u;




typedef struct {
    A_long                  compWidthA;
    A_long                 compHeightA;
    AEGP_DownsampleFactor dsfP;
} CompositionProp;


typedef struct {
    PF_Boolean		initializedB;
    AEGP_PluginID	my_id;
} my_global_data, *my_global_dataP, **my_global_dataH;

typedef struct {
    PF_State		state;
} my_sequence_data, *my_sequence_dataP, **my_sequence_dataH;


typedef struct{
	PF_FpLong           userRatioF;
	PF_FpLong           layerRatioF;
    PF_FpLong           layerWidthF;
    PF_FpLong           layerHeightF;
	PF_FpLong           InputWidthF;
	PF_FpLong           InputHeightF;
	PF_FpLong           PixRatioNumF;
	PF_FpLong           PixRatioDenF;
    PF_InData           in_data;
    PF_SampPB           samp_pb;

	PF_Boolean          PoTransparentB;
    PF_FpLong           PreseTvalueF;
    PF_FpLong           SlidervalueF;
    PF_Pixel            Color;
    PF_Pixel32          Color32;
    
    PF_FpLong           scaleFactorF;
    PF_FpLong           scaleFactorxF;
    PF_FpLong           scaleFactoryF;
    PF_Boolean          compModeB;
    A_long              forceSizeB;
    
    PF_Fixed            x_tA;
    PF_Fixed            y_tA;
    PF_Fixed            x_offF;
    PF_Fixed            y_offF;
    PF_FpLong           x_offsetF;
    PF_FpLong           y_offsetF;
    
    PF_FpLong            compWidthF;
    PF_FpLong            compHeightF;
    PF_FpLong            compRatioF;
    
    PF_FpLong            letoffxF;
    PF_FpLong            letoffyF;
    
    PF_FpLong            compoffxF;
    PF_FpLong            compoffyF;
    
    PF_FpLong            layerPx;
    PF_FpLong            layerPy;
	PF_FpLong            layerSx;
	PF_FpLong            layerSy;

    
} prerender_letP, *pre_render_letPP, **pre_render_letPH;


enum {
    	LETB_INPUT = 0,
	LETB_MODE,
	LETB_PRESET,
	LETB_SLIDER,
    LETB_TRSP,
    LETB_COLOR,
    
    LETB_GR1,
    LETB_CENTER,
    LETB_RESIZE,
    END_TOPIC_GR1,
    
    LETB_GR2,
    LETB_LAYER_ANALYS,
    LETB_TIME_ANALYS,
    LETB_COLOR_ANALYS,
    LETB_BTN_ANALYS,
    END_TOPIC_GR2,
    
    LETB_GR3,
    LETB_SIZE_SOURCE,
    END_TOPIC_GR3,
    
	LETB_NUM_PARAMS
};

enum {
	LETB_MODE_DISK_ID = 1,
	LETB_PRESET_DISK_ID,
	LETB_RATIO_DISK_ID,
    LETB_TRSP_DISK_ID,
    LETB_COLOR_DISK_ID,
    TOPIC_GR1_DISK_ID,
    LETB_CENTER_DISK_ID,
    LETB_RESIZE_DISK_ID,
    END_TOPIC_GR1_DISK_ID,
    TOPIC_GR2_DISK_ID,
    LETB_LAYER_ANALYS_DISK_ID,
    LETB_TIME_ANALYS_DISK_ID,
    LETB_COLOR_ANALYS_DISK_ID,
    LETB_DETECT_DISK_ID,
    END_TOPIC_GR2_DISK_ID,
    TOPIC_GR3_DISK_ID,
    LETB_SIZE_SOURCE_DISK_ID,
    END_TOPIC_GR3_DISK_ID,

};

extern "C" {
DllExport	PF_Err 
EntryPointFunc(	
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra );
}	
#endif  // LETTERBOX_H
