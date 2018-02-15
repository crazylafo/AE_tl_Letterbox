//
//  getPixel.h
//  fSample
//
//  Created by Thomas Laforge on 08/02/2018.
//
//

#ifndef getPixel_h
#define getPixel_h





static  PF_Pixel
*sampleIntegral32(PF_EffectWorld &def,
                  int x,
                  int y)
{
    return (PF_Pixel*)((char*)def.data +
                       (y * def.rowbytes) +
                       (x * sizeof(PF_Pixel)));
}
//same in 16bits
static PF_Pixel16
*sampleIntegral64(PF_EffectWorld &def,
                  int x,
                  int y)
{
    assert(PF_WORLD_IS_DEEP(&def));
    return (PF_Pixel16*)((char*)def.data +
                         (y * def.rowbytes) +
                         (x * sizeof(PF_Pixel16)));
}
//same in 32 bits
static  PF_PixelFloat
*sampleIntegral128(PF_EffectWorld &def,
                   int x,
                   int y)
{
    return (PF_PixelFloat*)((char*)def.data +
                            (y * def.rowbytes) +
                            (x * sizeof(PF_PixelFloat)));
}


static void
GetPixelValue(
              PF_EffectWorld  *WorldP,
              PF_PixelFormat  pxFormat,
              int x,
              int y,
              PF_PixelFloat		*pixvalueF)
{
    switch (pxFormat)
    {
        case PF_PixelFormat_ARGB128:
            pixvalueF = sampleIntegral128(*WorldP, x, y);
            break;
            
        case PF_PixelFormat_ARGB64:
            PF_Pixel16 temp16;
            temp16 = *sampleIntegral64(*WorldP, x, y);
            pixvalueF->red = PF_FpShort(temp16.red) / PF_MAX_CHAN16;
            pixvalueF->green = PF_FpShort(temp16.green) / PF_MAX_CHAN16;
            pixvalueF->blue = PF_FpShort(temp16.blue) / PF_MAX_CHAN16;
            break;
            
        case PF_PixelFormat_ARGB32:
            PF_Pixel temp8;
            temp8 = *sampleIntegral32(*WorldP, x, y);
            pixvalueF->red = PF_FpShort(temp8.red) / PF_MAX_CHAN8;
            pixvalueF->green = PF_FpShort(temp8.green) / PF_MAX_CHAN8;
            pixvalueF->blue = PF_FpShort(temp8.blue) / PF_MAX_CHAN8;
            break;
            
    }
}

#endif /* getPixel_h */
