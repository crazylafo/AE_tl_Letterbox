#include "letterbox.h"



static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	
	PF_Err	err			=	PF_Err_NONE;
	AEGP_SuiteHandler		suites(in_data->pica_basicP);
	PF_AppPersonalTextInfo	personal_info;

	AEFX_CLR_STRUCT(personal_info);

	ERR(suites.AppSuite4()->PF_GetPersonalInfo(&personal_info));
		
	if (!err) {
		suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
                                              "%s v%d.%d\r%s",
                                              STR(StrID_Name),
                                              MAJOR_VERSION,
                                              MINOR_VERSION,
                                              STR(StrID_Description));
	}
	return err;
}




static PF_Err
GlobalSetup (
             PF_InData		*in_data,
             PF_OutData		*out_data,
             PF_ParamDef		*params[],
             PF_LayerDef		*output )
{
	PF_Err 				err 		= PF_Err_NONE;
	
	PF_Handle			globH		= NULL;
	my_global_dataP		globP		= NULL;

	AEGP_SuiteHandler		suites(in_data->pica_basicP);

	out_data->my_version = PF_VERSION(	MAJOR_VERSION,
										MINOR_VERSION,
										BUG_VERSION,
										STAGE_VERSION,
										BUILD_VERSION);

	out_data->out_flags		=	PF_OutFlag_PIX_INDEPENDENT 			|
								PF_OutFlag_SEND_UPDATE_PARAMS_UI	|
								PF_OutFlag_USE_OUTPUT_EXTENT		|
                                PF_OutFlag_WIDE_TIME_INPUT          |
                                PF_OutFlag_DEEP_COLOR_AWARE;


	
	out_data->out_flags2	=	PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG	|
                                PF_OutFlag2_SUPPORTS_SMART_RENDER				|
								PF_OutFlag2_FLOAT_COLOR_AWARE					|
								PF_OutFlag2_DOESNT_NEED_EMPTY_PIXELS			|
                                PF_OutFlag2_I_USE_COLORSPACE_ENUMERATION        |
								PF_OutFlag2_I_MIX_GUID_DEPENDENCIES|
                                PF_OutFlag2_AUTOMATIC_WIDE_TIME_INPUT;
    
    
    

    // For Premiere - declare supported pixel formats
    if (in_data->appl_id == 'PrMr') {
        
        auto pixelFormatSuite =
        AEFX_SuiteScoper<PF_PixelFormatSuite1>(in_data,
                                               kPFPixelFormatSuite,
                                               kPFPixelFormatSuiteVersion1,
                                               out_data);
        
        //	Add the pixel formats we support in order of preference.
        (*pixelFormatSuite->ClearSupportedPixelFormats)(in_data->effect_ref);
        (*pixelFormatSuite->AddSupportedPixelFormat)(
                                                     in_data->effect_ref,
                                                     PrPixelFormat_VUYA_4444_16u);
        (*pixelFormatSuite->AddSupportedPixelFormat)(
                                                     in_data->effect_ref,
                                                     PrPixelFormat_BGRA_4444_32f);
        (*pixelFormatSuite->AddSupportedPixelFormat)(
                                                     in_data->effect_ref,
                                                     PrPixelFormat_BGRA_4444_8u);
        (*pixelFormatSuite->AddSupportedPixelFormat)(
                                                     in_data->effect_ref,
                                                     PrPixelFormat_VUYA_4444_8u);
     
       
    }

	
	globH	=	suites.HandleSuite1()->host_new_handle(sizeof(my_global_data));
	
	if (globH) {
		globP = reinterpret_cast<my_global_dataP>(suites.HandleSuite1()->host_lock_handle(globH));
		if (globP) {
			globP->initializedB = TRUE;

			if (in_data->appl_id != 'PrMr') {
				// This is only needed for the AEGP suites, which Premiere Pro doesn't support
				ERR(suites.UtilitySuite3()->AEGP_RegisterWithAEGP(NULL, STR(StrID_Name), &globP->my_id));
			}
            if (!err){
                out_data->global_data 	= globH;
			}
		}
		suites.HandleSuite1()->host_unlock_handle(globH);
	} else	{
		err = PF_Err_INTERNAL_STRUCT_DAMAGED;
	}
	return err;
}


static PF_Err	
ParamsSetup(
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output)
{
	PF_Err				err					= PF_Err_NONE;
	PF_ParamDef			def;

    
  
	
    AEFX_CLR_STRUCT(def);

	def.flags		=	PF_ParamFlag_SUPERVISE			|
                        PF_ParamFlag_CANNOT_TIME_VARY   |
						PF_ParamFlag_CANNOT_INTERP;
						

    
	PF_ADD_POPUP(		STR(StrID_ModeName),
						MODE_SIZE,
						MODE_BASIC,
						STR(StrID_ModeChoices),
						LETB_MODE_DISK_ID);
    
    AEFX_CLR_STRUCT(def);

    
    
    def.flags = PF_ParamFlag_SUPERVISE |
                PF_ParamFlag_CANNOT_INTERP;
    


	PF_ADD_POPUP(   STR(StrID_PresetName),
					LETB_PRESET_SIZE,
					LETB_PRESET_TFZ,
					STR(StrID_PresetChoices),
					LETB_PRESET_DISK_ID);
    
    AEFX_CLR_STRUCT(def);
    
	PF_ADD_FLOAT_SLIDERX(STR(StrID_SliderName),
					0, 
					10,
					0, 
					10,
					2.35,
					PF_Precision_THOUSANDTHS,
					0,
					PF_ParamFlag_EXCLUDE_FROM_HAVE_INPUTS_CHANGED,
					LETB_RATIO_DISK_ID);
	


	AEFX_CLR_STRUCT(def);
    
    
    def.flags		|=	PF_ParamFlag_SUPERVISE |
                        PF_ParamFlag_CANNOT_TIME_VARY;
    

    
    PF_ADD_CHECKBOX(STR(StrID_TrspName),
                    STR(StrID_TrspDescription),
                    FALSE,
                    0,
                    LETB_TRSP_DISK_ID);
    
    AEFX_CLR_STRUCT(def);
    
    def.flags		|=	PF_ParamFlag_SUPERVISE;
    
    PF_ADD_COLOR(	STR(StrID_ColorName),
                 0,
                 0,
                 0,
                 LETB_COLOR_DISK_ID);
    
    AEFX_CLR_STRUCT(def);
    
    PF_ADD_TOPICX (STR(StrID_sample_Param_Name),
                   0,
                   TOPIC_GR1_DISK_ID);
    
    AEFX_CLR_STRUCT(def);
    
    def.flags		|=	PF_ParamFlag_SUPERVISE;


    PF_ADD_POINT(	STR(StrID_PointName),
                 50,
                 50,
                 0,
                 LETB_CENTER_DISK_ID);
    
    AEFX_CLR_STRUCT(def);
    

    
    PF_ADD_FLOAT_SLIDERX(STR(StrID_SliderResizeName),
                         0,
                         1000,
                         0,
                         100,
                         100,
                         PF_Precision_HUNDREDTHS,
                         0,
                         0,
                         LETB_RESIZE_DISK_ID);
     AEFX_CLR_STRUCT(def);
    PF_END_TOPIC (END_TOPIC_GR1_DISK_ID);
    AEFX_CLR_STRUCT(def);

    if (in_data->appl_id != 'PrMr')
    {
    PF_ADD_TOPICX (STR(StrID_detect_Param_Name),
                   0,
                   TOPIC_GR2_DISK_ID);
    AEFX_CLR_STRUCT(def);
    
    
    def.flags		|=	PF_ParamFlag_SUPERVISE |
                    PF_ParamFlag_CANNOT_TIME_VARY;
    
    PF_ADD_LAYER (STR(StrID_Layer_detectName),
                  PF_LayerDefault_MYSELF,
                  LETB_LAYER_ANALYS_DISK_ID);
    AEFX_CLR_STRUCT(def);
    
    
    
    def.flags		=	PF_ParamFlag_SUPERVISE			|
    PF_ParamFlag_CANNOT_TIME_VARY                       |
    PF_ParamFlag_CANNOT_INTERP;
    
    PF_ADD_POPUP(		STR( StrID_Layer_timeAnalysName),
                 TIME_SIZE,
                 TIME_FRAME,
                 STR(StrID_Time_ModeChoices),
                 LETB_TIME_ANALYS_DISK_ID);
    
    AEFX_CLR_STRUCT(def);
    

    def.flags		|=	PF_ParamFlag_SUPERVISE;
    PF_ADD_COLOR(	STR( StrID_Layer_colorAnalysName),
                 0,
                 0,
                 0,
                 LETB_COLOR_ANALYS_DISK_ID);
    
     AEFX_CLR_STRUCT(def);
    
    def.ui_flags	= PF_PUI_STD_CONTROL_ONLY;
    PF_ADD_BUTTON(STR(StrID_CheckboxName), STR(StrID_CheckboxCaption), FALSE, PF_ParamFlag_SUPERVISE, LETB_DETECT_DISK_ID);
    AEFX_CLR_STRUCT(def);
    
    
    PF_END_TOPIC (END_TOPIC_GR2_DISK_ID);
    AEFX_CLR_STRUCT(def);
    
    PF_ADD_TOPICX (STR(StrID_settings_Param_Name),
                   0,
                   TOPIC_GR3_DISK_ID);
    AEFX_CLR_STRUCT(def);
    
    def.flags		=	PF_ParamFlag_SUPERVISE			|
    PF_ParamFlag_CANNOT_TIME_VARY   |
    PF_ParamFlag_CANNOT_INTERP;

    
    PF_ADD_POPUP(STR(StrID_SizeSourceName),
                 REF_LAYER,
                 REF_COMPOSITION,
                 STR(StrID_SizeSourceChoices),
                 LETB_SIZE_SOURCE_DISK_ID);
    
    

    AEFX_CLR_STRUCT(def);
    
    PF_END_TOPIC (END_TOPIC_GR3_DISK_ID);
    AEFX_CLR_STRUCT(def);
    
        out_data->num_params = LETB_NUM_PARAMS;
        
    }
    else
    {
        out_data->num_params = LETB_NUM_PARAMS -9; // NO TWO TOPICS FOR PREMIERE
        
    }
	return err;
}


