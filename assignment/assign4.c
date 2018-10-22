/*
****************************************************************************
* Class: SER486
* Term: Spring 2017
* Instructor: Richard Whitehouse 
*
* Assigned Project Number: 4
* File Name: assign4.c
* 
* Programmer: Ting Yeu Yang (ID:1211367063)
* 
****************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <signal.h>

#define SYS_DELAY_TIME 1000     // ms
#define LUX_DELAY_TIME 500      // ms

#define TSL2561_ADDR             (0x39)
#define TSL2561_COMMAND_BIT      (0x80)
#define digitShift		 (256)

/* Register */
#define TSL2561_REG_CTRL         (0x00)
#define TSL2561_REG_TIMING       (0x01)
#define TSL2561_CH0_REG_LOW      (0x8C)
#define TSL2561_CH0_REG_HIGH     (0x8D)

/* Data */
#define TSL2561_CTRL_PWR_UP      (0x03)
#define TSL2561_CTRL_PWR_DOWN    (0x00)
#define TSL2561_GAIN_LOW         (0x00)   // low gain (1x)
#define TSL2561_GAIN_HIGH        (0x01)   // high gain (16x)

/* globel variables */ 
volatile int fd;  // i2c device identifier

/* Detect the luminosity and return it */
int getLux(int fd)
{  
    // Wait for the conversion
    delay(LUX_DELAY_TIME);

    // Reads visible + IR diode from the I2C device auto
    short int dataLow = wiringPiI2CReadReg16(fd, TSL2561_CH0_REG_LOW);
    short int dataHigh = wiringPiI2CReadReg16(fd, TSL2561_CH0_REG_HIGH);  
  
    return (dataHigh * digitShift + dataLow);
}

/* Disable the i2c connection and exit the program */
void closeConn(int sig)
{    
    // Disable the device
    wiringPiI2CWriteReg8(fd, TSL2561_REG_CTRL, TSL2561_CTRL_PWR_DOWN);
    
    exit(sig);
}

int main (int argc, char *argv[])
{
    //int fd;
    short int lux;    

    if(wiringPiSetup() != 0)
    {
    	printf("wiringPiSetup() fails.\n");
        return 1;
    }
    
    // use [gpio i2cdetect] to get TSL2561_ADDR
    fd = wiringPiI2CSetup(TSL2561_ADDR);
    
    // Enable the device
    wiringPiI2CWriteReg8(
        fd, TSL2561_REG_CTRL | TSL2561_COMMAND_BIT, TSL2561_CTRL_PWR_UP);
  
    // Set timing
    wiringPiI2CWriteReg8(
        fd, TSL2561_REG_TIMING | TSL2561_COMMAND_BIT, TSL2561_GAIN_LOW);
        
    // ctrl-c interrupt process
    signal(SIGINT, closeConn);
    
    printf("Ready...\n");

    while(1)
    {
        delay(SYS_DELAY_TIME); 	
        lux = getLux(fd);
	    printf("%d\n", lux);        
    }
	
    return 0 ;
}
