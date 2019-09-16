#include "App_Adxl.h"

#include "ADXL.h"


ADXL_InitTypeDef AppAdxl;

void App_Adxl_INIT()
{
    AppAdxl.SPIMode = SPIMODE_4WIRE;
    AppAdxl.LPMode = LPMODE_NORMAL;
    AppAdxl.IntMode =INT_ACTIVEHIGH;            //INT_ACTIVELOW;
    AppAdxl.Justify = JUSTIFY_SIGNED;           //JUSTIFY_MSB;
    AppAdxl.Range = RANGE_4G;
    AppAdxl.LinkMode = LINKMODEON;
    AppAdxl.Rate = BWRATE_1600;//BWRATE_3200;
    AppAdxl.Resolution = RESOLUTION_10BIT;
    AppAdxl.AutoSleep = AUTOSLEEPOFF;
    adxlStatus rs = Dev_ADXL_Init( &AppAdxl );
    if( rs == ADXL_OK )
    {
        printf( "TRUE\r\n" );
    }
    else
    {
        printf( "Fault\r\n" );
    }
}
