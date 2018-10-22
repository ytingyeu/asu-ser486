/*
****************************************************************************
* Class: SER486
* Term: Spring 2017
* Instructor: Richard Whitehouse 
*
* Assigned Project Number: Semester Project
* File Name: project.c
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
#include <libconfig.h>

/* PIN Setting */
#define RELAY_PIN 1

/* Luminosity Setting  */
#define DARK_LUX 10

/* Flags */
#define LIGHT_ON 1
#define LIGHT_OFF 0

/* Time Setting  */
#define SYS_DELAY_TIME     1000   // ms
#define LUX_DELAY_TIME      500   // ms
#define CYC_DELAY_TIME    (0.25)   // hrs
#define NEXT_DAY_DELAY        6   // hrs
#define HR_TO_MS        3600000
#define SPEED_UP      (1.0/60/6)  // speed up for demo, 1 hr to 10 sec

/* TSL2561 Setting */
#define TSL2561_ADDR             (0x39)
#define TSL2561_COMMAND_BIT      (0x80)
#define DIGIT_SHIFT	         (256)

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
volatile int fd;                          // i2c device identifier

/* function prototype */
int getLux(int fd);
void closeConn(int sig);
int getLightHrs(void);

/* in-line function */
void error(const char *msg) { perror(msg); exit(0); }


/* main function  */
int main (int argc, char *argv[])
{
    short int lux;
    int lightFlag = LIGHT_OFF;
    int enoughFlag = 0;

    unsigned long lightOnTime = 0;
    unsigned long lightOffTime = 0; 
    unsigned long lightOnDuration = 0;
    unsigned long lightOffDuration = 0;

    // initialize config_t struct
    config_t cfg;
    config_init(&cfg);

    // load config file
    if(!config_read_file(&cfg, "config.cfg"))
    {
    	config_destroy(&cfg);
	error("Loading config.cfg fails");
    }

    // read config
    int setting_hrs;
    config_lookup_int(&cfg, "hrs", &setting_hrs);
    int setDuration = setting_hrs * HR_TO_MS * SPEED_UP;
    printf("Expected light-on hrs: %d hrs\n", setting_hrs);
    printf("After speed-up: %d ms\n", setDuration);



    // setup wiringPi
    if(wiringPiSetup() != 0)
    {
    	error("wiringPiSetup() fails.\n");
    }
    
    // use [gpio i2cdetect] to get TSL2561_ADDR
    fd = wiringPiI2CSetup(TSL2561_ADDR);
    
    // setup output pin
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
        
    // ctrl-c process
    signal(SIGINT, closeConn);
    
    printf("Ready...\n");

    while(1)
    {        
        // get lux value
        lux = getLux(fd);
        printf("lux = %d\n", lux);

	// It's dark, the light is off, and the light is not enough
	// -> turn on the light and record the time
        if(lux <= DARK_LUX && lightFlag == LIGHT_OFF && enoughFlag == 0)
        {            
            lightFlag = LIGHT_ON;
            lightOnTime = millis();
        }
	// It's dark and the light is already on
        else if(lux <= DARK_LUX && lightFlag == LIGHT_ON)
        {
            // get current time
            lightOnDuration = millis() - lightOnTime;
            
	    // If the light-on duration is enough
	    // -> turn off the light, set enoughFlag, update time info
            if(lightOnDuration >= setDuration)
            {
                lightFlag = LIGHT_OFF;
		enoughFlag = 1;
	        lightOffTime = millis();
		lightOffDuration = 0;

                // setup and write log file
                FILE *fptr = fopen("project.log", "a+");  // log file
                if(fptr == NULL)
                {
		    error("Opening log file fails\n");
		}

		fprintf(fptr, "Light-on duration = %lu ms\n", lightOnDuration);
		fclose(fptr);
            }            
	}
	// It's bright, or the light is enough
	// -> turn off the light
        else
        {
            lightFlag = LIGHT_OFF;
	    lightOffDuration = millis() - lightOffTime;
        }
	
        // check the status of light and then turn on/off it
        if(lightFlag == LIGHT_ON)
        {
            digitalWrite(RELAY_PIN, HIGH);
	    printf("On Duration = %lu\n", lightOnDuration);
	}
        else
        {
            digitalWrite(RELAY_PIN, LOW);
	    printf("Off Duration = %lu\n", lightOffDuration);
        }
        
	// If the light is enough, reset enough flag
	// after twelve hrs
        if(enoughFlag == 1)
	{
	    if(millis()  - lightOffTime > NEXT_DAY_DELAY * HR_TO_MS * SPEED_UP)
	    {
                // reset enoughFlag and timing
	        enoughFlag = 0;
		lightOnDuration = 0;
	    }
	}

        delay(CYC_DELAY_TIME * HR_TO_MS * SPEED_UP);
        
    }

    return 0 ;
}

/* Detect the luminosity and return it */
int getLux(int fd)
{ 
    // Enable the device
    wiringPiI2CWriteReg8(
        fd, TSL2561_REG_CTRL | TSL2561_COMMAND_BIT, TSL2561_CTRL_PWR_UP);
  
    // Set timing
    wiringPiI2CWriteReg8(
        fd, TSL2561_REG_TIMING | TSL2561_COMMAND_BIT, TSL2561_GAIN_LOW);
 
    // Wait for the conversion
    delay(LUX_DELAY_TIME);

    // Reads visible + IR diode from the I2C device auto
    short int dataLow = wiringPiI2CReadReg16(fd, TSL2561_CH0_REG_LOW);
    short int dataHigh = wiringPiI2CReadReg16(fd, TSL2561_CH0_REG_HIGH);
    
    // Disable the device
    wiringPiI2CWriteReg8(fd, TSL2561_REG_CTRL, TSL2561_CTRL_PWR_DOWN);
  
    return (dataHigh * DIGIT_SHIFT + dataLow);
}

/* Disable the i2c connection and exit the program */
void closeConn(int sig)
{    
    // Disable the device
    wiringPiI2CWriteReg8(fd, TSL2561_REG_CTRL, TSL2561_CTRL_PWR_DOWN);
    exit(sig);
}

