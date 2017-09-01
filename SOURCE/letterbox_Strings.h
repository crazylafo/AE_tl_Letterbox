#pragma once

typedef enum {
	StrID_NONE, 
	StrID_Name,
	StrID_Description,

	StrID_ModeName,
	StrID_ModeChoices, 

	StrID_PresetName,
	StrID_PresetChoices,
	StrID_PresetChoicesChanged,



	StrID_SliderName,  

	StrID_CheckboxName,
	StrID_CheckboxCaption,
    
    StrID_TrspName,
    StrID_TrspDescription,
    
    StrID_ColorName,
    
    StrID_sample_Param_Name,
    
    StrID_PointName,
    
    StrID_SliderResizeName,
    
    
	StrID_Err_LoadSuite,
	StrID_Err_FreeSuite,
    
    

	StrID_TopicName,
	StrID_TopicNameDisabled, 
	StrID_PresetNameDisabled,

	StrID_GeneralError,
	
	StrID_NUMTYPES
} StrIDType;

#ifdef __cplusplus
extern "C" {
#endif
char	*GetStringPtr(int strNum);
#ifdef __cplusplus
}
#endif