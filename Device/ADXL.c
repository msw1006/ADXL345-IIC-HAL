#include "ADXL.h"
#include "i2c.h"

float GAINX = 0.0f;
float GAINY = 0.0f;
float GAINZ = 0.0f;

adxlStatus Dev_ADXL_Init( ADXL_InitTypeDef* adxl )
{
    if( HAL_I2C_IsDeviceReady( &I2chandler, ADXL_I2C_ADDR, 2, 5 ) != HAL_OK )
    {
        printf( "ADXL IsDeviceReady ERROR\r\n" );
        return ADXL_ERR;
    }
    else
    {
        printf( "ADXL IsDeviceReady OK\r\n" );
    }
    HAL_Delay( 5 );
    uint8_t testval = 0;
    // The Device Address register is constant, i.e. = 0xE5
    readRegister( DEVID, &testval, 1 );
    if( testval != 0xE5 ) {
        return ADXL_ERR;
    }
    printf( "ADXL DEVID=0x%X \r\n", testval );
    // Init. of BW_RATE and DATAFORMAT registers
    adxlBW( adxl );
    adxlFormat( adxl );
    // Settings gains
    if( adxl->Resolution == RESOLUTION_10BIT )
    {
        switch( adxl->Range ) {
        case RANGE_2G:
            GAINX = GAINY = GAINZ = 1 / 255.0f;
            break;
        case RANGE_4G:
            GAINX = GAINY = GAINZ = 1 / 127.0f;
            break;
        case RANGE_8G:
            GAINX = GAINY = GAINZ = 1 / 63.0f;
            break;
        case RANGE_16G:
            GAINX = GAINY = GAINZ = 1 / 31.0f;
            break;
        }
    } else
    {
        GAINX = GAINY = GAINZ = 1 / 255.0f;
    }
    // Setting AutoSleep and Link bits
    uint8_t reg = 0;
    readRegister( POWER_CTL, &reg, 1 );
    
    if( ( adxl->AutoSleep ) == AUTOSLEEPON ) {
        reg |= ( 1 << 4 );
    }
    else {
        reg &= ~( 1 << 4 );
    }
    if( ( adxl->LinkMode ) == LINKMODEON ) {
        reg |= ( 1 << 5 );
    }
    else {
        reg &= ~( 1 << 5 );
    }
    printf( "ADXL POWER_CTL=0x%X \r\n", reg );
    writeRegister( POWER_CTL, reg );

    printf( "ADXL initOk \r\n" );
    return ADXL_OK;
}





/** Writing ADXL Registers.
* @address: 8-bit address of register
* @value  : 8-bit value of corresponding register
* Since the register values to be written are 8-bit, there is no need to multiple writing
*/
static void writeRegister( uint8_t address, uint8_t value )
{
  
  uint8_t address0=address;
    if( address > 63 ) {
        address = 63;
    }
    // Setting R/W = 0, i.e.: Write Mode
    address &= ~( 0x80 );
    
    printf("writeRegister Old=%d New=%d \r\n",address0,address);

    uint8_t d[2];
    d[0] = address;
    d[1] = value;
    uint16_t AddrW = 0x3a;
    HAL_I2C_Master_Transmit( &I2chandler, ADXL_I2C_ADDR, ( uint8_t* )d, 2, 10 );
}


/** Reading ADXL Registers.
* @address: 8-bit address of register
* @retval value  : array of 8-bit values of corresponding register
* @num      : number of bytes to be written
*/

void readRegister( uint8_t address, uint8_t* value, uint8_t num )
{
  
  uint8_t address0=address;
    if( address > 63 ) {
        address = 63;
    }
    // Multiple Byte Read Settings
    if( num > 1 ) {
        address |= 0x40;
    }
    else {
        address &= ~( 0x40 );
    }
    // Setting R/W = 1, i.e.: Read Mode
    address |= ( 0x80 );
    
    
        printf("readRegister Old=%d New=%d \r\n",address0,address);
    
    
   
    while( HAL_I2C_Master_Transmit( &I2chandler, ADXL_I2C_ADDR, &address, 1, 1000 ) != HAL_OK );
    while( HAL_I2C_Master_Receive( &I2chandler, ADXL_I2C_ADDR, value, num, 1000 ) != HAL_OK );
   
}

/**
Bandwidth Settings:
 Setting BW_RATE register
 BWRATE[4] = LOW POWER SETTING
 BWRATE[0-3] = DATA RATE i.e. 0110 for 6.25 Hz // See Table 6,7
 @param LPMode = 0 // Normal mode, Default
                             = 1 // Low Power Mode
 @param BW : Badwidth; See Tables 6 and 7

                                NORMAL MODE
                BW value    |  Output Data Rate (Hz)
                ---------------------------------
                        6       |               6.25 // Default
                        7       |               12.5
                        8       |               25
                        9       |               50
                        10      |               100
                        11      |               200
                        12      |               400
                        13      |               800
                        14      |               1600
                        15      |               3200


                                LOWPOWER MODE
                BW value    |  Output Data Rate (Hz)
                ---------------------------------
                        7       |               12.5    // Default
                        8       |               25
                        9       |               50
                        10      |               100
                        11      |               200
                        12      |               400
            */