static void
//DEFINE PRESETS RATIO IN BASIC UI
GetModeValue(
             A_long			Mode,
             A_long          *ModeValue)
{
    switch (Mode) {
        case 1:
            *ModeValue = 0;
            break;
            
        case 2:
            *ModeValue = 1;
            break;

            
    }
}


static void
//DEFINE PRESETS RATIO IN BASIC UI FOR SMART FX
GetPresetRatioValue(
                    A_long			PRESET,
                    PF_FpLong		*presetratioF)
{
    switch (PRESET) {
        case LETB_PRESET_FOT:
            *presetratioF = 1.33;
            break;
            
        case LETB_PRESET_SON:
            *presetratioF = 1.77;
            break;
            
        case LETB_PRESET_OHF:
            *presetratioF = 1.85;
            break;
            
        case LETB_PRESET_TTF:
            *presetratioF = 2.35;
            break;
        case LETB_PRESET_TTN:
            *presetratioF = 2.39;
            break;
            
        case LETB_PRESET_TFZ:
            *presetratioF = 2.40;
            break;
    }
    
}




//DETECT RATIO IN THE LAYER
static void
GetRatioFromWorld (
                   PF_InData		*in_data,
                   PF_EffectWorld  *detectWorldP,
                   PF_PixelFormat  pxformat,
                   PF_FpLong	   *detectedRatioF,
                   PF_PixelFloat   analysColor)
{

    PF_FpLong InputWidthF ,InputHeightF ,PixRatioNumF,PixRatioDenF;
    //A_long state =0;//value to indicate the state of the detection.
    
    InputWidthF  = detectWorldP->width;
    InputHeightF  = detectWorldP->height;
    PixRatioNumF = in_data->pixel_aspect_ratio.num;
    PixRatioDenF = in_data->pixel_aspect_ratio.den;
    
    PF_FpLong scale_x = in_data->downsample_x.num / (float)in_data->downsample_x.den,
    scale_y = in_data->downsample_y.num / (float)in_data->downsample_y.den;
    InputWidthF  *= scale_x;
    InputHeightF  *= scale_y;


    PF_FpShort Tolerencepx_blue = analysColor.blue;
	PF_FpShort Tolerencepx_green = analysColor.green;
	PF_FpShort Tolerencepx_red = analysColor.red;

    
    A_long halfWidthA = A_long (InputWidthF*0.5);
    A_long halfHeightA = A_long (InputHeightF *0.5);

	PF_FpLong ratioHF, ratioVF;
    ratioHF =0;
    ratioVF =0;
    
 
    for (A_long i =1; i< halfHeightA; i++)
    {

        for (A_long j =1; j< A_long (InputWidthF); j++)
        {
            PF_PixelFloat PixelValue;
            AEFX_CLR_STRUCT(PixelValue);
             GetPixelValue(detectWorldP, pxformat, j, i, &PixelValue);
            
			if (PixelValue.blue > Tolerencepx_blue || PixelValue.green > Tolerencepx_green || PixelValue.red > Tolerencepx_red)
			{
                if (i <5)
                {
                    ratioHF =0;
                    
                }
                else
                {
                    ratioHF = (((double)InputWidthF) *  PixRatioNumF) / ((double)InputHeightF *PixRatioDenF - 2 * i);
                }
				i = halfHeightA - 1;
				break;

			}
			
        }
    }
    
    for (A_long k  =1; k<halfWidthA; k++)
    {
        for (A_long l =1; l< A_long (InputHeightF); l++)
        {
            PF_PixelFloat PixelValueh;
            AEFX_CLR_STRUCT(PixelValueh);
            GetPixelValue (detectWorldP, pxformat, k, l, &PixelValueh);

			if (PixelValueh.blue > Tolerencepx_blue || PixelValueh.green > Tolerencepx_green || PixelValueh.red > Tolerencepx_red)
			{
				ratioVF = (((double)InputWidthF - 2 * k) *  PixRatioNumF ) / ((double)InputHeightF *PixRatioDenF);
				k = halfWidthA - 1;
				break;
			}

        }
    }
    PF_FpLong layerRatio = (((double)InputWidthF) *  PixRatioNumF) / ((double)InputHeightF *PixRatioDenF); //ratio input from layer
	if (ratioHF > layerRatio)
	{
		*detectedRatioF = ratioHF;

	}
    
	else if (ratioVF <layerRatio &&
			ratioVF >0.000 )
	{
		*detectedRatioF = ratioVF;
	}

    else
    {
		*detectedRatioF = 0;
    }


}


static PF_FpLong
CalculateBox(
	void		*refcon,
	PF_FpLong		xL,
	PF_FpLong		yL)

{
	PF_Err		err = PF_Err_NONE;


    PF_FpLong CondBlackHupF, CondBlackHdownF, CondBlackVleftF, CondBlackVrightF, offsetCompxF, offsetCompyF;
	prerender_letP	*letP = reinterpret_cast<prerender_letP*>(refcon);

	int userRatioInt = int(letP->userRatioF * 100);
	int layerRatioInt = int(letP->layerRatioF * 100);
    
    
    
    //Modification of offsets here.

	//definitions for horizontal letterbox
    CondBlackHupF = ((letP->InputHeightF - (letP->InputWidthF/(letP->userRatioF)))/ 2)-(0.5*letP->letoffyF)+letP->compoffyF;
    if(letP->compoffyF >0)
    {
        offsetCompyF = ABS(letP->compoffyF);
        
    }
    else
    {
        offsetCompyF = - ABS(letP->compoffyF);
    }
    
    CondBlackHdownF =   letP->layerHeightF -(((letP->InputHeightF - (letP->InputWidthF/(letP->userRatioF)))/ 2)-(0.5*letP->letoffyF))+  offsetCompyF;

    

	//definitions for verticals letterbox
    CondBlackVleftF = ((letP->InputWidthF - letP->InputHeightF *  letP->userRatioF)/2)-(0.5*letP->letoffxF)+letP->compoffxF;
    if(letP->compoffxF >0)
    {
        offsetCompxF = ABS(letP->compoffxF);
        
    }
    else
    {
        offsetCompxF = - ABS(letP->compoffxF);
    }
    
    CondBlackVrightF = letP->layerWidthF   - (((letP->InputWidthF - letP->InputHeightF *  letP->userRatioF)/2)-(0.5*letP->letoffxF)) +  offsetCompxF;


	if (letP)
	{
		if ((letP->userRatioF == 0.0) || (userRatioInt == layerRatioInt))
		{
			return 1.0;
		}
		else if (letP->userRatioF > letP->layerRatioF) //if ratio from UI up to footage's ratio--> then mask the height
		{
			if  (yL < (CondBlackHupF) || yL >(CondBlackHdownF))
			{
				return 0.0;
			}
			else
			{
				return 1.0;
			}
		}
		else if (letP->userRatioF < letP->layerRatioF) //if ratio from UI under to ratio footage--> then mask the width
		{

			if (xL <  CondBlackVleftF || xL > CondBlackVrightF)
			{
				return 0.0;
			}
			else
			{
				return 1.0;
			}

		}

		else
		{
			return 1.0;
		}

	}
	return err;
}

static PF_Err 
PixelFunc8 (	
	void 		*refcon,
	A_long 		xL,
	A_long 		yL,
	PF_Pixel8 	*inP,
	PF_Pixel8 	*outP)
{
	prerender_letP*	letP = reinterpret_cast<prerender_letP*>(refcon);
    PF_InData			*in_data	= &(letP->in_data);
    PF_Err				err			= PF_Err_NONE;
    PF_Fixed                new_xFi		= 0,
                            new_yFi		= 0;
    PF_Pixel8 *PosOutP = outP;
    

    if (letP){

        if (letP->PoTransparentB == TRUE)
        {
            outP->alpha = A_u_short(PF_MAX_CHAN8*(1-(CalculateBox(refcon, xL, yL))));
            outP->red = letP->Color.red;
            outP->green = letP->Color.green;
            outP->blue = letP->Color.blue;
        }
        else
        {

            new_xFi = PF_Fixed( (((A_long)xL << 16) + letP->x_offF)/ letP->scaleFactorF);
            new_yFi = PF_Fixed( (((A_long)yL << 16) + letP->y_offF)/ letP->scaleFactorF);
            
            ERR(letP->in_data.utils->subpixel_sample (in_data->effect_ref,
                                                            new_xFi,
                                                            new_yFi,
                                                            &letP->samp_pb,
                                                            PosOutP));

            outP->alpha	= inP->alpha;
            outP->red = A_u_short (     PosOutP->red    *   CalculateBox(refcon, xL, yL)+(letP->Color.red * (1- CalculateBox(refcon, xL, yL))));
            outP->green = A_u_short (   PosOutP->green  *   CalculateBox(refcon, xL, yL)   +(letP->Color.green * (1- CalculateBox(refcon, xL, yL))));
            outP->blue = A_u_short (    PosOutP->blue   *   CalculateBox(refcon, xL, yL)+ (letP->Color.blue * (1- CalculateBox(refcon, xL, yL))));
        }
    }

	return err;
}

static PF_Err 
PixelFunc16(	
	void 		*refcon,
	A_long 		xL,
	A_long 		yL,
	PF_Pixel16 *inP,
	PF_Pixel16 *outP)
{
	prerender_letP*	letP = reinterpret_cast<prerender_letP*>(refcon);
    PF_Err				err			= PF_Err_NONE;
    PF_InData			*in_data	= &(letP->in_data);
    PF_Fixed                new_xFi		= 0,
    new_yFi		= 0;
	

	if (letP){
        register PF_Pixel16		scratch16;
        
        scratch16.red		=	CONVERT8TO16(letP->Color.red);
        scratch16.green		=	CONVERT8TO16(letP->Color.green);
        scratch16.blue		=	CONVERT8TO16(letP->Color.blue);
        
        if (letP->PoTransparentB == TRUE)
        {
            outP->alpha = A_long(PF_MAX_CHAN16*(1-(CalculateBox(refcon, xL, yL))));
            outP->red =     A_long ( scratch16.red);
            outP->green = A_long (scratch16.green);
            outP->blue =A_long (scratch16.blue);
        }
        else
        {
            
            new_xFi = PF_Fixed((((A_long)xL << 16) + letP->x_offF )/letP->scaleFactorF);
            new_yFi = PF_Fixed((((A_long)yL << 16) + letP->y_offF)/letP->scaleFactorF);
            ERR(letP->in_data.utils->subpixel_sample16 (in_data->effect_ref,
                                                        new_xFi,
                                                        new_yFi,
                                                        &letP->samp_pb,
                                                        outP));
            
            outP->red = A_long ( outP->red   *   CalculateBox(refcon, xL, yL)+ (scratch16.red * (1- CalculateBox(refcon, xL, yL))));
            outP->green = A_long ( outP->green *   CalculateBox(refcon, xL, yL)+ (scratch16.green * (1- CalculateBox(refcon, xL, yL))));
            outP->blue = A_long (outP->blue   *   CalculateBox(refcon, xL, yL)+ (scratch16.blue * (1- CalculateBox(refcon, xL, yL))));
            outP->alpha	= inP->alpha;
        }
	}
	return err;
}

