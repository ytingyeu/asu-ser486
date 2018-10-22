/*
****************************************************************************
* Class: SER486
* Term: Spring 2017
* Instructor: Richard Whitehouse 
*
* Assigned Project Number: 2
* File Name: assign2.c
* 
* Programmer: Ting Yeu Yang (ID:1211367063)
* 
******************************************************************************
*/

#include <stdio.h>
#include <wiringPi.h> 
 
#define BUTTON_PIN 7      // BCM 18
#define LED_ONE_PIN 0     // BCM 17
#define LED_TWO_PIN 2     // BCM 27
#define LED_THREE_PIN 3   // BCM 22
#define LED_FOUR_PIN 21   // BCM 5
#define LED_FIVE_PIN 22   // BCM 6
#define LED_SIX_PIN 23    // BCM 13
#define LED_SEVEN_PIN 24  // BCM 19
#define LED_EIGHT_PIN 25  // BCM 26
#define SYS_DELAY_TIME 500      // ms
#define SYS_DEBOUNCE_DUR 200    // ms
#define DIR_RIGHT 1
#define NUM_OF_LED 8
#define INDEX_OFFSET 1


// globel variables 
int ledPinArr[NUM_OF_LED] = {
    LED_ONE_PIN, LED_TWO_PIN, LED_THREE_PIN, LED_FOUR_PIN, 
    LED_FIVE_PIN, LED_SIX_PIN, LED_SEVEN_PIN, LED_EIGHT_PIN
};
int lightOnDir = DIR_RIGHT;
long lastInterruptTime = 0;


// switch the direction of lighting-on LEDs
void switchDirection(void)
{
    // Debouncing
    long currInterruptTime = millis();
    
    // Reverse the direction if the trigger-intervel is more
    // than the debounce duration
    if((currInterruptTime - lastInterruptTime) > SYS_DEBOUNCE_DUR)
    {
        lightOnDir = -lightOnDir; // Reverse the direction
    }
    
    // Update lastInterruptTime
    lastInterruptTime = currInterruptTime;
} 
 

int main(void) 
{
    // initialize
    int targetLedIndex = 0;
    int index;
    
    wiringPiSetup();
    
    for(index = 0; index < NUM_OF_LED; index++)
    {
        pinMode(ledPinArr[index], OUTPUT);
        digitalWrite(ledPinArr[index], LOW);
    }
    
    // interrupt process
    wiringPiISR(BUTTON_PIN, INT_EDGE_FALLING, &switchDirection);   
    
    digitalWrite(ledPinArr[targetLedIndex], HIGH);
    delay(SYS_DELAY_TIME);
 
    for (;;)
    {
        digitalWrite(ledPinArr[targetLedIndex], LOW);
        targetLedIndex += lightOnDir;
        
        // Return to the beginning or the end of the LED-line
        if(targetLedIndex >= NUM_OF_LED)
        {
            targetLedIndex = targetLedIndex % NUM_OF_LED;
        }
        else if(targetLedIndex < 0)
        {
            targetLedIndex = NUM_OF_LED - INDEX_OFFSET;
        }
        
        digitalWrite(ledPinArr[targetLedIndex], HIGH);

        delay(SYS_DELAY_TIME);
    }
    
    return 0;
}