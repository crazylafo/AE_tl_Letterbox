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
                                PF_OutFlag_DEEP_COLOR_AWARE;


	
	out_data->out_flags2	=	PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG	|
                                PF_OutFlag2_SUPPORTS_SMART_RENDER				|
								PF_OutFlag2_FLOAT_COLOR_AWARE					|
								PF_OutFlag2_DOESNT_NEED_EMPTY_PIXELS			|
                                PF_OutFlag2_I_USE_COLORSPACE_ENUMERATION;       // FOR PREMIERE
								//PF_OutFlag2_I_MIX_GUID_DEPENDENCIES;
    
    
    

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
                                                     PrPixelFormat_BGRA_4444_32f);
        (*pixelFormatSuite->AddSupportedPixelFormat)(
                                                     in_data->effect_ref,
                                                     PrPixelFormat_BGRA_4444_8u);
        (*pixelFormatSuite->AddSupportedPixelFormat)(
                                                     in_data->effect_ref,
                                                     PrPixelFormat_VUYA_4444_8u);
       
    }
    //	This looks more complex than it is. Basically, this
	//	code allocates a handle, and (if successful) sets a 
	//	sentinel value for later use, and gets an ID for using
	//	AEGP calls. It then stores those value in  in_data->global_data.
	
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
SequenceSetdown (	
					PF_InData		*in_data,
					PF_OutData		*out_data)
{
	PF_Err err = PF_Err_NONE;
	
	if (in_data->sequence_data){
		AEGP_SuiteHandler suites(in_data->pica_basicP);
		suites.HandleSuite1()->host_dispose_handle(in_data->sequence_data);
	}
	return err;
}