static PF_Err 
PixelFuncFloat(	
	void			*refcon,
	A_long			xL,
	A_long			yL,
	PF_PixelFloat	*inP,
	PF_PixelFloat	*outP)
{
    PF_Err				err			= PF_Err_NONE;
	prerender_letP*	letP = reinterpret_cast<prerender_letP*>(refcon);
    PF_Fixed			new_xFi 		= 0,
                        new_yFi 		= 0;
	
	if (letP){
        if (letP->PoTransparentB == TRUE)
        {
            outP->alpha = inP->alpha *(1-(CalculateBox(refcon, xL, yL)));
            outP->red   =   letP->Color32.red;
            outP->green =   letP->Color32.green;
            outP->blue  =   letP->Color32.blue;
        }
        else
        {
            if ((letP->x_offF !=0)&& (letP->y_offF !=0)&&(letP->scaleFactorF !=1)) // because it's very slower in 32 bits than in 16/8bits, we do a  special condition when transformation is not afected by user.
            {
            AEGP_SuiteHandler suites(letP->in_data.pica_basicP);
                new_xFi = PF_Fixed((((A_long)xL << 16) + letP->x_offF )/letP->scaleFactorF);
                new_yFi = PF_Fixed((((A_long)yL << 16) + letP->y_offF)/letP->scaleFactorF);
            
            
            ERR(suites.SamplingFloatSuite1()->nn_sample_float(letP->in_data.effect_ref,
                                                                    new_xFi,
                                                                    new_yFi, 
                                                                    &letP->samp_pb,
                                                                    outP));
                
                outP->red =   (outP->red   *   CalculateBox(refcon, xL, yL)+ (letP->Color32.red * (1- CalculateBox(refcon, xL, yL))));
                outP->green = (outP->green *   CalculateBox(refcon, xL, yL)+ (letP->Color32.green * (1- CalculateBox(refcon, xL, yL))));
                outP->blue =  (outP->blue   *   CalculateBox(refcon, xL, yL)+ (letP->Color32.blue * (1- CalculateBox(refcon, xL, yL))));
                outP->alpha	= inP->alpha;
            }
            else
            {
                outP->red =   (inP->red   *   CalculateBox(refcon, xL, yL)+ (letP->Color32.red * (1- CalculateBox(refcon, xL, yL))));
                outP->green = (inP->green *   CalculateBox(refcon, xL, yL)+ (letP->Color32.green * (1- CalculateBox(refcon, xL, yL))));
                outP->blue =  (inP->blue   *   CalculateBox(refcon, xL, yL)+ (letP->Color32.blue * (1- CalculateBox(refcon, xL, yL))));
                outP->alpha	= inP->alpha;
                
            }
            
        }
	}
	return err;
}
//RENDER FUNCTIONS FOR PREMIERE
/*
static PF_Err
PixelFuncVUYA_32f(
                  void			*refcon,
                  A_long			xL,
                  A_long			yL,
                  PF_PixelFloat	*inP,
                  PF_PixelFloat	*outP)
{
    PF_Err			err = PF_Err_NONE;
    
    
    PF_Pixel_VUYA_32f *inVUYA_32f, *outVUYA_32f;
    
    inVUYA_32f = reinterpret_cast<PF_Pixel_VUYA_32f*>(inP);
    outVUYA_32f = reinterpret_cast<PF_Pixel_VUYA_32f*>(outP);
    
    prerender_letP*	letP = reinterpret_cast<prerender_letP*>(refcon);
    PF_Pixel_VUYA_32f ColorYuv;

    
    if (letP) {

        
        ColorYuv.luma = PF_FpShort ((0.257 * letP->Color.red) + (0.504 * letP->Color.green) + (0.098 * letP->Color.blue))+16/219;
        ColorYuv.Pb =   PF_FpShort (-(0.148 * letP->Color.red) - (0.291 * letP->Color.green) + (0.439 * letP->Color.blue) )+128/224;
        ColorYuv.Pr =   PF_FpShort ((0.439 * letP->Color.red) - (0.368 * letP->Color.green) - (0.071 * letP->Color.blue) ) +128/224;
        
        if (letP->PoTransparentB == TRUE)
        {
            outVUYA_32f->alpha   =(1 - (CalculateBox(refcon, xL, yL)));
            outVUYA_32f->luma =   ColorYuv.luma;
            outVUYA_32f->Pb =     ColorYuv.Pb;
            outVUYA_32f->Pr =     ColorYuv.Pr;
        }
        else
        {
        
            outVUYA_32f->alpha =   inVUYA_32f->alpha;
           
            outVUYA_32f->luma = inVUYA_32f->luma   *   CalculateBox(refcon, xL, yL)+ (ColorYuv.luma*(1- CalculateBox(refcon, xL, yL)));
            outVUYA_32f->Pb = inVUYA_32f->Pb *      CalculateBox(refcon, xL, yL)+ (ColorYuv.Pb*(1- CalculateBox(refcon, xL, yL)));
            outVUYA_32f->Pr = inVUYA_32f->Pr *       CalculateBox(refcon, xL, yL) + (ColorYuv.Pr*(1- CalculateBox(refcon, xL, yL)));

        }
    }
    return err;
}*/

static PF_Err
PixelFuncBGRA_32f(
                     void			*refcon,
                     A_long			xL,
                     A_long			yL,
                     PF_PixelFloat	*inP,
                     PF_PixelFloat	*outP)
{
    PF_Err			err = PF_Err_NONE;

    
    PF_Pixel_BGRA_32f *inBGRA_32fP, *outBGRA_32fP;
    inBGRA_32fP = reinterpret_cast<PF_Pixel_BGRA_32f*>(inP);
    outBGRA_32fP = reinterpret_cast<PF_Pixel_BGRA_32f*>(outP);
    prerender_letP*	letP = reinterpret_cast<prerender_letP*>(refcon);
    
    if (letP) {
        if (letP->PoTransparentB == TRUE)
        {
            if (letP->Color.blue + letP->Color.green +letP->Color.red ==0)
            {
                outBGRA_32fP->blue =    0;
                outBGRA_32fP->green =   0;
                outBGRA_32fP->red =     0;
                outBGRA_32fP->alpha =   (1-(CalculateBox(refcon, xL, yL)));
                
                
            }
            else
            {
            outBGRA_32fP->blue =    letP->Color.blue;
            outBGRA_32fP->green =    letP->Color.green;
            outBGRA_32fP->red =  letP->Color.blue;
            outBGRA_32fP->alpha =   (1-(CalculateBox(refcon, xL, yL)));
            }
        }
        else
        {
            outBGRA_32fP->alpha = 1;
            outBGRA_32fP->red =     inBGRA_32fP->red    *   CalculateBox(refcon, xL, yL)+ (letP->Color.red    * (1- CalculateBox(refcon, xL, yL)));
            outBGRA_32fP->green =   inBGRA_32fP->green  *   CalculateBox(refcon, xL, yL)+ (letP->Color.green  * (1- CalculateBox(refcon, xL, yL)));
            outBGRA_32fP->blue =    inBGRA_32fP->blue   *   CalculateBox(refcon, xL, yL)+ (letP->Color.blue   * (1- CalculateBox(refcon, xL, yL)));
        }
    }
    return err;
}
static PF_Err
PixelFuncVUYA_16u(
                            void		*refcon,
                            A_long		xL,
                            A_long		yL,
                            PF_Pixel16	*inP,
                            PF_Pixel16	*outP)
{
    PF_Err			err = PF_Err_NONE;
    
    PF_Pixel_VUYA_16u *inVUYA_16uP, *outVUYA_16uP;
    
    inVUYA_16uP = reinterpret_cast<PF_Pixel_VUYA_16u*>(inP);
    outVUYA_16uP = reinterpret_cast<PF_Pixel_VUYA_16u*>(outP);
    
    prerender_letP*	letP = reinterpret_cast<prerender_letP*>(refcon);
    PF_Pixel_VUYA_8u ColorYuv;
    PF_Pixel_VUYA_16u ColorYuv16;
    
    if (letP) {
        
        
        ColorYuv.luma = A_u_char(  (0.257 * letP->Color.red) + (0.504 * letP->Color.green) + (0.098 * letP->Color.blue) + 16);
        ColorYuv.Pb = A_u_char(-(0.148 * letP->Color.red) - (0.291 * letP->Color.green) + (0.439 * letP->Color.blue) + 128);
        ColorYuv.Pr = A_u_char((0.439 * letP->Color.red) - (0.368 * letP->Color.green) - (0.071 * letP->Color.blue) + 128);
        
        ColorYuv16.luma = ColorYuv.luma* PF_MAX_CHAN16/PF_MAX_CHAN8;
        ColorYuv16.Pb =   ColorYuv.Pb* PF_MAX_CHAN16/PF_MAX_CHAN8;
        ColorYuv16.Pr =   ColorYuv.Pr* PF_MAX_CHAN16/PF_MAX_CHAN8;
        
        if (letP->PoTransparentB == TRUE)
        {
            outVUYA_16uP->alpha   = A_u_short(PF_MAX_CHAN16*(1 - (CalculateBox(refcon, xL, yL))));
            outVUYA_16uP->luma =   ColorYuv16.luma;
            outVUYA_16uP->Pb =     ColorYuv16.Pb;
            outVUYA_16uP->Pr =     ColorYuv16.Pr;
        }
        else
        {
            outVUYA_16uP->alpha =    PF_MAX_CHAN8;
            outVUYA_16uP->luma = A_u_short(inVUYA_16uP->luma    *   CalculateBox(refcon, xL, yL) + (ColorYuv16.luma*(1- CalculateBox(refcon, xL, yL))));
            outVUYA_16uP->Pb = A_u_short  (inVUYA_16uP->Pb   *   CalculateBox(refcon, xL, yL)+ ColorYuv16.Pb*(1- CalculateBox(refcon, xL, yL)));
            outVUYA_16uP->Pr = A_u_short (inVUYA_16uP->Pr  *   CalculateBox(refcon, xL, yL) + ColorYuv16.Pr*(1- CalculateBox(refcon, xL, yL)));
        }
    }
    return err;
}

