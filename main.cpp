//ESE519 Lab6 Controller Part 1 (INCOMPLETE IMPLIMENTATION)
//Author: Carter Sharer
//Date: 10/18/2016
//Sources: ESE350 Whack-A-Mole: https://developer.mbed.org/users/mlab4/code/ESE350-Whack-a-Mole/
//This is a simple working templete for sending and receving between two MRF24J40 Modules
//Contains a communication protocol for sending a receiving 

#include "mbed.h"
#include <string>
#include "Joystick.h"
#include "wifiGETWrapper.h"
//#define SEND        //Uncomment if you want to transmit data

#define NONE 250

//============================
//==    Pin Assignments     ==
//============================
//Knobs
#define POT1 p17  //Knob1
#define POT2 p18  //Knob2
#define POT3 p16  //Knob3
#define POT4 p15  //Knob4
//JoyStick
#define POTV p19 //Vertial
#define POTH p20 //Horizontal
//Button
#define BUTTON1 p21
#define COMMUNICATION_FORMAT "Jstick_h:|%0.0f|Jstick_v:|%0.0f|Knob1|%0.2f|Knob2|%0.2f|Knob3|%0.2f|Knob4|%0.2f|Button:|%d"

//============================
//==        Objects         ==
//============================
//Knobs
AnalogIn pot1(POT1);
AnalogIn pot2(POT2);
AnalogIn pot3(POT3);
AnalogIn pot4(POT4);
float knob1, knob2, knob3, knob4;

//Joystick
Joystick jstick(POTV, POTH);
float jstick_h, jstick_v;

//Button
DigitalIn Button(BUTTON1);
bool button;

// LEDs
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

// Timer
Timer timer;

// Serial port for showing RX data.
Serial pc(USBTX, USBRX);

// Send / receive buffers.
// IMPORTANT: The MRF24J40 is intended as zigbee tranceiver; it tends
// to reject data that doesn't have the right header. So the first
// 8 bytes in txBuffer look like a valid header. The remaining 120
// bytes can be used for anything you like.
char txBuffer[128];
char rxBuffer[128];
int rxLen;
//***************** You can start coding here *****************//

//Returns true if c is a letter (upper or lower case), false otherwise
bool isLetter(char c) {
    
    if ((c>=65 && c < 90) | (c>=97 && c < 122)){
        return true;
        }
    else{
        return false;
        }
}

//Returns true if c is a number character (0-9), false otherwise
bool isNumber(char c) {
    
    if (c>=48 && c < 57){
        return true;
        }
    else{
        return false;
        }
}