static PF_Err 
SequenceSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data)
{
	PF_Err err = PF_Err_NONE;
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	err = SequenceSetdown(in_data, out_data);
	
	if (!err){
		PF_Handle	seq_dataH =	suites.HandleSuite1()->host_new_handle(sizeof(my_sequence_data));
		
		if (seq_dataH){
			my_sequence_dataP seqP = static_cast<my_sequence_dataP>(suites.HandleSuite1()->host_lock_handle(seq_dataH));
			if (seqP){
				if (in_data->appl_id != 'PrMr')
				{
					ERR(suites.ParamUtilsSuite3()->PF_GetCurrentState(	in_data->effect_ref,
																		LETB_SLIDER,
																		NULL,
																		NULL,
																		&seqP->state));
                    
                    
				}
                

				out_data->sequence_data = seq_dataH;
				suites.HandleSuite1()->host_unlock_handle(seq_dataH);
			}
		} else {	// whoa, we couldn't allocate sequence data; bail!
			err = PF_Err_OUT_OF_MEMORY;
		}
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
    AEFX_CLR_STRUCT(def);
    
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
    
    

    def.flags		|=	PF_ParamFlag_SUPERVISE |
    PF_ParamFlag_CANNOT_TIME_VARY;
    
    def.ui_flags	= PF_PUI_STD_CONTROL_ONLY;
    // IN AE the checkbox detect the ratio of the frame, but for know in premiere it gets the ratio of the layer size.
    if (in_data->appl_id == 'PrMr')
    {
        PF_ADD_CHECKBOX("Input Ratio",
                        "Get the ratio of the layer size",
                        FALSE,
                        0,
                        LETB_DETECT_DISK_ID);
        
        
    }
    else
    {
        PF_ADD_CHECKBOX(STR(StrID_CheckboxName),
                        STR(StrID_CheckboxCaption),
                        FALSE,
                        0,
                        LETB_DETECT_DISK_ID);
    }
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
    
    def.flags		|=	PF_ParamFlag_SUPERVISE |
    PF_ParamFlag_CANNOT_TIME_VARY;
    
    PF_ADD_CHECKBOX(STR(StrID_ForceSName),
                    STR(StrID_ForceSDescription),
                    FALSE,
                    0,
                    LETB_FORCE_SCALE_DISK_ID);
    
    AEFX_CLR_STRUCT(def);
    PF_END_TOPIC (END_TOPIC_GR3_DISK_ID);
    AEFX_CLR_STRUCT(def);
    
    
    //AEGP GROUP
    
    def.flags		|=	PF_ParamFlag_SUPERVISE;
    
    
    PF_ADD_POINT(	STR(StrID_AEGP_GET_POSITION),
                 50,
                 50,
                 0,
                 AEGP_GET_POSITION_DISK_ID);

    
    AEFX_CLR_STRUCT(def);
    
    
    def.flags		|=	PF_ParamFlag_SUPERVISE;
    

    
    PF_ADD_POINT(	STR(StrID_AEGP_GET_ANCHORPOINT),
                 50,
                 50,
                 0,
                 AEGP_GET_ANCHORPOINT_DISK_ID);
    
    AEFX_CLR_STRUCT(def);
    
    def.flags		|=	PF_ParamFlag_SUPERVISE;
    
    
    PF_ADD_POINT(STR(StrID_AEGP_GET_SCALE),
                 100,
                 100,
                 0,
                 AEGP_GET_SCALE_DISK_ID);
    
    AEFX_CLR_STRUCT(def);


    PF_ADD_FLOAT_SLIDERX(STR(StrID_AEGP_GET_COMP_PIX_RATIO),
                    0,
                    20,
                    0,
                    20,
                    1,
                    PF_Precision_HUNDREDTHS,
                    0,
                    PF_ParamFlag_SUPERVISE,
                    AEGP_GET_COMP_PIX_RATIO_DISK_ID);
    AEFX_CLR_STRUCT(def);
    
     
     def.flags		|=	PF_ParamFlag_SUPERVISE;
     
     
     PF_ADD_POINT(STR(StrID_AEGP_GETCOMP_SIZE),
     50,
     50,
     0,
     AEGP_GET_COMPSIZE_DISK_ID);
     
     AEFX_CLR_STRUCT(def);


    
    
    out_data->num_params = LETB_NUM_PARAMS;
    

    
    

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
                   PF_FpLong		*detectedRatioF)
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


    PF_FpShort Tolerencepx8_blue = float (0.010);
	PF_FpShort Tolerencepx8_green = float(0.010);
	PF_FpShort Tolerencepx8_red = float(0.010);

    
    A_long halfWidthA = A_long (InputWidthF*0.5);
    A_long halfHeightA = A_long (InputHeightF *0.5);

	PF_FpLong ratioHF, ratioVF;
    
 
    for (A_long i =1; i< halfHeightA; i++)
    {

        for (A_long j =1; j< A_long (InputWidthF); j++)
        {
            PF_PixelFloat PixelValue;
            AEFX_CLR_STRUCT(PixelValue);
             GetPixelValue(detectWorldP, pxformat, j, i, &PixelValue);
            
			if (PixelValue.blue > Tolerencepx8_blue || PixelValue.green > Tolerencepx8_green || PixelValue.red > Tolerencepx8_red)
			{
				ratioHF = (((double)InputWidthF) *  PixRatioNumF) / ((double)InputHeightF *PixRatioDenF - 2 * i);
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

			if (PixelValueh.blue > Tolerencepx8_blue || PixelValueh.green > Tolerencepx8_green || PixelValueh.red > Tolerencepx8_red)
			{
				ratioVF = (((double)InputWidthF - 2 * k) *  PixRatioNumF ) / ((double)InputHeightF *PixRatioDenF);
				k = halfWidthA - 1;
				break;
			}

        }
    }
    PF_FpLong layerRatio = (((double)InputWidthF) *  PixRatioNumF) / ((double)InputHeightF *PixRatioDenF); //ratio input from layer
	if (ratioHF >layerRatio)
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
		*detectedRatioF = layerRatio;
    }


}


static PF_FpLong
CalculateBox(
	void		*refcon,
	PF_FpLong		xL,
	PF_FpLong		yL)