static PF_Err
PixelFuncBGRA_8u(
                 void		*refcon,
                 A_long		xL,
                 A_long		yL,
                 PF_Pixel8	*inP,
                 PF_Pixel8	*outP)
{
    PF_Err			err = PF_Err_NONE;
    PF_Pixel_BGRA_8u *inBGRA_8uP, *outBGRA_8uP;
    inBGRA_8uP = reinterpret_cast<PF_Pixel_BGRA_8u*>(inP);
    outBGRA_8uP = reinterpret_cast<PF_Pixel_BGRA_8u*>(outP);
    
    prerender_letP*	letP = reinterpret_cast<prerender_letP*>(refcon);
    
    if (letP) {
        if (letP->PoTransparentB == TRUE)
        {
            outBGRA_8uP->alpha = A_u_short ( PF_MAX_CHAN8*(1-(CalculateBox(refcon, xL, yL))));
            outBGRA_8uP->red = letP->Color.red;
            outBGRA_8uP->green = letP->Color.green;
            outBGRA_8uP->blue =letP->Color.blue;
        }
        else
        {
            outBGRA_8uP->alpha =    PF_MAX_CHAN8;
            outBGRA_8uP->red = A_u_short(inBGRA_8uP->red    *   CalculateBox(refcon, xL, yL)+ (letP->Color.red * (1- CalculateBox(refcon, xL, yL))));
            outBGRA_8uP->green = A_u_short(inBGRA_8uP->green  *   CalculateBox(refcon, xL, yL)+ (letP->Color.green * (1- CalculateBox(refcon, xL, yL))));
            outBGRA_8uP->blue = A_u_short(inBGRA_8uP->blue   *   CalculateBox(refcon, xL, yL)+ (letP->Color.blue * (1- CalculateBox(refcon, xL, yL))));
        }
    }
    return err;
}


static PF_Err
PixelFuncVUYA_8u(
                    void		*refcon,
                    A_long		xL,
                    A_long		yL,
                    PF_Pixel8	*inP,
                    PF_Pixel8	*outP)
{
    PF_Err			err = PF_Err_NONE;
    
    PF_Pixel_VUYA_8u *inVUYA_8uP, *outVUYA_8uP;
   
    inVUYA_8uP = reinterpret_cast<PF_Pixel_VUYA_8u*>(inP);
    outVUYA_8uP = reinterpret_cast<PF_Pixel_VUYA_8u*>(outP);
    
    prerender_letP*	letP = reinterpret_cast<prerender_letP*>(refcon);
    PF_Pixel_VUYA_8u ColorYuv;
    
    if (letP) {
        

        ColorYuv.luma = A_u_char(  (0.257 * letP->Color.red) + (0.504 * letP->Color.green) + (0.098 * letP->Color.blue) + 16);
        ColorYuv.Pb = A_u_char(-(0.148 * letP->Color.red) - (0.291 * letP->Color.green) + (0.439 * letP->Color.blue) + 128);
        ColorYuv.Pr = A_u_char((0.439 * letP->Color.red) - (0.368 * letP->Color.green) - (0.071 * letP->Color.blue) + 128);
        

        if (letP->PoTransparentB == TRUE)
        {
            outVUYA_8uP->alpha   = A_u_short(PF_MAX_CHAN8*(1 - (CalculateBox(refcon, xL, yL))));
            outVUYA_8uP->luma =   ColorYuv.luma;
            outVUYA_8uP->Pb =     ColorYuv.Pb;
            outVUYA_8uP->Pr =     ColorYuv.Pr;
        }
        else
        {
            outVUYA_8uP->alpha =    PF_MAX_CHAN8;
            outVUYA_8uP->luma = A_u_char(inVUYA_8uP->luma    *   CalculateBox(refcon, xL, yL) + (ColorYuv.luma*(1- CalculateBox(refcon, xL, yL))));
            outVUYA_8uP->Pb = A_u_char  (inVUYA_8uP->Pb   *   CalculateBox(refcon, xL, yL)+ ColorYuv.Pb*(1- CalculateBox(refcon, xL, yL)));
            outVUYA_8uP->Pr = A_u_char  (inVUYA_8uP->Pr  *   CalculateBox(refcon, xL, yL) + ColorYuv.Pr*(1- CalculateBox(refcon, xL, yL)));
        }
    }
    return err;
}


static PF_Err
IterateDeep(
             PF_InData			*in_data,
             long				progress_base,
             long				progress_final,
             PF_EffectWorld		*src,
             void				*refcon,
             PF_Err(*pix_fn)(void *refcon, A_long x, A_long y, PF_Pixel16 *in, PF_Pixel16 *out),
             PF_EffectWorld		*dst)
{
    PF_Err	err = PF_Err_NONE;
    char	*localSrc, *localDst;
    localSrc = reinterpret_cast<char*>(src->data);
    localDst = reinterpret_cast<char*>(dst->data);
    
    for (int y = progress_base; y < progress_final; y++)
    {
        for (int x = 0; x < in_data->width; x++)
        {
            pix_fn(refcon,
                   static_cast<A_long> (x),
                   static_cast<A_long> (y),
                   reinterpret_cast<PF_Pixel16*>(localSrc),
                   reinterpret_cast<PF_Pixel16*>(localDst));
            localSrc += 16;
            localDst += 16;
        }
        localSrc += (src->rowbytes - in_data->width * 16);
        localDst += (dst->rowbytes - in_data->width * 16);
    }
    
    return err;
}



//function for iterating float in premiere (because it doesn't support the suite)
static PF_Err
IterateFloat(
             PF_InData			*in_data,
             long				progress_base,
             long				progress_final,
             PF_EffectWorld		*src,
             void				*refcon,
             PF_Err(*pix_fn)(void *refcon, A_long x, A_long y, PF_PixelFloat *in, PF_PixelFloat *out),
             PF_EffectWorld		*dst)
{
    PF_Err	err = PF_Err_NONE;
    char	*localSrc, *localDst;
    localSrc = reinterpret_cast<char*>(src->data);
    localDst = reinterpret_cast<char*>(dst->data);
    
    for (int y = progress_base; y < progress_final; y++)
    {
        for (int x = 0; x < in_data->width; x++)
        {
            pix_fn(refcon,
                   static_cast<A_long> (x),
                   static_cast<A_long> (y),
                   reinterpret_cast<PF_PixelFloat*>(localSrc),
                   reinterpret_cast<PF_PixelFloat*>(localDst));
            localSrc += 16;
            localDst += 16;
        }
        localSrc += (src->rowbytes - in_data->width * 16);
        localDst += (dst->rowbytes - in_data->width * 16);
    }
    
    return err;
}



static PF_Err
MakeParamCopy(
              PF_InData     *in_data,
              PF_ParamDef *actual[],	/* >> */
              PF_ParamDef copy[])		/* << */
{
    for (A_short iS = 0; iS < LETB_NUM_PARAMS; ++iS)	{
        AEFX_CLR_STRUCT(copy[iS]);	// clean params are important!
    }
    copy[LETB_INPUT]			= *actual[LETB_INPUT];
    copy[LETB_MODE]             = *actual[LETB_MODE];
    copy[LETB_PRESET]			= *actual[LETB_PRESET];
    copy[LETB_SLIDER]			= *actual[LETB_SLIDER];
    copy[LETB_TRSP]             = *actual[LETB_TRSP];
    copy[LETB_COLOR]            = *actual[LETB_COLOR];
    copy[LETB_GR1]              = *actual[LETB_GR1];
    copy[LETB_CENTER]           = *actual[LETB_CENTER];
    copy[LETB_RESIZE]           = *actual[LETB_RESIZE];
    if (in_data->appl_id != 'PrMr')
    {
        copy[LETB_BTN_ANALYS]         = *actual[LETB_BTN_ANALYS];
        copy[LETB_GR2]              = *actual[LETB_GR2];
        copy[LETB_SIZE_SOURCE]       = *actual[LETB_SIZE_SOURCE];
        
    }
  
    return PF_Err_NONE;
    
}



