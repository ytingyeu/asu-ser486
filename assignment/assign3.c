/*
****************************************************************************
* Class: SER486
* Term: Spring 2017
* Instructor: Richard Whitehouse 
*
* Assigned Project Number: 3
* File Name: assign3.c
* 
* Programmer: Ting Yeu Yang (ID:1211367063)
* 
****************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define BUTTON_FORWARD 8   // BCM 2
#define BUTTON_STOP 9      // BCM 3
#define BUTTON_BACKWARD 7  // BCM 4
#define L293D_EN 4         // BCM 23, enable.disable
#define L293D_A 5          // BCM 24, control
#define L293D_B 6          // BCM 25, control

#define SYS_DELAY_TIME 2000      // ms
#define SYS_DEBOUNCE_DUR 200     // ms

long lastInterruptTime = 0;

void trunForward(void)
{
    // Debouncing
    long currInterruptTime = millis();

    if((currInterruptTime - lastInterruptTime) > SYS_DEBOUNCE_DUR)
    {
    	printf("Forward...\n");
        digitalWrite(L293D_A, LOW);
	digitalWrite(L293D_B, HIGH);
        digitalWrite(L293D_EN, HIGH);
    }
    
    // Update lastInterruptTime
    lastInterruptTime = currInterruptTime;
}

void turnBackward(void)
{
    // Debouncing
    long currInterruptTime = millis();    

    if((currInterruptTime - lastInterruptTime) > SYS_DEBOUNCE_DUR)
    {
    	printf("Backward...\n");
        digitalWrite(L293D_A, HIGH);
	digitalWrite(L293D_B, LOW);
        digitalWrite(L293D_EN, HIGH);
    }
    
    // Update lastInterruptTime
    lastInterruptTime = currInterruptTime;
}

void trunStop(void)
{
    // Debouncing
    long currInterruptTime = millis();

    if((currInterruptTime - lastInterruptTime) > SYS_DEBOUNCE_DUR)
    {
        printf("Stop...\n");
        digitalWrite(L293D_EN, LOW);
    }
    
    // Update lastInterruptTime
    lastInterruptTime = currInterruptTime;
}

int main (int argc, char *argv[])
{
    if(wiringPiSetup() == 0)
    {
    	printf("wiringPiSetup() OK.\n");
    }

    // pin initialization
    pinMode(L293D_A, OUTPUT);
    pinMode(L293D_B, OUTPUT);    
    pinMode(L293D_EN, OUTPUT);
    digitalWrite(L293D_A,LOW);
    digitalWrite(L293D_B,LOW);
    digitalWrite(L293D_EN,LOW);
    
    pinMode(BUTTON_FORWARD, INPUT);
    pinMode(BUTTON_BACKWARD, INPUT);    
    pinMode(BUTTON_STOP, INPUT);
    
    // interrupt process
    wiringPiISR(BUTTON_FORWARD, INT_EDGE_FALLING, &trunForward);
    wiringPiISR(BUTTON_BACKWARD, INT_EDGE_FALLING, &turnBackward);
    wiringPiISR(BUTTON_STOP, INT_EDGE_FALLING, &trunStop);
    
    printf("Ready...\n");

    while(1)
    {
        delay(SYS_DELAY_TIME);
    }
	
    return 0 ;
}