{
	PF_Err		err = PF_Err_NONE;


    PF_FpLong CondBlackHupF, CondBlackHdownF, CondBlackVleftF, CondBlackVrightF;
	prerender_letP	*letP = reinterpret_cast<prerender_letP*>(refcon);

	int userRatioInt = int(letP->userRatioF * 100);
	int layerRatioInt = int(letP->layerRatioF * 100);
    
    
    
    //Modification of offsets here.

	//definitions for horizontal letterbox
    CondBlackHupF = ((letP->InputHeightF*letP->scaleFactoryF  - (letP->InputWidthF/(letP->userRatioF)))/ 2/letP->scaleFactoryF) ;
    CondBlackHdownF =   letP->InputHeightF - CondBlackHupF;

	//definitions for verticals letterbox
    CondBlackVleftF = ((letP->InputWidthF*letP->scaleFactorxF  - (letP->InputHeightF *  letP->userRatioF))/2/letP->scaleFactorxF);
    CondBlackVrightF = letP->InputWidthF   -  CondBlackVleftF;


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
    copy[LETB_CHECKBOX]         = *actual[LETB_CHECKBOX];
    copy[LETB_TRSP]             = *actual[LETB_TRSP];
    copy[LETB_COLOR]            = *actual[LETB_COLOR];
    copy[LETB_GR1]              = *actual[LETB_GR1];
    copy[LETB_CENTER]           = *actual[LETB_CENTER];
    copy[LETB_RESIZE]           = *actual[LETB_RESIZE];
    copy[LETB_GR2]              = *actual[LETB_GR2];
    copy[LETB_SIZE_SOURCE]       = *actual[LETB_SIZE_SOURCE];
    copy[LETB_FORCE_SCALE]      = *actual[LETB_FORCE_SCALE];
    
    copy[AEGP_GET_POSITION]      = *actual[AEGP_GET_POSITION];
    copy[AEGP_GET_SCALE]         = *actual[AEGP_GET_SCALE];
    copy[AEGP_GET_ANCHORPOINT]   = *actual[AEGP_GET_ANCHORPOINT];
    copy[AEGP_GET_COMPSIZE]     = *actual[AEGP_GET_COMPSIZE];
    copy[AEGP_GET_COMP_PIX_RATIO]= *actual[AEGP_GET_COMP_PIX_RATIO];
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
    
    if (which_hitP->param_index == LETB_CHECKBOX)
    {
        // If checkbox is checked, change slider value to the layer ratio.
        AEGP_SuiteHandler		suites(in_data->pica_basicP);
        
        
        
        if (params[LETB_CHECKBOX]->u.bd.value == TRUE)
        {
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
					PF_ParamDef paramInput;
					AEFX_CLR_STRUCT(paramInput);
                    AEFX_CLR_STRUCT(scanWorldP);
					ERR(PF_CHECKOUT_PARAM(	in_data,
										  LETB_LAYER_ANALYS,
										  in_data->current_time,
										  in_data->time_step,
										  in_data->time_scale,
										  &paramInput));
					scanWorldP = &paramInput.u.ld;
					

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
                 	GetRatioFromWorld (in_data, scanWorldP, detectFormat,&scanlayerRatioF);
                    ERR2(PF_CHECKIN_PARAM(in_data, &paramInput));
                }
                
            }
            else // FOR KNOW CHEAT AND DETECT THE LAYER RATIO
            {
                PF_FpLong InputWidthF , InputHeightF , PixRatioNumF, PixRatioDenF;
                InputWidthF  = in_data->width;
                InputHeightF  = in_data->height;
                PixRatioNumF = in_data->pixel_aspect_ratio.num;
                PixRatioDenF = in_data->pixel_aspect_ratio.den;
                
               
                PF_FpLong scale_x = in_data->downsample_x.num/ (float)in_data->downsample_x.den,
                scale_y = in_data->downsample_y.num/ (float)in_data->downsample_y.den;
                InputWidthF  *= scale_x;
                InputHeightF  *= scale_y;
                scanlayerRatioF  =  InputWidthF  / InputHeightF ; //ratio input from layer
            }
           
            
            params[LETB_SLIDER]->u.fs_d.value= scanlayerRatioF;
            params[LETB_SLIDER]->uu.change_flags = PF_ChangeFlag_CHANGED_VALUE;
            
            params[LETB_CHECKBOX]->u.bd.value = FALSE;
           
            ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                            LETB_CHECKBOX,
                                                            params[LETB_CHECKBOX]));
        }
    }
    
    
    
    
    if (which_hitP->param_index)
    {
        AEGP_SuiteHandler		suites(in_data->pica_basicP);
        
        AEGP_StreamRefH  aegp_get_position_streamH       = NULL,
        aegp_get_anchorpoint_streamH    = NULL,
        aegp_get_scale_streamH          = NULL,
        aegp_get_compSize_streamH       = NULL,
        aegp_get_compPixRatio_streamH   = NULL;
        AEGP_EffectRefH			meH				= NULL;
        my_global_dataP		globP				= reinterpret_cast<my_global_dataP>(DH(out_data->global_data));
        
        // AEGP COMUNICATION
        
        ERR(suites.PFInterfaceSuite1()->AEGP_GetNewEffectForEffect(globP->my_id, in_data->effect_ref, &meH));
        
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, AEGP_GET_POSITION, &aegp_get_position_streamH));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, AEGP_GET_ANCHORPOINT,& aegp_get_anchorpoint_streamH));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, AEGP_GET_SCALE, &aegp_get_scale_streamH ));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, AEGP_GET_COMPSIZE,&aegp_get_compSize_streamH));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, AEGP_GET_COMP_PIX_RATIO,&aegp_get_compPixRatio_streamH));
        
        //AND NOW WE ADD EXPRESSIONS
        /*
        ERR(suites.StreamSuite4()->AEGP_SetExpression(globP->my_id,aegp_get_position_streamH, ("transform.position")));
        ERR(suites.StreamSuite4()->AEGP_SetExpression(globP->my_id,aegp_get_anchorpoint_streamH, ("transform.anchorPoint")));
        ERR(suites.StreamSuite4()->AEGP_SetExpression(globP->my_id,aegp_get_scale_streamH, ("transform.scale")));
        ERR(suites.StreamSuite4()->AEGP_SetExpression(globP->my_id,aegp_get_compPixRatio_streamH, ("thisComp.pixelAspect")));
        ERR(suites.StreamSuite4()->AEGP_SetExpression(globP->my_id,aegp_get_compSize_streamH, ("[thisComp.width, thisComp.height]")));*/
        
        
        //dispose stream
        if (meH){
            ERR2(suites.EffectSuite2()->AEGP_DisposeEffect(meH));
        }
        
        if (aegp_get_position_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(aegp_get_position_streamH));
        }
        if (aegp_get_anchorpoint_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(aegp_get_anchorpoint_streamH));
        }
        if (aegp_get_scale_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(aegp_get_scale_streamH));
            
        }if (aegp_get_compSize_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(aegp_get_compSize_streamH));
        }
        if (aegp_get_compPixRatio_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(aegp_get_compPixRatio_streamH));
        }
        
        ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                        AEGP_GET_POSITION,
                                                        params[AEGP_GET_POSITION]));
        
        ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                        AEGP_GET_SCALE,
                                                        params[AEGP_GET_SCALE]));
        
        ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                        AEGP_GET_ANCHORPOINT,
                                                        params[AEGP_GET_ANCHORPOINT]));
        ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                        AEGP_GET_COMP_PIX_RATIO,
                                                        params[AEGP_GET_COMP_PIX_RATIO]));
        
        ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                        AEGP_GET_COMPSIZE,
                                                        params[AEGP_GET_COMPSIZE]));
        
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
    my_sequence_dataP	seqP				= reinterpret_cast<my_sequence_dataP>(DH(out_data->sequence_data));
    AEGP_StreamRefH     preset_streamH		= NULL,
    slider_streamH		= NULL,
    trsp_streamH        = NULL,
    color_streamH       = NULL,
    topic_streamH       = NULL,
    center_streamH      = NULL,
    resize_streamH      = NULL,
    topic2_streamH      = NULL,
    sizeSource_streamH  = NULL,
    forceScale_streamH  = NULL,
    aegp_get_position_streamH       = NULL,
    aegp_get_anchorpoint_streamH    = NULL,
    aegp_get_scale_streamH          = NULL,
    aegp_get_compSize_streamH       = NULL,
    aegp_get_compPixRatio_streamH   = NULL;
    
    PF_ParamType		param_type;
    PF_ParamDefUnion	param_union;
    
    A_Boolean			hide_oneB       = FALSE,
                        hide_twoB		= FALSE,
                        hide_threeB     = FALSE;
    
    AEGP_EffectRefH			meH				= NULL;
    AEGP_SuiteHandler		suites(in_data->pica_basicP);
    
    
    PF_ParamDef		param_copy[LETB_NUM_PARAMS];
    ERR(MakeParamCopy(params, param_copy));
    

    
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
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, LETB_FORCE_SCALE,&forceScale_streamH));
        
        
        // AEGP COMUNICATION
        
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, AEGP_GET_POSITION, &aegp_get_position_streamH));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, AEGP_GET_ANCHORPOINT,& aegp_get_anchorpoint_streamH));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, AEGP_GET_SCALE, &aegp_get_scale_streamH ));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, AEGP_GET_COMPSIZE,&aegp_get_compSize_streamH));
        ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(globP->my_id, meH, AEGP_GET_COMP_PIX_RATIO,&aegp_get_compPixRatio_streamH));

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
        if (forceScale_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(forceScale_streamH));
        }
        
        if (aegp_get_position_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(aegp_get_position_streamH));
        }
        if (aegp_get_anchorpoint_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(aegp_get_anchorpoint_streamH));
        }
        if (aegp_get_scale_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(aegp_get_scale_streamH));
        }
        if (aegp_get_compSize_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(aegp_get_compSize_streamH));
        }
        if (aegp_get_compPixRatio_streamH){
            ERR2(suites.StreamSuite2()->AEGP_DisposeStream(aegp_get_compPixRatio_streamH));
        }
        

        
        if (!err){
            out_data->out_flags |= PF_OutFlag_FORCE_RERENDER;
        }
        
        
        
        // Demonstrate using PF_AreStatesIdentical to check whether a parameter has changed
        PF_State		new_state;
        A_Boolean		something_changedB		= FALSE;
        
        ERR(suites.ParamUtilsSuite3()->PF_GetCurrentState(		in_data->effect_ref,
                                                          LETB_SLIDER,
                                                          NULL,
                                                          NULL,
                                                          &new_state));
        
        ERR(suites.ParamUtilsSuite3()->PF_AreStatesIdentical(	in_data->effect_ref,
                                                             &seqP->state,
                                                             &new_state,
                                                             &something_changedB));
        
        if (something_changedB || !globP->initializedB)	{
            //	If something changed (or it's the first time we're being called),
            //	get the new state and store it in our sequence data
            
            ERR(suites.ParamUtilsSuite3()->PF_GetCurrentState(	in_data->effect_ref,
                                                              LETB_SLIDER,
                                                              NULL,
                                                              NULL,
                                                              &seqP->state));
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
                // 	Since we're in advanced mode, show the advanced-only params and hide the preset param
                
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
            //HIDE THOSE PARAM ON ADOBE PREMIERE TOPIC_GR3_DISK_ID   LETB_SIZE_SOURCE_DISK_ID)   LETB_FORCE_SCALE_DISK_ID
           
          
            
          
            if (!err)
                {
                    param_copy[LETB_GR3].ui_flags |=	PF_PUI_INVISIBLE;
                    ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                    LETB_GR3,
                                                                    &param_copy[LETB_GR3]));
                }
            if (!err)
            {
                param_copy[LETB_SIZE_SOURCE].ui_flags |=	PF_PUI_INVISIBLE;
                ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                LETB_SIZE_SOURCE,
                                                                &param_copy[LETB_SIZE_SOURCE]));
            }
            if (!err)
            {
                param_copy[LETB_FORCE_SCALE].ui_flags |=	PF_PUI_INVISIBLE;
                ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref,
                                                                LETB_FORCE_SCALE,
                                                                &param_copy[LETB_FORCE_SCALE]));
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
    PF_LayerDef		 *posOutput = output;

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
    
     letP.layerRatioF = (((double)in_data->width) *  letP.PixRatioNumF) / ((double)in_data->height*letP.PixRatioDenF); //ratio input from layer
    

    letP.Color = params[LETB_COLOR]->u.cd.value;
    letP.PoTransparentB = params[LETB_TRSP]->u.bd.value;
    


    if (in_data->appl_id == 'PrMr') {

        if (MODE_BASIC == params[LETB_MODE]->u.pd.value){
            GetPresetRatioValue( params[LETB_PRESET]->u.pd.value, &letP.userRatioF);
        } else {
            letP.userRatioF = params[LETB_SLIDER]->u.fs_d.value;
        }
        
        //POSITION PART
        PF_FpLong scaleFactor;
        scaleFactor = params[LETB_RESIZE]->u.fs_d.value/100;

        PF_Pixel transparent_black = {0, 0, 0, 0};
        ERR(PF_FILL(&transparent_black, &posOutput->extent_hint, posOutput));
        PF_CompositeMode composite_mode;
        AEFX_CLR_STRUCT(composite_mode);
        PF_FloatMatrix float_matrix;
        AEFX_CLR_STRUCT(float_matrix.mat);
        AEFX_CLR_STRUCT(float_matrix.mat);
        float_matrix.mat[2][2]  = 1;//identity
        float_matrix.mat[2][0] =  FIX2INT(params[LETB_CENTER]->u.td.x_value) - 0.5* in_data->width* scaleFactor; // This is the x translation
        float_matrix.mat[2][1] =  FIX2INT(params[LETB_CENTER]->u.td.y_value) - 0.5* in_data->height* scaleFactor; // This is the y translation
        float_matrix.mat[0][0] = scaleFactor; //scale matrix
        float_matrix.mat[1][1] = scaleFactor; //scale matrix
        
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
        
        
        
        
        switch (destinationPixelFormat)
        {
                
            case PrPixelFormat_BGRA_4444_8u:
                

                AEFX_CLR_STRUCT(composite_mode);
                composite_mode.opacity = 255;
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
                                                    &posOutput->extent_hint,
                                                    posOutput));
                
                iterate8Suite->iterate(in_data,
                                       0,                                                                       // progress base
                                       (output->extent_hint.bottom - output->extent_hint.top),                  // progress final
                                      posOutput,                                                                // src
                                       NULL,                                                                    // area - null for all pixels
                                       (void*)&letP,                                                          // refcon - your custom data pointer
                                       PixelFuncBGRA_8u,                                                        // pixel function pointer
                                       output);
                
                break;
                
            case PrPixelFormat_VUYA_4444_8u:
                

                AEFX_CLR_STRUCT(composite_mode);
                composite_mode.opacity = 255;
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
                                                    &posOutput->extent_hint,
                                                    posOutput));
                iterate8Suite->iterate(in_data,
                                       0,                                                           // progress base
                                       (output->extent_hint.bottom - output->extent_hint.top),		// progress final
                                       posOutput,                                                   // src
                                       NULL,                                                        // area - null for all pixels
                                       (void*)&letP,                                              // refcon - your custom data pointer
                                       PixelFuncVUYA_8u,                                            // pixel function pointer
                                       output);
                
                break;
                
            case PrPixelFormat_BGRA_4444_32f:
                
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
                                                    &posOutput->extent_hint,
                                                    posOutput));
                
                // Premiere doesn't support IterateFloatSuite1, so we've rolled our own
                IterateFloat(in_data,
                             0,                                                                     // progress base
                             (output->extent_hint.bottom - output->extent_hint.top),                // progress final
                             posOutput,
                             (void*)&letP,                                                        // refcon - your custom data pointer
                             PixelFuncBGRA_32f,                                                     // pixel function pointer
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
    PF_ParamDef             param_center, param_scale,  param_force_size, param_size_source, param_aegp_position, param_aegp_scale, param_aegp_anchorpoint, param_aegp_compSize, param_aegp_compPixRatio;
	PF_Handle	infoH		=	suites.HandleSuite1()->host_new_handle(sizeof(prerender_letP));


	prerender_letP		*letP = NULL;
    

	if (infoH){
		letP = reinterpret_cast<prerender_letP*>(suites.HandleSuite1()->host_lock_handle(infoH));
		if (letP){
            AEFX_CLR_STRUCT(*letP);
           
            
			extraP->output->pre_render_data = infoH;
            
             AEFX_CLR_STRUCT(in_result);
			if (!err){
				req.preserve_rgb_of_zero_alpha = TRUE;
                req.field = PF_Field_FRAME;

				
				ERR(extraP->cb->checkout_layer(	in_data->effect_ref,
												LETB_INPUT,
												LETB_INPUT,
												&req,
												in_data->current_time,
												in_data->time_step,
												in_data->time_scale,
												&in_result));
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
                    
                    AEFX_CLR_STRUCT(param_size_source);
                    ERR(PF_CHECKOUT_PARAM(in_data,
                                          LETB_SIZE_SOURCE,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &param_size_source));
                    
                    A_long tempCompMode;
                    GetModeValue(param_size_source.u.pd.value, &tempCompMode);
                    ERR2(PF_CHECKIN_PARAM(in_data, &param_size_source));
                    
                    
                    AEFX_CLR_STRUCT(param_force_size);
                    ERR(PF_CHECKOUT_PARAM(in_data,
                                          LETB_FORCE_SCALE,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &param_force_size));
                    if (param_force_size.u.bd.value == TRUE)
                    {
                        letP->forceSizeB = 1;
                        
                    }
                    else
                    {
                         letP->forceSizeB = 0;
                    }

                    ERR2(PF_CHECKIN_PARAM(in_data, & param_force_size));
                    
                    // CHECKOUT PARAM FROM AEGP
                    
                    AEFX_CLR_STRUCT(param_aegp_position);
                    ERR(PF_CHECKOUT_PARAM(in_data,
                                          AEGP_GET_POSITION,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &param_aegp_position));
                    
                    letP->positionXF = param_aegp_position.u.td.x_value;
                    letP->positionYF = param_aegp_position.u.td.y_value;
                    ERR2(PF_CHECKIN_PARAM(in_data, &param_aegp_position));
                    
                    AEFX_CLR_STRUCT(param_aegp_scale);
                    ERR(PF_CHECKOUT_PARAM(in_data,
                                          AEGP_GET_SCALE,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &param_aegp_scale));
                    letP->layerscale_dflt_x = param_aegp_scale.u.td.x_dephault;
                    letP->layerscale_dflt_y = param_aegp_scale.u.td.y_dephault;

                    letP->layerscale_x = param_aegp_scale.u.td.x_value;
                    letP->layerscale_y = param_aegp_scale.u.td.y_value;
                    ERR2(PF_CHECKIN_PARAM(in_data, &param_aegp_scale));
                    
                    AEFX_CLR_STRUCT(param_aegp_anchorpoint);
                    ERR(PF_CHECKOUT_PARAM(in_data,
                                          AEGP_GET_ANCHORPOINT,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &param_aegp_anchorpoint));
                    
                    letP->acPointTD.x = param_aegp_anchorpoint.u.td.x_value;
                    letP->acPointTD.y = param_aegp_anchorpoint.u.td.y_value;
                    ERR2(PF_CHECKIN_PARAM(in_data, &param_aegp_anchorpoint));
                    
                    AEFX_CLR_STRUCT(param_aegp_compPixRatio);
                    ERR(PF_CHECKOUT_PARAM(in_data,
                                          AEGP_GET_COMP_PIX_RATIO,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &param_aegp_compPixRatio));
                    
                    letP->compPixRatioF = param_aegp_compPixRatio.u.fs_d.value;
                    
                    ERR2(PF_CHECKIN_PARAM(in_data, &param_aegp_compPixRatio));
                    
                    
                    
                    
                    AEFX_CLR_STRUCT(param_aegp_compSize);
                    ERR(PF_CHECKOUT_PARAM(in_data,
                                          AEGP_GET_COMPSIZE,
                                          in_data->current_time,
                                          in_data->time_step,
                                          in_data->time_scale,
                                          &param_aegp_compSize));
                    
                    letP->compWidthF    = PF_FpLong(FIX2INT_ROUND(param_aegp_compSize.u.td.x_value));
                    letP->compHeightF   = PF_FpLong(FIX2INT_ROUND( param_aegp_compSize.u.td.y_value));
                    ERR2(PF_CHECKIN_PARAM(in_data, &param_aegp_compSize));

                    
                    if (tempCompMode ==1)
                    {
                        letP->compModeB = true;
                    }
                    else
                    {
                        letP->compModeB = false;
                    }

                    letP->PixRatioNumF = in_data->pixel_aspect_ratio.num;
                    letP->PixRatioDenF = in_data->pixel_aspect_ratio.den;
                    PF_FpLong scale_x = in_data->downsample_x.num / (float)in_data->downsample_x.den,
                              scale_y = in_data->downsample_y.num / (float)in_data->downsample_y.den;
                    
                    
                    letP->InputWidthF  = PF_FpLong (in_data->width);
                    letP->InputHeightF  = PF_FpLong (in_data->height);
                    
                    
                    //DOWNSCALE
                    letP->InputWidthF   *=   scale_x;
                    letP->InputHeightF  *=   scale_y;
                    letP->positionXF    *=   scale_x;
                    letP->positionYF    *=   scale_y;
                    letP->acPointTD.x   *=   scale_x;
                    letP->acPointTD.y   *=   scale_y;
                    letP->compWidthF    *=   scale_x;
                    letP->compHeightF   *=   scale_y;
                    
                    
                   

                    if (letP->compModeB == TRUE)
                    {
                        letP->compRatioF = (((double)letP->compWidthF)) / ((double)letP->compHeightF*letP->compPixRatioF); //ratio from the current comp;
                        letP->layerRatioF = letP->compRatioF;
                    }
                    else
                    {
                        letP->layerRatioF = (((double)in_data->width) *  letP->PixRatioNumF) / ((double)in_data->height*letP->PixRatioDenF); //ratio input from layer;
                      
                    }
                    
                    if (letP->forceSizeB ==1)
                    {
                        letP->scaleFactorxF =  PF_FpLong   (letP->layerscale_x*letP->PixRatioNumF/(letP->layerscale_dflt_x*(float)letP->PixRatioDenF));
                        letP->scaleFactoryF =  PF_FpLong   (letP->layerscale_y/letP->layerscale_dflt_y);

                        
                    }
                    else
                    {
                        letP->scaleFactorxF = letP->PixRatioNumF/(float)letP->PixRatioDenF; //1.000;
                        letP->scaleFactoryF = 1.000;
                    }

                    
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


	my_sequence_dataP	seqP	= reinterpret_cast<my_sequence_dataP>(DH(in_data->sequence_data));
    AEGP_SuiteHandler suites(in_data->pica_basicP);
    

    PF_ParamDef param_preset, param_slider, param_mode, param_trsp, param_color;
   


	
	ERR(AEFX_AcquireSuite(	in_data, 
							out_data, 
							kPFWorldSuite, 
							kPFWorldSuiteVersion2, 
							STR(StrID_Err_LoadSuite),
							(void**)&wsP));
	if (!err && wsP && seqP){

		prerender_letP *letP = reinterpret_cast<prerender_letP*>(suites.HandleSuite1()->host_lock_handle(reinterpret_cast<PF_Handle>(extraP->input->pre_render_data)));
        
		if (letP){
            


			// checkout input & output buffers.
			ERR((extraP->cb->checkout_layer_pixels(	in_data->effect_ref, LETB_INPUT, &inputP)));
			ERR(extraP->cb->checkout_output(in_data->effect_ref, &outputP));
            letP->in_data = *in_data;
            letP->samp_pb.src = inputP;
            

            // determine requested output depth
            ERR(wsP->PF_GetPixelFormat(outputP, &format));
            originPt.h = (A_short)(in_data->output_origin_x);
            originPt.v = (A_short)(in_data->output_origin_y);
            
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
			ERR2(extraP->cb->checkin_layer_pixels(in_data->effect_ref, LETB_INPUT));
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
            
            case PF_Cmd_COMPLETELY_GENERAL:
                err = RespondtoAEGP(in_data,
                                    out_data,
                                    params,
                                    output,
                                    extra);
                break;
                

			case PF_Cmd_SEQUENCE_SETUP:
				err = SequenceSetup(in_data,out_data);
				break;

			case PF_Cmd_SEQUENCE_SETDOWN:
				err = SequenceSetdown(in_data,out_data);
				break;

			case PF_Cmd_SEQUENCE_RESETUP:
				err = SequenceSetup(in_data,out_data);
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