static PF_Err
UserChangedParam(
                 PF_InData						*in_data,
                 PF_OutData						*out_data,
                 PF_ParamDef					*params[],
                 PF_LayerDef					*outputP,
                 const PF_UserChangedParamExtra	*which_hitP)
{
    PF_Err				err					= PF_Err_NONE,
    err2					= PF_Err_NONE;
     my_global_dataP		globP				= reinterpret_cast<my_global_dataP>(DH(out_data->global_data));
    
    if (which_hitP->param_index == LETB_MODE) //when switching from preset to advanced mode : the ratio value is kept.
    {
        
        PF_FpLong tempRatio;
        GetPresetRatioValue(params[LETB_PRESET]->u.pd.value, &tempRatio);
        params[LETB_SLIDER]->u.fs_d.value = tempRatio;
        params[LETB_SLIDER]->uu.change_flags = PF_ChangeFlag_CHANGED_VALUE;
        AEGP_SuiteHandler		suites(in_data->pica_basicP);
        ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                        LETB_SLIDER,
                                                        params[LETB_SLIDER]));
        
        
    }
    
    if (which_hitP->param_index == LETB_BTN_ANALYS)
    {
        // If checkbox is checked, change slider value to the layer ratio.
        AEGP_SuiteHandler		suites(in_data->pica_basicP);
        
        
            params[LETB_MODE]->u.pd.value = MODE_ADVANCED;
            params[LETB_MODE]->uu.change_flags = PF_ChangeFlag_CHANGED_VALUE;
            ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                           LETB_MODE,
                                                           params[LETB_MODE]));
            
            PF_FpLong scanlayerRatioF;
            if (in_data->appl_id != 'PrMr')
            {
                
				PF_EffectWorld *scanWorldP;
				PF_PixelFormat detectFormat = PF_PixelFormat_INVALID;
               
				if (!err) 
				{
                    //GET COLOR LIMIT DEFINED BY USER
                    PF_ParamDef param_color_analys;
                    AEFX_CLR_STRUCT(param_color_analys);
                    ERR(PF_CHECKOUT_PARAM(	in_data,
                                          LETB_COLOR_ANALYS,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &param_color_analys));
                    
                    PF_PixelFloat analysColor;
                    ERR(suites.ColorParamSuite1()->PF_GetFloatingPointColorFromColorDef(in_data->effect_ref, &param_color_analys, &analysColor));
                    
                    // temporary problem with the offitial getpixelformat in windows so cheat with this conditions group.
                    AEGP_ProjectH projH;
                    AEGP_ProjBitDepth bit_depthP;
                    ERR(suites.ProjSuite5()->AEGP_GetProjectByIndex(0, &projH));
                    ERR(suites.ProjSuite6()->AEGP_GetProjectBitDepth(projH, &bit_depthP));
                    
                    AEFX_CLR_STRUCT(detectFormat);
                    if (bit_depthP == AEGP_ProjBitDepth_8) //the same for the two worlds
                    {
                        detectFormat = PF_PixelFormat_ARGB32;
                    }
                    else if (bit_depthP == AEGP_ProjBitDepth_16)
                    {
                        detectFormat = PF_PixelFormat_ARGB64;
                    }
                    else
                    {
                        detectFormat = PF_PixelFormat_ARGB128;
                    }
                    
                    // METHOD CURRENT FRAME OR WHOLE LAYER
                    PF_ParamDef method_analys;
                    AEFX_CLR_STRUCT(method_analys);
                    ERR(PF_CHECKOUT_PARAM(	in_data,
                                          LETB_TIME_ANALYS,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &method_analys));
                    PF_ParamDef paramInput;
                    if (method_analys.u.pd.value ==TIME_LAYER)
                    {
                        AEGP_LayerH curLayerH, targetLayerH;
                        A_Time    durationPT, offsetPT, inPPT;
                        AEGP_EffectRefH  effectPH;
                        AEGP_StreamRefH    streamPH;
                        AEGP_StreamValue2   valueP;
                        AEGP_CompH     compPH;
                        A_FpLong        fpsPF;
                        const A_Time        analystime = {0,100};
                        
                        //return current layer and current layer Comp
                        ERR(suites.PFInterfaceSuite1()->AEGP_GetEffectLayer(in_data->effect_ref, &curLayerH));
                        ERR(suites.LayerSuite8()->AEGP_GetLayerParentComp(curLayerH, &compPH));
                        ERR(suites.CompSuite11()->AEGP_GetCompFramerate(compPH, &fpsPF));
                        

                        ERR(suites.PFInterfaceSuite1()->AEGP_GetNewEffectForEffect(globP->my_id,in_data->effect_ref,&effectPH));
                        ERR(suites.StreamSuite4()->AEGP_GetNewEffectStreamByIndex(globP->my_id, effectPH, LETB_LAYER_ANALYS, &streamPH));
                        ERR(suites.StreamSuite4()->AEGP_GetNewStreamValue(globP->my_id,streamPH, AEGP_LTimeMode_LayerTime, &analystime , NULL, &valueP));

                        ERR(suites.LayerSuite8()->AEGP_GetLayerFromLayerID(compPH, valueP.val.layer_id, &targetLayerH)); //return the targeted layer
                        ERR(suites.LayerSuite8()->AEGP_GetLayerOffset(targetLayerH, &offsetPT));
                        ERR(suites.LayerSuite8()->AEGP_GetLayerInPoint(targetLayerH,AEGP_LTimeMode_CompTime, &inPPT));
                        ERR(suites.LayerSuite8()->AEGP_GetLayerDuration(targetLayerH,AEGP_LTimeMode_CompTime, &durationPT)); // return the duration of the targeted layer


                        scanlayerRatioF =0;
                        A_long  durationFramesA, inPPTA, totalfA;

                        inPPTA = A_long (PF_FpLong(inPPT.value/inPPT.scale)*fpsPF);
                        durationFramesA= A_long (PF_FpLong(durationPT.value/durationPT.scale)*fpsPF);
                        totalfA =  inPPTA +durationFramesA;
                        
                        for (A_long i = inPPTA ; i< totalfA ; i++)
                        {
                           if ((i) && (err = (PF_PROGRESS(in_data, i,totalfA))))
                           {
                               return err;
                           }
                            A_Time compTime;
                            A_Boolean activeB;
                            compTime.value = i*inPPT.scale/fpsPF;
                            compTime.scale = inPPT.scale;
                            

                            ERR(suites.LayerSuite8()->AEGP_IsVideoActive (targetLayerH,AEGP_LTimeMode_CompTime, & compTime, &activeB));
                            
                            if (activeB)
                                {
                                    AEFX_CLR_STRUCT(paramInput);
                                    AEFX_CLR_STRUCT(scanWorldP);
                                    
                                    ERR(PF_CHECKOUT_PARAM(in_data,
                                                          LETB_LAYER_ANALYS,
                                                          (i*in_data->time_step),
                                                          in_data->time_step,
                                                          durationPT.scale,
                                                          &paramInput));
                                    
                                    scanWorldP = &paramInput.u.ld;
                                    
                                    PF_FpLong tempLayerRatio;
                                    AEFX_CLR_STRUCT(tempLayerRatio);
                                    GetRatioFromWorld (in_data, scanWorldP, detectFormat, &tempLayerRatio,analysColor);
                                    if (tempLayerRatio >scanlayerRatioF)
                                    {
                                        scanlayerRatioF = tempLayerRatio;
                                    }
                                    ERR2(PF_CHECKIN_PARAM(in_data, &paramInput));
                                }
                        }
                        ERR(suites.EffectSuite3()->AEGP_DisposeEffect(effectPH));
                        ERR(suites.StreamSuite4()->AEGP_DisposeStream(streamPH));
                    }
                    else
                    {
                        
                        AEFX_CLR_STRUCT(paramInput);
                        AEFX_CLR_STRUCT(scanWorldP);
                        ERR(PF_CHECKOUT_PARAM(	in_data,
                                              LETB_LAYER_ANALYS,
                                              in_data->current_time,
                                              in_data->time_step,
                                              in_data->time_scale,
                                              &paramInput));
                        
                        scanWorldP = &paramInput.u.ld;
                        GetRatioFromWorld (in_data, scanWorldP, detectFormat,&scanlayerRatioF,analysColor);
                        ERR2(PF_CHECKIN_PARAM(in_data, &paramInput));
                        
                    }
                     ERR2(PF_CHECKIN_PARAM(in_data, &param_color_analys));
                }
                
            }

            params[LETB_SLIDER]->u.fs_d.value= scanlayerRatioF;
            params[LETB_SLIDER]->uu.change_flags = PF_ChangeFlag_CHANGED_VALUE;
            
            params[LETB_BTN_ANALYS]->u.bd.value = FALSE;
           
            ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                            LETB_BTN_ANALYS,
                                                            params[LETB_BTN_ANALYS]));
        }

    return err;
}