//Pulls data out of rxBuffer and updates global variables accordingly 
//Len is the length of the rxBuffer we are going to scan 
void communication_protocal(int len)
{
    bool found_name = false;
    bool found_num = false;
    bool complete_name = false;
    bool complete_num = false;
    uint8_t name_start = NONE; uint8_t name_end = NONE;
    uint8_t num_start = NONE; uint8_t num_end = NONE;
    
    //Loop through all charaters in rxBuffer
    for(uint8_t i = 0; i <= rxLen; i++) {
        char c = rxBuffer[i];
        //pc.printf("Indexed char '%c'\r\n", c);

        //Is it the start of a name? (Check if its a letter)
        if(isLetter(c) & name_start==NONE) { //if a num
            //If We havent found a name yet, this is start of a name
            if(found_name == false) {
                //pc.printf("found name start at: '%c'\r\n", c);
                name_start = i;
                found_name = true;
            }
        }
        //Is is a 'end of name' charater? Check for ' ', ':', '-'
        else if(((c == '|') | (c == ':') | (c == '-')) & found_name & !complete_name) {// found end name character
            if(found_name) {
                complete_name = true;
                name_end = i;
                //pc.printf("found end of name at: '%c'\r\n", txBuffer[name_end]);
            }
        }

        //Is it a 'start of a number' charater? Check if its a number, or '-', or a '.'
        else if( (isNumber(c) | (c=='-') | (c=='.')) & complete_name & num_start==NONE) {
            if(found_num == false) {
                //pc.printf("found num start at: '%c'\r\n",c);
                num_start = i;
                found_num = true;
            }
        }
        //Is it a 'end of number' character? Check if its a ' ', ':', '-', or a letter
        else if( (((c=='|')|(c==':')|(c=='-')) | isLetter(c)) & found_num & complete_name) {
            if(found_num) {
                complete_num = true;
                num_end = i;
                //pc.printf("found end of num at: '%c' \r\n", txBuffer[num_end]);
            }
        }
        
        //If we have a complete name AND number value (ie. start and end of each != NONE)
        if(found_name & found_num & complete_name & complete_num) {
            //pc.printf("Found MATCH\r\n");
            //Reset flags
            found_name = false;
            found_num = false;
            complete_name = false;
            complete_num = false;
            
            //Set name
            uint8_t nameLen = uint8_t((name_end-name_start) + 1);
            char * name[nameLen];
            *name = &rxBuffer[name_start];
            rxBuffer[name_end] = '\0';
            
            //Set num
            uint8_t numLen = uint8_t((num_end-num_start) + 1);
            char * num[numLen];
            *num = &rxBuffer[num_start];
            rxBuffer[num_end] = '\0';
            
            //Now that we have isolated a name and its number value
            //we want to set the corresponding value to this number.
            //Ex: if name is 'Knob4' and num is '0.34', we want to the the 
            // variable name knob4 to the value 0.34.  
            //Do this for all variable names in COMMUNICATION_FORMAT
            //HINT: look up strcmp, and atof
            if (strcmp(name, "jstick_h") == 0){
                jstick_h = atof(num);
                }
            else if (strcmp(name, "jstick_v") == 0){
                jstick_v = atof(num);
                }
            else if (strcmp(name, "knob1") == 0){
                knob1 = atof(num);
                }
            else if (strcmp(name, "knob2") == 0){
                knob2 = atof(num);
                }
            else if (strcmp(name, "knob3") == 0){
                knob3 = atof(num);
                }
            else if (strcmp(name, "knob4") == 0){
                knob4 = atof(num);
                }
            else if (strcmp(name, "button") == 0){
                button = atof(num);
                }
            else{
                //error
                }
            
            //Reset flags
            name_start = NONE;
            name_end = NONE;
            num_start = NONE;
            num_end = NONE;
        }
    }
}

int main (void)
{
    
    //Set Baud rate (9600-115200 is ideal)
    pc.baud(115200);
    pc.printf("\r\n Start! \r\n");
    //This is the wifi network in Detkin that we will be using. If you are somewhere else, change this line.
    initConnection("Rob's iPhone","212117650");
    //Start Timer
    timer.start();

    //Scale Joystick Values, range[-100, 100]
    jstick.setScale(-100, 100);
    int counter = 0;
    while(1) {
        //(1) Read Joystick Values, round to int8_t presision
        jstick_h = (int8_t)jstick.horizontal();
        jstick_v = (int8_t)jstick.vertical();
        //pc.printf("H: %0.2f V:%0.2f \r\n", jstick.horizontal(), jstick.vertical());

        //(2) Read Pot Values, Scale, and round to precision
        knob1 = (uint8_t)(pot1.read() * 100); //rounded to uint8_t
        knob2 = (pot2.read()*100);
        knob3 = (pot3.read());
        knob4 = (int)(pot4.read() * 100);  //rounded to float
        
        printf("Knob1: %f  Knob2: %f  Knob3: %f  Knob4: %f JStick_h: %f  JStick_v: %f  \r\n", knob1, knob2, knob3, knob4, jstick_h, jstick_v);

        //(3)Read Button Val, Add to buffer
        button = !Button.read(); //button is active low

#ifdef SEND    
        //SEND DATA: Send some data every 1/2 second
        if(timer.read_ms() >= 500) {
            //Reset the timer to 0
            timer.reset();
            // Toggle LED 2.
            led2 = led2^1;

            //(5) Add all values to buffer to be sent
            sprintf(txBuffer, COMMUNICATION_FORMAT, jstick_h, jstick_v, knob1, knob2, knob3, knob4, button);
    
            //(6) Send the buffer
            sendGET(txBuffer);
            pc.printf("Sent| %s\r\n", txBuffer);
        }   
#endif

    } //end loop
}//end main