static void adxlBW( ADXL_InitTypeDef* adxl )
{
    uint8_t bwreg = 0;
    writeRegister( BW_RATE, bwreg );
    if( adxl->LPMode  == LPMODE_LOWPOWER )
    {
        // Low power mode
        bwreg |= ( 1 << 4 );
        if( ( ( adxl->Rate ) < 7 ) && ( ( adxl->Rate ) > 12 ) ) {
            bwreg += 7;
        }
        else {
            bwreg += ( adxl->Rate );
        }
        writeRegister( BW_RATE, bwreg );
    }
    else
    {
        // Normal Mode
        if( ( ( adxl->Rate ) < 6 ) && ( ( adxl->Rate ) > 15 ) ) {
            bwreg += 6;
        }
        else {
            bwreg += ( adxl->Rate );
        }
        writeRegister( BW_RATE, bwreg );
    }
}


/**
    Data Format Settings
    DATA_FORMAT[7-0] = SELF_TEST  SPI  INT_INVERT  0  FULL_RES  Justify  Range[2]

    SPI bit:            0 = 4-wire (Default)
                        1 = 3-wire
    INT_Invert:         0 = Active High (Default)
                        1 = Active Low
    Full Res:           0 = 10-bit (Default)
                        1 = Full Resolution
    Justify:            0 = Signed (Default)
                        1 = MSB
    Range:
                Range value     |  Output Data Rate (Hz)
                ---------------------------------
                        0       |       +-2g    // Default
                        1       |       +-4g
                        2       |       +-8g
                        3       |       +-16g
*/

static void adxlFormat( ADXL_InitTypeDef* adxl )
{
    uint8_t formatreg = 0;
    writeRegister( DATA_FORMAT, formatreg );
    formatreg = ( adxl->SPIMode << 6 ) | ( adxl->IntMode << 5 ) | ( adxl->Justify << 2 ) | ( adxl->Resolution << 3 );
    formatreg += ( adxl -> Range );
       writeRegister( DATA_FORMAT, formatreg );
}


/** Reading Data
* @retval : data: array of accel.
            outputType  : OUTPUT_SIGNED: signed int
                           OUTPUT_FLOAT: float
            if output is float, the GAIN(X-Y-Z) should be defined in definitions.
* @usage :  Depending on your desired output, define an array of type uint16_t or float with 3 cells:
            uint16_t acc[3];
            ADXL_getAccel(acc,OUTPUT_SIGNED);
            and so on...
*/
void ADXL_getAccel( void* Data, uint8_t outputType )
{
    uint8_t data[6] = {0, 0, 0, 0, 0, 0};
    readRegister( DATA0, data, 6 );
    if( outputType == OUTPUT_SIGNED )
    {
        int16_t* acc = Data;
        // Two's Complement
        acc[0] = ( int16_t )( ( data[1] * 256 + data[0] ) );
        acc[1] = ( int16_t )( ( data[3] * 256 + data[2] ) );
        acc[2] = ( int16_t )( ( data[5] * 256 + data[4] ) );
    }
    else if( outputType == OUTPUT_FLOAT )
    {
        float* fdata = Data;
        fdata[0] = ( ( int16_t )( ( data[1] * 256 + data[0] ) ) ) * GAINX * 9.8f;
        fdata[1] = ( ( int16_t )( ( data[3] * 256 + data[2] ) ) ) * GAINY * 9.8f;
        fdata[2] = ( ( int16_t )( ( data[5] * 256 + data[4] ) ) ) * GAINZ * 9.8f;
    }
}


/** Starts Measure Mode
* @param: s = ON or OFF

*/
void ADXL_Measure( Switch s )
{
    uint8_t reg;
    readRegister( POWER_CTL, &reg, 1 );
    switch( s ) {
    case ON:
        reg &= ~( 1 << 2 );
        reg |= ( 1 << 3 );
        writeRegister( POWER_CTL, reg );
        break;
    case OFF:
        reg &= ~( 1 << 3 );
        writeRegister( POWER_CTL, reg );
        break;
    }
}

/** Starts Sleep Mode
* @param: s         =  ON or OFF
* @param: rate  =  SLEEP_RATE_1HZ
                                     SLEEP_RATE_2HZ
                                     SLEEP_RATE_4HZ
                                     SLEEP_RATE_8HZ
*/
void ADXL_Sleep( Switch s, uint8_t rate )
{
    uint8_t reg;
    readRegister( POWER_CTL, &reg, 1 );
    switch( s ) {
    case ON:
        reg |= ( 1 << 2 );
        reg &= ~( 1 << 3 );
        reg += rate;
        writeRegister( POWER_CTL, reg );
        break;
    case OFF:
        reg &= ~( 1 << 2 );
        writeRegister( POWER_CTL, reg );
        break;
    }
}

/** Starts Standby Mode
* @param: s = ON or OFF
        OFF: Takes the module into sleep mode.

*/
void ADXL_Standby( Switch s )
{
    uint8_t reg;
    readRegister( POWER_CTL, &reg, 1 );
    switch( s ) {
    case ON:
        reg &= ~( 1 << 2 );
        reg &= ~( 1 << 3 );
        writeRegister( POWER_CTL, reg );
        break;
    case OFF:
        reg |= ( 1 << 2 );
        writeRegister( POWER_CTL, reg );
        break;
    }
}