static PF_Err
UpdateParameterUI(
                  PF_InData			*in_data,
                  PF_OutData			*out_data,
                  PF_ParamDef			*params[],
                  PF_LayerDef			*outputP)
{
    PF_Err				err					= PF_Err_NONE,
    err2				= PF_Err_NONE;
    my_global_dataP		globP				= reinterpret_cast<my_global_dataP>(DH(out_data->global_data));
    AEGP_StreamRefH     preset_streamH		= NULL,
    slider_streamH		= NULL,
    trsp_streamH        = NULL,
    color_streamH       = NULL,
    topic_streamH       = NULL,
    center_streamH      = NULL,
    resize_streamH      = NULL,
    topic2_streamH      = NULL,
    sizeSource_streamH  = NULL;


    
    PF_ParamType		param_type;
    PF_ParamDefUnion	param_union;
    
    A_Boolean			hide_oneB       = FALSE,
                        hide_twoB		= FALSE,
                        hide_threeB     = FALSE;
    
    AEGP_EffectRefH			meH				= NULL;
    AEGP_SuiteHandler		suites(in_data->pica_basicP);
    
    
    PF_ParamDef		param_copy[LETB_NUM_PARAMS];
    ERR(MakeParamCopy(in_data, params, param_copy));
    

    
    if (in_data->appl_id != 'PrMr') {
        
        
        if (params[LETB_MODE]->u.pd.value == MODE_BASIC)
        {
            hide_oneB        = FALSE;
            hide_twoB		 = TRUE;
        }
        else
        {
            hide_oneB        = TRUE;
            hide_twoB		 = FALSE;
        }
        if (params[LETB_TRSP]->u.bd.value == TRUE)
        {
            hide_threeB =TRUE;
        }
        else
        {
            hide_threeB = FALSE;
        }
        
        // Twirl open the slider param
        param_copy[LETB_SLIDER].param_type	=	PF_Param_FLOAT_SLIDER;
        param_copy[LETB_SLIDER].flags		&= ~PF_ParamFlag_COLLAPSE_TWIRLY;
        
        ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                        LETB_SLIDER,
                                                        &param_copy[LETB_SLIDER]));
        
        // Changing visibility of params in AE is handled through stream suites
        
        ERR(suites.PFInterfaceSuite1()->AEGP_GetNewEffectForEffect(globP->my_id, in_data->effect_ref, &meH));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, LETB_PRESET, 	&preset_streamH));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, LETB_SLIDER,	&slider_streamH));
        
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, LETB_GR1, &topic_streamH ));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, LETB_CENTER, &center_streamH));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, LETB_RESIZE,&resize_streamH));
        

        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, LETB_GR2, &topic2_streamH ));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, LETB_SIZE_SOURCE, &sizeSource_streamH));

        // Toggle visibility of parameters
        //HIDE ONE
        ERR(suites.DynamicStreamSuite2()->AEGP_SetDynamicStreamFlag(preset_streamH, 	AEGP_DynStreamFlag_HIDDEN, FALSE, hide_oneB));
        //HIDE TWO
        ERR(suites.DynamicStreamSuite2()->AEGP_SetDynamicStreamFlag(slider_streamH, 	AEGP_DynStreamFlag_HIDDEN, FALSE, hide_twoB));
        //HIDE THREE
        ERR(suites.DynamicStreamSuite2()->AEGP_SetDynamicStreamFlag(topic_streamH, 	AEGP_DynStreamFlag_HIDDEN, FALSE, hide_threeB));
        ERR(suites.DynamicStreamSuite2()->AEGP_SetDynamicStreamFlag(center_streamH, AEGP_DynStreamFlag_HIDDEN, FALSE, hide_threeB));
        ERR(suites.DynamicStreamSuite2()->AEGP_SetDynamicStreamFlag(resize_streamH, AEGP_DynStreamFlag_HIDDEN, FALSE, hide_threeB));
        
        // ALWAYS HIDE AEGP_COMUNICATIONS. TADA MAGIC

        
        // Change popup menu items
        ERR(suites.EffectSuite3()->AEGP_GetEffectParamUnionByIndex(globP->my_id, meH, LETB_PRESET_DISK_ID, &param_type, &param_union));
        if (param_type == PF_Param_POPUP)
        {
            strcpy((char*)param_union.pd.u.namesptr, GetStringPtr(StrID_PresetChoicesChanged));
        }
        
        if (meH){
            ERR2(suites.EffectSuite2()->AEGP_DisposeEffect(meH));
        }
        if (preset_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(preset_streamH));
        }
        if (slider_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(slider_streamH));
        }
        if (trsp_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(trsp_streamH));
        }
        if (color_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(color_streamH));
        }
        if (topic_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(topic_streamH));
        }
        if (center_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(center_streamH));
        }
        if (resize_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(resize_streamH));
        }
        if (topic2_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(topic2_streamH));
        }
        if (sizeSource_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(sizeSource_streamH));
        }

        if (!err){
            out_data->out_flags |= PF_OutFlag_FORCE_RERENDER;
        }
        
        

        
    } else { // Premiere Pro doesn't support the stream suites, but uses a UI flag instead
        
        //	If our global data is present...
        if (!err && globP) {
            // Test all parameters except layers for changes
            
            //	Hide on preset/costum.
            if (!err && (MODE_BASIC == params[LETB_MODE]->u.pd.value))
            {
                if (!err && (param_copy[LETB_PRESET].ui_flags & PF_PUI_INVISIBLE)) {
                    
                    param_copy[LETB_PRESET].ui_flags &= ~PF_PUI_INVISIBLE;
                    
                    ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                    LETB_PRESET,
                                                                    &param_copy[LETB_PRESET]));
                }
                
                if (!err) {
                    param_copy[LETB_SLIDER].ui_flags |= PF_PUI_INVISIBLE;
                    ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                    LETB_SLIDER,
                                                                    &param_copy[LETB_SLIDER]));
                }
                

            }
            else
            {
                
                
                if (!err) {
                    param_copy[LETB_PRESET].ui_flags |=	PF_PUI_INVISIBLE;
                    ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                    LETB_PRESET,
                                                                    &param_copy[LETB_PRESET]));
                }
                
                
                if (!err && (param_copy[LETB_SLIDER].ui_flags & PF_PUI_INVISIBLE)) {
                    // Note: As of Premiere Pro CS5.5, this is currently not honored
                    param_copy[LETB_SLIDER].flags	&= ~PF_ParamFlag_COLLAPSE_TWIRLY;
                    param_copy[LETB_SLIDER].ui_flags &= ~PF_PUI_INVISIBLE;
                    
                    ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref, 
                                                                    LETB_SLIDER,
                                                                    &param_copy[LETB_SLIDER]));
                }

            }


          

            // Hide on transparent/not transparent
            if(!err && (params[LETB_TRSP]->u.bd.value == TRUE))
            {
                if (!err)
                {
                    param_copy[LETB_GR1].ui_flags |=	PF_PUI_INVISIBLE;
                    ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                    LETB_GR1,
                                                                    &param_copy[LETB_GR1]));
                }
                if (!err)
                {
                    param_copy[LETB_CENTER].ui_flags |=	PF_PUI_INVISIBLE;
                    ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                    LETB_CENTER,
                                                                    &param_copy[LETB_CENTER]));
                }
                if (!err)
                {
                    param_copy[LETB_RESIZE].ui_flags |=	PF_PUI_INVISIBLE;
                    ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                    LETB_RESIZE,
                                                                    &param_copy[LETB_RESIZE]));
                }

                
                    
            }
            else
            {
                if (!err && (param_copy[LETB_GR1].ui_flags & PF_PUI_INVISIBLE))
                {
                param_copy[LETB_GR1].ui_flags &= ~PF_PUI_INVISIBLE;
                ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                        LETB_GR1,
                                                                        &param_copy[LETB_GR1]));
                }
                if (!err && (param_copy[LETB_CENTER].ui_flags & PF_PUI_INVISIBLE))
                {
                    param_copy[LETB_CENTER].ui_flags &= ~PF_PUI_INVISIBLE;
                            
                    ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                    LETB_CENTER,
                                                                    &param_copy[LETB_CENTER]));
                }
                if (!err && (param_copy[LETB_RESIZE].ui_flags & PF_PUI_INVISIBLE))
                {
                    param_copy[LETB_RESIZE].ui_flags &= ~PF_PUI_INVISIBLE;
                                
                    ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                    LETB_RESIZE,
                                                                    &param_copy[LETB_RESIZE]));
                }
                    
            }
            
            if (err)  // <--- Note: not !err, err!
            {
                strcpy(out_data->return_msg, STR(StrID_GeneralError));
                out_data->out_flags = PF_OutFlag_DISPLAY_ERROR_MESSAGE;
            }
            else
            {
                globP->initializedB = TRUE;	// set our cheesy first time indicator
            }

        } 
        
        if (!err) {
            globP->initializedB	=	TRUE;
        }
        out_data->out_flags |= 	PF_OutFlag_REFRESH_UI 		|
								PF_OutFlag_FORCE_RERENDER;
    }
    
    return err;
}

static PF_Err
GlobalSetdown(
              PF_InData		*in_data)
{
    AEGP_SuiteHandler	suites(in_data->pica_basicP);
    
    if (in_data->global_data) {
        suites.HandleSuite1()->host_dispose_handle(in_data->global_data);
    }
    
    return PF_Err_NONE;
}




static PF_Err	
Render(	PF_InData		*in_data,
		PF_OutData		*out_data,
		PF_ParamDef		*params[],
		PF_LayerDef		*output)
{
	PF_Err	err	= PF_Err_NONE;

    AEGP_SuiteHandler	suites(in_data->pica_basicP);
    PF_EffectWorld  *inputP  = &params[LETB_INPUT]->u.ld;
    

    prerender_letP		letP;

    if (in_data->appl_id == 'PrMr') {
        
     
        
        letP.in_data = *in_data;
        letP.samp_pb.src = inputP;
        
        letP.InputWidthF  = in_data->width;
        letP.InputHeightF  = in_data->height;
        letP.PixRatioNumF = in_data->pixel_aspect_ratio.num;
        letP.PixRatioDenF = in_data->pixel_aspect_ratio.den;
        
        PF_FpLong scale_x = in_data->downsample_x.num/ (float)in_data->downsample_x.den,
        scale_y = in_data->downsample_y.num/ (float)in_data->downsample_y.den;
        
        letP.InputWidthF  *= scale_x;
        letP.InputHeightF  *= scale_y;
        
        letP.compoffxF =0;
        letP.compoffyF =0;
        
        //Those param are used in AE smart render not in Prems
        letP.letoffxF = 0;
        letP.letoffyF = 0;
        letP.layerWidthF =   letP.InputWidthF;
        letP.layerHeightF =  letP.InputHeightF;
        
        
            letP.layerRatioF = (((double)in_data->width) *  letP.PixRatioNumF) / ((double)in_data->height*letP.PixRatioDenF); //ratio input from layer
            
            
            letP.Color = params[LETB_COLOR]->u.cd.value;
            letP.PoTransparentB = params[LETB_TRSP]->u.bd.value;
            

        if (MODE_BASIC == params[LETB_MODE]->u.pd.value){
            GetPresetRatioValue( params[LETB_PRESET]->u.pd.value, &letP.userRatioF);
        } else {
            letP.userRatioF = params[LETB_SLIDER]->u.fs_d.value;
        }
        
     
        
        // Get the Premiere pixel format suite
        AEFX_SuiteScoper<PF_PixelFormatSuite1> pixelFormatSuite =
        AEFX_SuiteScoper<PF_PixelFormatSuite1>(in_data,
                                               kPFPixelFormatSuite,
                                               kPFPixelFormatSuiteVersion1,
                                               out_data);
        
        PrPixelFormat destinationPixelFormat = PrPixelFormat_BGRA_4444_8u;
        
        pixelFormatSuite->GetPixelFormat(output, &destinationPixelFormat);
        
        AEFX_SuiteScoper<PF_Iterate8Suite1> iterate8Suite =
        AEFX_SuiteScoper<PF_Iterate8Suite1>(in_data,
                                            kPFIterate8Suite,
                                            kPFIterate8SuiteVersion1,
                                            out_data);
        
        
        //TRANSFORM POSITION &&SCALE WORLD
        PF_FpLong scaleFactor;
        scaleFactor = params[LETB_RESIZE]->u.fs_d.value/100;
        
        PF_LayerDef		 posOutput;
        AEFX_CLR_STRUCT(posOutput);
        posOutput = *output;
        
        PF_Pixel fill_trsp = {0,0,0,0};
        ERR(PF_FILL(&fill_trsp, &posOutput.extent_hint, &posOutput));
        PF_CompositeMode composite_mode;
        AEFX_CLR_STRUCT(composite_mode);
        PF_FloatMatrix float_matrix;
        AEFX_CLR_STRUCT(float_matrix.mat);
        float_matrix.mat[2][2]  = 1;//identity
        float_matrix.mat[2][0] =  FIX2INT(params[LETB_CENTER]->u.td.x_value) - 0.5* in_data->width* scaleFactor; // This is the x translation
        float_matrix.mat[2][1] =  FIX2INT(params[LETB_CENTER]->u.td.y_value) - 0.5* in_data->height* scaleFactor; // This is the y translation
        float_matrix.mat[0][0] =  scaleFactor; //scale matrix
        float_matrix.mat[1][1] =  scaleFactor; //scale matrix
        
        if (destinationPixelFormat == PrPixelFormat_BGRA_4444_8u ||destinationPixelFormat ==PrPixelFormat_VUYA_4444_8u)
        {
            AEFX_CLR_STRUCT(composite_mode);
            composite_mode.opacity = PF_MAX_CHAN8;
            composite_mode.xfer = PF_Xfer_COPY;
            
            
            ERR(in_data->utils->transform_world(in_data->effect_ref,
                                                in_data->quality,
                                                PF_MF_Alpha_STRAIGHT,
                                                in_data->field,
                                                inputP,
                                                &composite_mode,
                                                NULL,
                                                &float_matrix,
                                                1,
                                                TRUE,
                                                &posOutput.extent_hint,
                                                &posOutput));
        }
        else if (destinationPixelFormat == PrPixelFormat_BGRA_4444_32f)
        {
            AEFX_CLR_STRUCT(composite_mode);
            composite_mode.opacity = 1;
            composite_mode.xfer = PF_Xfer_COPY;
            
            
            ERR(in_data->utils->transform_world(in_data->effect_ref,
                                                in_data->quality,
                                                PF_MF_Alpha_STRAIGHT,
                                                in_data->field,
                                                inputP,
                                                &composite_mode,
                                                NULL,
                                                &float_matrix,
                                                1,
                                                TRUE,
                                                &posOutput.extent_hint,
                                                &posOutput));
        }
        else
        {
            AEFX_CLR_STRUCT(composite_mode);
            composite_mode.opacity = PF_MAX_CHAN16;
            composite_mode.opacitySu =PF_MAX_CHAN16;
            composite_mode.xfer = PF_Xfer_COPY;
            ERR(in_data->utils->transform_world(in_data->effect_ref,
                                                in_data->quality,
                                                PF_MF_Alpha_STRAIGHT,
                                                in_data->field,
                                                inputP,
                                                &composite_mode,
                                                NULL,
                                                &float_matrix,
                                                1,
                                                TRUE,
                                                &posOutput.extent_hint,
                                                &posOutput));
        }
        
        
        
        
        switch (destinationPixelFormat)
        {
                
            case PrPixelFormat_BGRA_4444_8u:
                
                iterate8Suite->iterate(in_data,
                                       0,
                                       (output->extent_hint.bottom - output->extent_hint.top),
                                      &posOutput,
                                       NULL,
                                       (void*)&letP,
                                       PixelFuncBGRA_8u,
                                       output);
                
                break;
                
            case PrPixelFormat_VUYA_4444_8u:
                

                iterate8Suite->iterate(in_data,
                                       0,
                                       (output->extent_hint.bottom - output->extent_hint.top),
                                       &posOutput,
                                       NULL,
                                       (void*)&letP,
                                       PixelFuncVUYA_8u,
                                       output);
                
                break;
                
            case PrPixelFormat_BGRA_4444_32f:
                // Premiere doesn't support IterateFloatSuite1, so we've rolled our own
                IterateFloat(in_data,
                             0,
                             (output->extent_hint.bottom - output->extent_hint.top),
                             &posOutput,
                             (void*)&letP,
                             PixelFuncBGRA_32f,
                             output);
                
                break;
            case PrPixelFormat_VUYA_4444_16u:
                //float YUV has problem with matrix operation so cheat and use 16bits
                IterateDeep(in_data,
                                        0,
                                        (output->extent_hint.bottom - output->extent_hint.top),
                                        &posOutput,
                                        (void*)&letP,
                                        PixelFuncVUYA_16u,
                                        output);
                break;
            

            default:
                //	Return error, because we don't know how to handle the specified pixel type
                return PF_Err_UNRECOGNIZED_PARAM_TYPE;
        }
    }
   
return err;
}



