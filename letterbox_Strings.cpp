
#include "letterbox.h"
	
typedef struct {
	unsigned long	index;
	char			str[256];
} TableString;



TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,					"",
	StrID_Name,					"tl_Letterbox",
	StrID_Description,			"Generates a simple letterbox with specified ratio",
    

	StrID_ModeName,				"Mode",
	StrID_ModeChoices,			"Preset|"
								"Custom",
    
	StrID_PresetName,			"Presets",
	StrID_PresetChoices,		" 4/3|"
								"16/9|"
								"1.85|"
								"2.35|"
								"2.39|"
								"2.40",


	StrID_PresetChoicesChanged,	" 4/3|"
								"16/9|"
								"1.85|"
								"2.35|"
								"2.39|"
								"2.40",
    

	StrID_SliderName,			"Custom",

	
    

    StrID_TrspName,             "Transparent",
    StrID_TrspDescription,      "Generate on a transparent",
    
    StrID_ColorName,			"Fill scopes",
    
    StrID_sample_Param_Name,    "Transform Content",
    StrID_PointName,			"Center",
    StrID_SliderResizeName,     "Resize",
    
    StrID_detect_Param_Name,    "Detection",
    StrID_Layer_detectName,     "Layer Source",
    StrID_CheckboxName,			"Detect Layer Ratio",
    StrID_CheckboxCaption,		"Try to detect the black scopes",
    
    StrID_settings_Param_Name,   "Settings",
    StrID_SizeSourceName,		"Source Size",
    StrID_SizeSourceChoices,	"Layer|"
                                "Composition",
    StrID_ForceSName,             "Force the scale",
    StrID_ForceSDescription,      "Force the layer to keep the full size",
    
     StrID_AEGP_GET_POSITION,           "Return Position of the layer", //invisible by final user
     StrID_AEGP_GET_ANCHORPOINT,       "Return anchor point of the layer", //invisible by final user
     StrID_AEGP_GET_SCALE,             "Return scale of the layer", //invisible by final user

    
    StrID_AEGP_GET_COMP_PIX_RATIO,     "Return Pixel Aspect ratio of the comp", //invisible by final user
    StrID_AEGP_GETCOMP_SIZE,          "Return width height of the compo",
    
    StrID_Err_LoadSuite,		"Couldn't acquire suite.",
	StrID_Err_FreeSuite,		"Couldn't free suite.",

	StrID_PresetNameDisabled,	"Preset Locked",

	StrID_GeneralError,			"Error handling Changed Param."
};

char	*GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
};
