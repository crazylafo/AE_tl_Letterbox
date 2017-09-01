

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
#include "PrSDKAESupport.h"


#define	MAJOR_VERSION		1
#define	MINOR_VERSION		3
#define	BUG_VERSION			3
#define	STAGE_VERSION		PF_Stage_RELEASE
#define	BUILD_VERSION		2

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

typedef struct {
	PF_Boolean		initializedB;
	AEGP_PluginID	my_id;
} my_global_data, *my_global_dataP, **my_global_dataH;

typedef struct {
	PF_State		state;
} my_sequence_data, *my_sequence_dataP, **my_sequence_dataH;

typedef struct {
    A_u_char	blue, green, red, alpha;
} PF_Pixel_BGRA_8u;

typedef struct {
    A_u_char	Pr, Pb, luma, alpha;
} PF_Pixel_VUYA_8u;

typedef struct {
    PF_FpShort	blue, green, red, alpha;
} PF_Pixel_BGRA_32f;
/*
typedef struct {
    PF_FpShort	Pr, Pb, luma, alpha;
} PF_Pixel_VUYA_32f;*/




typedef struct {
	PF_FpLong           userRatioF;
	PF_FpLong           layerRatioF;
	PF_FpLong           InWidthF;
	PF_FpLong           InHeightF;
	PF_FpLong           PixRatioNumF;
	PF_FpLong           PixRatioDenF;
	A_long              x_tA;
	A_long              y_tA;
	PF_Boolean          PoTransparentB;
    PF_InData           in_data;
    PF_SampPB           samp_pb;
    PF_FpLong           PreseTvalueF;
    PF_FpLong           SlidervalueF;
    PF_Pixel            Color;
    PF_Pixel32          Color32;
    PF_Fixed            x_offF;
    PF_Fixed            y_offF;
    PF_FpLong           scaleFactor;
    
    


} prerender_stuff, *pre_render_stuffP, **pre_render_stuffH;


enum {
    LETB_INPUT = 0,
	LETB_MODE,
	LETB_PRESET,
	LETB_SLIDER,
	LETB_CHECKBOX,
    LETB_TRSP,
    LETB_COLOR,
    LETB_GR1,
    LETB_CENTER,
    LETB_RESIZE,
    END_TOPIC_GR1,
	LETB_NUM_PARAMS
};

enum {
	LETB_MODE_DISK_ID = 1,
	LETB_PRESET_DISK_ID,
	LETB_GROUP_START_DISK_ID,
	LETB_SLIDER_DISK_ID,
	LETB_CHECKBOX_DISK_ID,
    LETB_TRSP_DISK_ID,
    LETB_COLOR_DISK_ID,
    TOPIC_GR1_DISK_ID,
    LETB_CENTER_DISK_ID,
    LETB_RESIZE_DISK_ID,
    END_TOPIC_GR1_DISK_ID,
    LETB_GROUP_END_DISK_ID,
	
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