static PF_Err
PreRender(
	PF_InData				*in_data,
	PF_OutData				*out_data,
	PF_PreRenderExtra		*extraP)
{
    PF_Err	err				= PF_Err_NONE,
    err2            = PF_Err_NONE;


	PF_RenderRequest req	= extraP->input->output_request;

	PF_CheckoutResult		in_result;
	AEGP_SuiteHandler		suites(in_data->pica_basicP);
    PF_ParamDef             param_center, param_scale;
    
	PF_Handle	infoH		=	suites.HandleSuite1()->host_new_handle(sizeof(prerender_letP));


	prerender_letP		*letP = NULL;
    

	if (infoH){
		letP = reinterpret_cast<prerender_letP*>(suites.HandleSuite1()->host_lock_handle(infoH));
		if (letP){
            AEFX_CLR_STRUCT(*letP);
            
            
            AEGP_LayerH		layerH;
            AEGP_CompH		compH;
            AEGP_ItemH      itemH;
            AEFX_SuiteScoper<AEGP_PFInterfaceSuite1> PFInterfaceSuite = AEFX_SuiteScoper<AEGP_PFInterfaceSuite1>(	in_data,
                                                                                                                 kAEGPPFInterfaceSuite,
                                                                                                                 kAEGPPFInterfaceSuiteVersion1,
                                                                                                                 out_data);
            AEFX_SuiteScoper<AEGP_LayerSuite8> layerSuite = AEFX_SuiteScoper<AEGP_LayerSuite8>(	in_data,
                                                                                               kAEGPLayerSuite,
                                                                                               kAEGPLayerSuiteVersion8,
                                                                                               out_data);
            AEFX_SuiteScoper<AEGP_CompSuite10> compSuite = AEFX_SuiteScoper<AEGP_CompSuite10>(  in_data,
                                                                                              kAEGPCompSuite,
                                                                                              kAEGPCompSuiteVersion10,
                                                                                              out_data);
            AEFX_SuiteScoper<AEGP_ItemSuite9> itemSuite = AEFX_SuiteScoper<AEGP_ItemSuite9>(  in_data,
                                                                                            kAEGPItemSuite,
                                                                                            kAEGPItemSuiteVersion9,
                                                                                            out_data);
            
            AEFX_SuiteScoper<AEGP_StreamSuite4> StreamSuite = AEFX_SuiteScoper<AEGP_StreamSuite4>(  in_data,
                                                                                            kAEGPStreamSuite,
                                                                                            kAEGPStreamSuiteVersion4,
                                                                                            out_data);
        
            
            
            
            PFInterfaceSuite->AEGP_GetEffectLayer(in_data->effect_ref, &layerH);
            layerSuite->AEGP_GetLayerParentComp(layerH, &compH);
            compSuite->AEGP_GetItemFromComp (compH, &itemH);
            A_long width, height;
            A_Time currTime;
            AEGP_StreamVal2 strValP;
            AEGP_StreamType  strTypeP;
             AEFX_CLR_STRUCT(width);
            AEFX_CLR_STRUCT(height);
            itemSuite->AEGP_GetItemDimensions(itemH, &width, &height);
            letP->compWidthF =  PF_FpLong (width);
            letP->compHeightF = PF_FpLong (height);
            
            layerSuite->AEGP_GetLayerCurrentTime (layerH, AEGP_LTimeMode_LayerTime, &currTime);
            StreamSuite->AEGP_GetLayerStreamValue(layerH, AEGP_LayerStream_POSITION, AEGP_LTimeMode_LayerTime, &currTime, NULL, &strValP, &strTypeP);
            letP->layerPx = strValP.three_d.x;
            letP->layerPy = strValP.three_d.y;
            
            
            if (extraP->cb->GuidMixInPtr) {
                extraP->cb->GuidMixInPtr(in_data->effect_ref, sizeof(width), reinterpret_cast<void *>(&width));
                extraP->cb->GuidMixInPtr(in_data->effect_ref, sizeof(height), reinterpret_cast<void *>(&height));
                extraP->cb->GuidMixInPtr(in_data->effect_ref, sizeof(strValP), reinterpret_cast<void *>(&strValP));
            }
           
            
           
            
			extraP->output->pre_render_data = infoH;
            
             AEFX_CLR_STRUCT(in_result);
			if (!err){
				req.preserve_rgb_of_zero_alpha = TRUE;
                req.field = PF_Field_FRAME;
                
                PF_LRect fullLayer = {0, 0, in_data->width, in_data->height};
                  UnionLRect(&fullLayer, &req.rect);

				
				ERR(extraP->cb->checkout_layer(	in_data->effect_ref,
												LETB_INPUT,
												LETB_INPUT,
												&req,
												in_data->current_time,
												in_data->time_step,
												in_data->time_scale,
												&in_result));
                
                extraP->output->flags = PF_RenderOutputFlag_RETURNS_EXTRA_PIXELS;
				if (!err){
                    
                    AEFX_CLR_STRUCT(param_center);
                    ERR(PF_CHECKOUT_PARAM(	in_data,
                                          LETB_CENTER,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &param_center));
                    
                    letP->x_tA =   param_center.u.td.x_value;
                     letP->y_tA =  param_center.u.td.y_value;
                    ERR2(PF_CHECKIN_PARAM(in_data, &param_center));
                    
                    AEFX_CLR_STRUCT(param_scale);
                    ERR(PF_CHECKOUT_PARAM(in_data,
                                          LETB_RESIZE,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &param_scale));
                    
                    letP->scaleFactorF = param_scale.u.fs_d.value/100;
                    ERR2(PF_CHECKIN_PARAM(in_data, &param_scale));
                    
                    PF_Fixed 	widthF	= INT2FIX(ABS(in_result.max_result_rect.right - in_result.max_result_rect.left)),
                                heightF = INT2FIX(ABS(in_result.max_result_rect.bottom - in_result.max_result_rect.top));
                    
                   
                    letP->x_offF = PF_Fixed((widthF * letP->scaleFactorF/2) - letP->x_tA);
                    letP->y_offF = PF_Fixed((heightF* letP->scaleFactorF/2) - letP->y_tA);

					UnionLRect(&in_result.result_rect, 		&extraP->output->result_rect);
					UnionLRect(&in_result.max_result_rect, 	&extraP->output->max_result_rect);	
				}
			}
			suites.HandleSuite1()->host_unlock_handle(infoH);
		}
	}
	return err;
}

static PF_Err
SmartRender(
	PF_InData				*in_data,
	PF_OutData				*out_data,
	PF_SmartRenderExtra		*extraP)
{
	
	PF_Err			err			= PF_Err_NONE,
					err2		= PF_Err_NONE;
	
	PF_EffectWorld	*inputP		= NULL,
					*outputP	= NULL;
	
	PF_PixelFormat	format		=	PF_PixelFormat_INVALID;
	PF_WorldSuite2	*wsP		=	NULL;
    PF_Point			originPt;

    AEGP_SuiteHandler suites(in_data->pica_basicP);
    

    PF_ParamDef param_preset, param_slider, param_mode, param_trsp, param_color,  param_size_source;
   


	
	ERR(AEFX_AcquireSuite(	in_data, 
							out_data, 
							kPFWorldSuite, 
							kPFWorldSuiteVersion2, 
							STR(StrID_Err_LoadSuite),
							(void**)&wsP));
	if (!err && wsP ){  //&& seqP

		prerender_letP *letP = reinterpret_cast<prerender_letP*>(suites.HandleSuite1()->host_lock_handle(reinterpret_cast<PF_Handle>(extraP->input->pre_render_data)));
        
		if (letP){
            
            AEFX_CLR_STRUCT(param_preset);
            ERR(PF_CHECKOUT_PARAM(in_data,
                                  LETB_PRESET,
                                  in_data->current_time,
                                  in_data->time_step,
                                  in_data->time_scale,
                                  &param_preset));
            
             GetPresetRatioValue(param_preset.u.pd.value, &letP->PreseTvalueF);
           
            ERR2(PF_CHECKIN_PARAM(in_data, &param_preset));
            
            
            AEFX_CLR_STRUCT(param_slider);
            ERR(PF_CHECKOUT_PARAM(	in_data,
                                  LETB_SLIDER,
                                  in_data->current_time,
                                  in_data->time_step,
                                  in_data->time_scale,
                                  &param_slider));
            
            letP->SlidervalueF =param_slider.u.fs_d.value;
            ERR2(PF_CHECKIN_PARAM(in_data, &param_slider));
            
            
            AEFX_CLR_STRUCT(param_mode);
            ERR(PF_CHECKOUT_PARAM(in_data,
                                  LETB_MODE,
                                  in_data->current_time,
                                  in_data->time_step,
                                  in_data->time_scale,
                                  &param_mode));
            
            
            A_long tempMode;
            GetModeValue(param_mode.u.pd.value, &tempMode);
            letP->userRatioF = (letP->PreseTvalueF * ABS(tempMode-1) )+ (letP->SlidervalueF * tempMode);
            ERR2(PF_CHECKIN_PARAM(in_data, &param_mode));
            
            AEFX_CLR_STRUCT(param_trsp);
            ERR(PF_CHECKOUT_PARAM(in_data,
                                  LETB_TRSP,
                                  in_data->current_time,
                                  in_data->time_step,
                                  in_data->time_scale,
                                  &param_trsp));
            letP->PoTransparentB = param_trsp.u.bd.value;
            ERR2(PF_CHECKIN_PARAM(in_data, &param_trsp));
            
            
            AEFX_CLR_STRUCT(param_color);
            ERR(PF_CHECKOUT_PARAM(	in_data,
                                  LETB_COLOR,
                                  in_data->current_time,
                                  in_data->time_step,
                                  in_data->time_scale,
                                  &param_color));
            

            letP->Color = param_color.u.cd.value;
            ERR(suites.ColorParamSuite1()->PF_GetFloatingPointColorFromColorDef(in_data->effect_ref, &param_color, &letP->Color32));

            ERR2(PF_CHECKIN_PARAM(in_data, &param_color));
            
            
            AEFX_CLR_STRUCT(param_size_source);
            ERR(PF_CHECKOUT_PARAM(in_data,
                                  LETB_SIZE_SOURCE,
                                  in_data->current_time,
                                  in_data->time_step,
                                  in_data->time_scale,
                                  &param_size_source));
            
            A_long tempCompMode;
            GetModeValue(param_size_source.u.pd.value, &tempCompMode);
            if (tempCompMode ==1)
            {
                letP->compModeB = true;
            }
            else
            {
                letP->compModeB = false;
            }
            
            ERR2(PF_CHECKIN_PARAM(in_data, &param_size_source));
            
            
   
            letP->PixRatioNumF = in_data->pixel_aspect_ratio.num;
            letP->PixRatioDenF = in_data->pixel_aspect_ratio.den;
            PF_FpLong scale_x = in_data->downsample_x.num / (float)in_data->downsample_x.den,
            scale_y = in_data->downsample_y.num / (float)in_data->downsample_y.den;
            
            
            letP->InputWidthF  = PF_FpLong (in_data->width);
            letP->InputHeightF  = PF_FpLong (in_data->height);
            
            //DOWNSCALE
            letP->InputWidthF   *=   scale_x;
            letP->InputHeightF  *=   scale_y;
            letP->compWidthF    *=   scale_x;
            letP->compHeightF   *=   scale_y;
            
            letP->layerWidthF =   letP->InputWidthF;
            letP->layerHeightF =  letP->InputHeightF;
            
            // checkout input & output buffers.
            ERR((extraP->cb->checkout_layer_pixels(	in_data->effect_ref, LETB_INPUT, &inputP)));
            ERR(extraP->cb->checkout_output(in_data->effect_ref, &outputP));
            letP->in_data = *in_data;
            letP->samp_pb.src = inputP;
            
            
            // determine requested output depth
            ERR(wsP->PF_GetPixelFormat(outputP, &format));
            originPt.h = (A_short)(in_data->output_origin_x);
            originPt.v = (A_short)(in_data->output_origin_y);
            
            
            
            //if compMode
            if (letP->compModeB ==true)
            {
                letP->layerRatioF = (((double)letP->compWidthF) *  letP->PixRatioNumF) / ((double)letP->compHeightF *letP->PixRatioDenF); //ratio input from comp;
                letP->letoffxF =  (PF_FpLong (letP->compWidthF-  letP->layerWidthF));
                letP->letoffyF =  (PF_FpLong (letP->compHeightF- letP->layerHeightF));
                letP->InputWidthF =  PF_FpLong (letP->compWidthF);
                letP->InputHeightF = PF_FpLong (letP->compHeightF);
                
                letP->compoffxF =(0.5*letP->compWidthF)- letP-> layerPx;
                letP->compoffyF =(0.5*letP->compHeightF)- letP-> layerPy;
                
                /*Given a layer handle and time, returns the layer-to-world transformation matrix.
                AEGP_GetLayerToWorldXform(
                                          AEGP_LayerH const A_Time A_Matrix4
                                          aegp_layerH,  *comp_timeP,  *transform);*/

            }
            else
            {
                letP->compoffxF =0;
                letP->compoffyF =0;
                letP->letoffxF =0;
                letP->letoffyF = 0;
                letP->layerRatioF = (((double)in_data->width) *  letP->PixRatioNumF) / ((double)in_data->height*letP->PixRatioDenF); //ratio input from layer;
            }

			switch (format) {
				
				case PF_PixelFormat_ARGB128:

					ERR(suites.IterateFloatSuite1()->iterate_origin(	in_data,
																0,
																(outputP->extent_hint.bottom - outputP->extent_hint.top),
                                                                inputP,
																&outputP->extent_hint,
                                                                &originPt,
																(void*)letP,
																PixelFuncFloat,
																outputP));
					break;
					
				case PF_PixelFormat_ARGB64:

					ERR(suites.Iterate16Suite1()->iterate_origin(in_data,
                                                                0,
                                                                (outputP->extent_hint.bottom - outputP->extent_hint.top),
                                                                inputP,
                                                                &outputP->extent_hint,
                                                                &originPt,
                                                                 (void*)letP,
                                                                 PixelFunc16,
                                                                 outputP));
					break;
					
                    
                    
				case PF_PixelFormat_ARGB32:
                    

                    ERR(suites.Iterate8Suite1()->iterate_origin(	in_data,
                                                                0,
                                                                (outputP->extent_hint.bottom - outputP->extent_hint.top),
                                                                inputP,
                                                                &outputP->extent_hint,
                                                                &originPt,
                                                                (void*)letP,
                                                                PixelFunc8,
                                                                outputP));
					break;

				default:
					err = PF_Err_INTERNAL_STRUCT_DAMAGED;
					break;
			}

		}
		suites.HandleSuite1()->host_unlock_handle(reinterpret_cast<PF_Handle>(extraP->input->pre_render_data));
        
    }
	ERR2(AEFX_ReleaseSuite(	in_data,
							out_data,
							kPFWorldSuite, 
							kPFWorldSuiteVersion2, 
							STR(StrID_Err_FreeSuite)));
	return err;
}

static PF_Err
RespondtoAEGP (
               PF_InData		*in_data,
               PF_OutData		*out_data,
               PF_ParamDef		*params[],
               PF_LayerDef		*output,
               void*			extraP)
{
    PF_Err			err = PF_Err_NONE;
    
    AEGP_SuiteHandler suites(in_data->pica_basicP);
    
    suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
                                          "%s",	
                                          reinterpret_cast<A_char*>(extraP));
    
    return err;
}




DllExport	PF_Err
EntryPointFunc(	
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra )
{
	PF_Err		err = PF_Err_NONE;

	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:
				err = About(in_data,
							out_data,
							params,
							output);
				break;

			case PF_Cmd_GLOBAL_SETUP:
				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;
			
			case PF_Cmd_GLOBAL_SETDOWN:
				err = GlobalSetdown(in_data);
									
				break;

			case PF_Cmd_PARAMS_SETUP:
				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
            
                

			case PF_Cmd_RENDER:
				err = Render(	in_data,
								out_data,
								params,
								output);
				break;

			case PF_Cmd_SMART_RENDER:
				err = SmartRender(	in_data,
									out_data,
									reinterpret_cast<PF_SmartRenderExtra*>(extra));
				break;

			case PF_Cmd_SMART_PRE_RENDER:
				err = PreRender(	in_data,
									out_data,
									reinterpret_cast<PF_PreRenderExtra*>(extra));
				break;
                
			case PF_Cmd_USER_CHANGED_PARAM:
				err = UserChangedParam(	in_data,
										out_data,
										params,
										output,
                                       reinterpret_cast<const PF_UserChangedParamExtra *>(extra));
                                       
				break;
                
            case PF_Cmd_COMPLETELY_GENERAL:
                err = RespondtoAEGP(in_data,
                                    out_data,
                                    params,
                                    output, 
                                    extra);
		break;
				  
				
       

			case PF_Cmd_UPDATE_PARAMS_UI:
				err = UpdateParameterUI(	in_data,
											out_data,
											params,
											output);

			default:
				break;
                
            
		}
	}
	catch(PF_Err &thrown_err){
		err = PF_Err_INTERNAL_STRUCT_DAMAGED;
	}
	return err;
}
